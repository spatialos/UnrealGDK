#include "LoadBalancing/InterestManager.h"
#include "Schema/ChangeInterest.h"
#include "Utils/InterestFactory.h"

#ifdef HAS_BULLET_PHYSICS
// This is needed to suppress some warnings that UE4 escalates that Bullet doesn't
THIRD_PARTY_INCLUDES_START
// This is needed to fix memory alignment issues
PRAGMA_PUSH_PLATFORM_DEFAULT_PACKING
#include <BulletCollision/BroadphaseCollision/btDbvt.h>
PRAGMA_POP_PLATFORM_DEFAULT_PACKING
THIRD_PARTY_INCLUDES_END
#else

// Try to grab improbable's phTree thingy

#endif

#include <algorithm>
#include <cmath>

DECLARE_CYCLE_STAT(TEXT("InterestManagerCompute"), STAT_InterestManagerComputation, STATGROUP_SpatialNet);
DECLARE_CYCLE_STAT(TEXT("InterestManagerComputeBroadphase"), STAT_InterestManagerComputationBroadphase, STATGROUP_SpatialNet);
DECLARE_CYCLE_STAT(TEXT("InterestManagerComputeBox"), STAT_InterestManagerComputationBox, STATGROUP_SpatialNet);
DECLARE_CYCLE_STAT(TEXT("InterestManagerComputeSort"), STAT_InterestManagerComputationSort, STATGROUP_SpatialNet);

namespace SpatialGDK
{
FTagComponentStorage::FTagComponentStorage(Worker_ComponentId InComponentId)
	: TagComponentId(InComponentId)
{
	Components.Add(TagComponentId);
}

void FTagComponentStorage::OnComponentAdded(Worker_EntityId EntityId, Worker_ComponentId ComponentId, Schema_ComponentData* Data)
{
	if (ensureAlways(TagComponentId == ComponentId))
	{
		TaggedObjects.Add(EntityId);
		Modified.Add(EntityId);
	}
}

void FTagComponentStorage::OnRemoved(Worker_EntityId EntityId)
{
	TaggedObjects.Remove(EntityId);
	Modified.Remove(EntityId);
}

void FTagComponentStorage::OnUpdate(Worker_EntityId EntityId, Worker_ComponentId InComponentId, Schema_ComponentUpdate* Update)
{
	if (!ensure(false))
	{
		return;
	}
	Modified.Add(EntityId);
}

struct FInterestManager::FBroadphaseImpl
{
	FBroadphaseImpl() {}

	void Add(uint32 Slot, FVector Position)
	{
#ifdef HAS_BULLET_PHYSICS
		btVector3 pos(Position.X / 100, Position.Y / 100, Position.Z / 100);
		auto box = btDbvtVolume::FromCE(pos, btVector3(0.1, 0.1, 0.1));

		btDbvtNode* node = Tree.insert(box, (void*)Slot);
		node->data = (void*)Slot;

		int32 TreeId = Nodes.Num();

		check(Nodes.Num() == Slot) Nodes.Push(node);
		Positions.push_back(pos);
		Volumes.expand(node->volume);
#endif
	}

	void Add(uint32 Slot, FBox2D Box)
	{
#ifdef HAS_BULLET_PHYSICS
		btVector3 btMin(Box.Min.X / 100, Box.Min.Y / 100, -FLT_MAX);
		btVector3 btMax(Box.Max.X / 100, Box.Max.Y / 100, FLT_MAX);

		btDbvtNode* node = Tree.insert(btDbvtVolume::FromMM(btMin, btMax), (void*)Slot);
		node->data = (void*)Slot;

		int32 TreeId = Nodes.Num();

		check(Nodes.Num() == Slot) Nodes.Push(node);
		Positions.push_back((btMin + btMax) * 0.5);
		Volumes.expand(node->volume);
#endif
	}

	void Update(uint32 Slot, FVector Position)
	{
#ifdef HAS_BULLET_PHYSICS
		btVector3 pos(Position.X / 100, Position.Y / 100, Position.Z / 100);
		Positions[Slot] = pos;
		Volumes[Slot] = btDbvtVolume::FromCE(pos, btVector3(0.1, 0.1, 0.1));
		Tree.update(Nodes[Slot], Volumes[Slot], 1.0);
#endif
	}

	void SwapEntries(int32 Slot1, int32 Slot2)
	{
#ifdef HAS_BULLET_PHYSICS
		Swap(Positions[Slot1], Positions[Slot2]);
		Swap(Volumes[Slot1], Volumes[Slot2]);
		Swap(Nodes[Slot1], Nodes[Slot2]);
#endif
	}

	void Remove(uint32 Slot)
	{
#ifdef HAS_BULLET_PHYSICS
		if (Nodes.Num() > 0)
		{
			int32 LastSlot = Nodes.Num();
			SwapEntries(Slot, LastSlot);
		}

		Tree.remove(Nodes[Slot]);
		Positions.pop_back();
		Volumes.pop_back();
		Nodes.Pop();
#endif
	}

#ifdef HAS_BULLET_PHYSICS
	struct VisitStackElem
	{
		VisitStackElem(btDbvtNode* InNode, uint32 InListEnd)
			: Node(InNode)
			, ObjListEnd(InListEnd)
		{
		}

		btDbvtNode* Node;
		uint32 ObjListEnd;
		int32 CurChild = -1;
	};

	template <typename CheckBBoxFn, typename CheckLeafFn>
	void TraverseTree(uint32 NumVisitors, const CheckBBoxFn& CheckBBox, const CheckLeafFn& CheckLeaf)
	{
		Tree.optimizeIncremental(1);

		if (Nodes.Num() == 0)
		{
			return;
		}

		TArray<uint32> Visitors;
		Visitors.Reserve(NumVisitors);
		for (uint32 i = 0; i < NumVisitors; ++i)
		{
			Visitors.Add(i);
		}

		TArray<VisitStackElem> NodeStack;
		NodeStack.Reserve(64);

		NodeStack.Add(VisitStackElem(Tree.m_root, NumVisitors));

		while (NodeStack.Num() > 0)
		{
			VisitStackElem& CurNode = NodeStack.Last();
			if (CurNode.CurChild == 1)
			{
				NodeStack.Pop();
			}
			else
			{
				CurNode.CurChild++;
				btDbvtNode* Child = CurNode.Node->childs[CurNode.CurChild];

				if (!Child->isleaf())
				{
					VisitStackElem& StackElem = NodeStack.Add_GetRef(VisitStackElem(Child, CurNode.ObjListEnd));

					for (int32 i = 0; i < (int32)StackElem.ObjListEnd; ++i)
					{
						if (!CheckBBox(Visitors[i], Child->volume))
						{
							Swap(Visitors[i], Visitors[StackElem.ObjListEnd - 1]);
							--StackElem.ObjListEnd;
							--i;
						}
					}
				}
				else
				{
					union CastStruct
					{
						uint32 u32;
						void* ptr;
					} Slot;
					Slot.ptr = Child->data;
					for (uint32_t i = 0; i < CurNode.ObjListEnd; ++i)
					{
						CheckLeaf(Visitors[i], Slot.u32);
					}
				}
			}
		}
	}
#endif

	void ComputeBoxVisibility(const TArray<FBox2D>& Regions, TArray<uint64>& OutVisibility)
	{
#ifdef HAS_BULLET_PHYSICS
		btAlignedObjectArray<btDbvtVolume> btRegions;
		btRegions.reserve(Regions.Num());

		for (const FBox2D& Region : Regions)
		{
			btVector3 btMin(Region.Min.X / 100, Region.Min.Y / 100, -FLT_MAX);
			btVector3 btMax(Region.Max.X / 100, Region.Max.Y / 100, FLT_MAX);
			btRegions.push_back(btDbvtVolume::FromMM(btMin, btMax));
		}

		TraverseTree(
			Regions.Num(),
			[&](uint32 RegionNum, const btDbvtVolume& Volume) {
				return Intersect(btRegions[RegionNum], Volume);
			},
			[&](uint32 RegionNum, uint32 Slot) {
				if (Intersect(btRegions[RegionNum], Positions[Slot]))
				{
					OutVisibility[Slot] |= 1ull << RegionNum;
				}
			});
#endif
	}

#ifdef HAS_BULLET_PHYSICS
	void ComputePointVisibility(const btAlignedObjectArray<btVector3>& InPositions, TArray<uint64>& OutVisibility)
	{
		TraverseTree(
			InPositions.size(),
			[&](uint32 PosNum, const btDbvtVolume& Volume) {
				return Intersect(Volume, InPositions[PosNum]);
			},
			[&](uint32 PosNum, uint32 Slot) {
				if (Intersect(Volumes[Slot], InPositions[PosNum]))
				{
					OutVisibility[PosNum] |= 1ull << Slot;
				}
			});
	}
#endif

#ifdef HAS_BULLET_PHYSICS
	btDbvt Tree;
	TArray<btDbvtNode*> Nodes;
	btAlignedObjectArray<btVector3> Positions;
	btAlignedObjectArray<btDbvtVolume> Volumes;
#endif
};

FInterestManager::~FInterestManager() = default;

FInterestManager::FInterestManager(InterestFactory& InInterestF, FSpatialPositionStorage& InPositions,
								   FAlwaysRelevantStorage& InAlwaysRelevant, FServerAlwaysRelevantStorage& InServerAlwaysRelevant)
	: InterestF(InInterestF)
	, Positions(InPositions)
	, AlwaysRelevant(InAlwaysRelevant)
	, ServerAlwaysRelevant(InServerAlwaysRelevant)
{
	SlotMap.Reserve(1 << 16);
	Entities.Reserve(1 << 16);
	EntityPosition.Reserve(1 << 16);
	EntityFlags.Reserve(1 << 16);
	Visibility.Reserve(1 << 16);
	ActiveTimestamp.Reserve(1 << 16);
#ifdef HAS_BULLET_PHYSICS
	Broadphase = MakeUnique<FBroadphaseImpl>();
#endif
}

void FInterestManager::Advance(const TSet<Worker_EntityId_Key>& DeletedEntities)
{
	for (Worker_EntityId Entity : DeletedEntities)
	{
		if (int32* SlotPtr = SlotMap.Find(Entity))
		{
			Remove(*SlotPtr);
			if (Broadphase)
			{
				Broadphase->Remove(*SlotPtr);
			}
			SlotMap.Remove(Entity);
		}
	}

	for (Worker_EntityId Entity : Positions.GetModifiedEntities())
	{
		int32 Slot;
		int32* SlotPtr = SlotMap.Find(Entity);
		const FVector& Pos = Positions.GetPositions()[Entity];
		if (SlotPtr != nullptr)
		{
			Slot = *SlotPtr;
			if (Broadphase)
			{
				Broadphase->Update(Slot, Pos);
			}
		}
		else
		{
			uint32 Flags = 0;
			if (AlwaysRelevant.GetObjects().Contains(Entity))
			{
				Flags |= AlwaysRelevantFlag;
			}
			if (ServerAlwaysRelevant.GetObjects().Contains(Entity))
			{
				Flags |= ServerAlwaysRelevantFlag;
			}
			Slot = Allocate();
			SlotMap.Add(Entity, Slot);
			Entities[Slot] = Entity;
			EntityFlags[Slot] = Flags;
			if (Broadphase)
			{
				Broadphase->Add(Slot, Pos);
			}
		}

		EntityPosition[Slot] = FVector2D(Pos.X, Pos.Y);
		ActiveTimestamp[Slot] = FPlatformTime::Cycles64();

		if (Slot >= ActiveEntities)
		{
			SwapEntries(ActiveEntities, Slot);
			++ActiveEntities;
		}
	}
}

int32 FInterestManager::Allocate()
{
	Entities.AddDefaulted();
	EntityPosition.AddDefaulted();
	EntityFlags.AddDefaulted();
	Visibility.AddDefaulted();
	ActiveTimestamp.AddDefaulted();

	++ActiveEntities;

	int32 NumEntities = Entities.Num();
	if (ActiveEntities != NumEntities)
	{
		MoveEntry(ActiveEntities - 1, NumEntities - 1);
	}
	return ActiveEntities - 1;
	// return NumEntities - 1;
}

void FInterestManager::Remove(int32 Slot)
{
	if (Slot < ActiveEntities)
	{
		SwapEntries(ActiveEntities - 1, Slot);
		Slot = ActiveEntities - 1;
		--ActiveEntities;
	}
	if (Slot < Entities.Num() - 1)
	{
		SwapEntries(Slot, Entities.Num() - 1);
	}
	Entities.Pop(false);
	EntityPosition.Pop(false);
	EntityFlags.Pop(false);
	Visibility.Pop(false);
}

void FInterestManager::MoveEntry(int32 SlotSrc, int32 SlotDest)
{
	Worker_EntityId EntitySrc = Entities[SlotSrc];

	SlotMap[EntitySrc] = SlotDest;

	Entities[SlotDest] = Entities[SlotSrc];
	EntityPosition[SlotDest] = EntityPosition[SlotSrc];
	EntityFlags[SlotDest] = EntityFlags[SlotSrc];
	ActiveTimestamp[SlotDest] = ActiveTimestamp[SlotSrc];
}

void FInterestManager::SwapEntries(int32 Slot1, int32 Slot2)
{
	Worker_EntityId Entity1 = Entities[Slot1];
	Worker_EntityId Entity2 = Entities[Slot2];

	SlotMap[Entity1] = Slot2;
	SlotMap[Entity2] = Slot1;

	Swap(Entities[Slot1], Entities[Slot2]);
	Swap(EntityPosition[Slot1], EntityPosition[Slot2]);
	Swap(EntityFlags[Slot1], EntityFlags[Slot2]);
	Swap(ActiveTimestamp[Slot1], ActiveTimestamp[Slot2]);
}

void FInterestManager::ComputeInterest(ISpatialOSWorker& Connection, const TArray<Worker_EntityId> Workers, const TArray<FBox2D>& Regions)
{
	SCOPE_CYCLE_COUNTER(STAT_InterestManagerComputation);

	if (Entities.Num() == 0)
	{
		return;
	}

	const double InactiveTime = 5.0;
	const uint64 TimeToConsiderInactive = FPlatformTime::Cycles64() - (InactiveTime / FPlatformTime::GetSecondsPerCycle64());
	for (int32 i = 0; i < (int32)ActiveEntities; ++i)
	{
		if (ActiveTimestamp[i] < TimeToConsiderInactive)
		{
			SwapEntries(i, ActiveEntities - 1);
			--ActiveEntities;
			--i;
		}
	}

	check(Regions.Num() <= 64);

	const uint32 NumRegions = Regions.Num();
	const uint32 NumEntities = Entities.Num();

	if (CachedServerInterest[0].Num() < (int32)NumRegions)
	{
		CachedServerInterest[0].SetNum(NumRegions);
		CachedServerInterest[1].SetNum(NumRegions);
		CachedServerInterest[2].SetNum(NumRegions);
	}

	if (Broadphase)
	{
		SCOPE_CYCLE_COUNTER(STAT_InterestManagerComputationBroadphase);
		const uint32 NumActiveEntities = ActiveEntities;

		uint64* VisibilityPtr = Visibility.GetData();
		const uint32* Flags = EntityFlags.GetData();
		for (uint32 i = 0; i < NumActiveEntities; ++i)
		{
			const uint64 Mask = UINT64_MAX;
			*VisibilityPtr = ((uint64)(*Flags != 0)) * Mask;
			++VisibilityPtr;
			++Flags;
		}

		Broadphase->ComputeBoxVisibility(Regions, Visibility);
	}
	else
	{
		SCOPE_CYCLE_COUNTER(STAT_InterestManagerComputationBox);
		const FVector2D* Pos = EntityPosition.GetData();
		const uint32* Flags = EntityFlags.GetData();
		uint64* VisibilityPtr = Visibility.GetData();

		const uint32 NumActiveEntities = ActiveEntities;

		for (uint32 i = 0; i < NumActiveEntities; ++i)
		{
			*VisibilityPtr = 0;

			for (uint32 j = 0; j < NumRegions; ++j)
			{
				const uint64 Mask = 1ull << j;

				*VisibilityPtr |= ((uint64)(*Flags != 0)) * Mask;
				const FBox2D& Box = Regions[j];
				*VisibilityPtr |= ((uint64)Box.IsInside(*Pos)) * Mask;
			}
			++Pos;
			++Flags;
			++VisibilityPtr;
		}
	}

	{
		SCOPE_CYCLE_COUNTER(STAT_InterestManagerComputationSort);

		const uint64* VisibilityPtr = Visibility.GetData();
		for (uint32 j = 0; j < NumRegions; ++j)
		{
			Swap(CachedServerInterest[2][j], CachedServerInterest[0][j]);
			CachedServerInterest[0][j].SetNum(0, false);
		}
		for (uint32 i = 0; i < NumEntities; ++i)
		{
			uint64 VisMask = *VisibilityPtr;
			while (VisMask != 0)
			{
				const uint32 j = _tzcnt_u64(VisMask);
				const uint64 Mask = 1ull << j;
				CachedServerInterest[0][j].Add(Entities[i]);
				VisMask &= ~Mask;
			}
			++VisibilityPtr;
		}
		for (uint32 j = 0; j < NumRegions; ++j)
		{
			auto& CurVisible = CachedServerInterest[0][j];
			auto& PrevVisible = CachedServerInterest[2][j];
			auto& Added = CachedServerInterest[1][j];
			Added.SetNum(0, false);
			CurVisible.Sort();
			int32 Removed = 0;
			int32 CursorPrev = 0;
			int32 CursorCur = 0;

			const int32 NumCur = CurVisible.Num();
			const int32 NumPrev = PrevVisible.Num();

			while (CursorCur < NumCur || CursorPrev < NumPrev)
			{
				if (CursorCur == NumCur || (CursorPrev < NumPrev && CurVisible[CursorCur] > PrevVisible[CursorPrev]))
				{
					PrevVisible[Removed] = PrevVisible[CursorPrev];
					++Removed;
					++CursorPrev;
					continue;
				}
				if (CursorPrev == NumPrev || (CursorCur < NumCur && CurVisible[CursorCur] < PrevVisible[CursorPrev]))
				{
					Added.Add(CurVisible[CursorCur]);
					++CursorCur;
					continue;
				}
				if (CurVisible[CursorCur] == PrevVisible[CursorPrev])
				{
					++CursorCur;
					++CursorPrev;
				}
			}
			PrevVisible.SetNum(Removed);
		}
	}

	{
		for (uint32 j = 0; false && j < NumRegions; ++j)
		{
			auto& Added = CachedServerInterest[1][j];
			auto& Removed = CachedServerInterest[2][j];

			const int32 NumAdded = Added.Num();
			const int32 NumRemoved = Removed.Num();

			if (NumAdded > 0 || NumRemoved > 0)
			{
				ChangeInterestRequest Request;
				Request.SystemEntityId = Workers[j];
				Request.bOverwrite = false;

				if (NumAdded > 0)
				{
					ChangeInterestQuery QueryAdd;
					QueryAdd.Components = InterestF.GetServerNonAuthInterestResultType().ComponentIds;
					QueryAdd.ComponentSets = InterestF.GetServerNonAuthInterestResultType().ComponentSetsIds;
					QueryAdd.Entities = MoveTemp(Added);
					Request.QueriesToAdd.Add(MoveTemp(QueryAdd));
				}

				if (NumRemoved > 0)
				{
					ChangeInterestQuery QueryRemove;
					QueryRemove.Components = InterestF.GetServerNonAuthInterestResultType().ComponentIds;
					QueryRemove.ComponentSets = InterestF.GetServerNonAuthInterestResultType().ComponentSetsIds;
					QueryRemove.Entities = MoveTemp(Removed);
					Request.QueriesToRemove.Add(MoveTemp(QueryRemove));
				}

				Request.SendRequest(Connection);
			}
		}
	}
}
} // namespace SpatialGDK
