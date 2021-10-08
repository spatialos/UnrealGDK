#include "LoadBalancing/InterestManager.h"
#include "Schema/ChangeInterest.h"
#include "SpatialView/SpatialOSWorker.h"
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

#include <immintrin.h>

DECLARE_CYCLE_STAT(TEXT("InterestManagerCompute"), STAT_InterestManagerComputation, STATGROUP_SpatialNet);
DECLARE_CYCLE_STAT(TEXT("InterestManagerComputeBroadphase"), STAT_InterestManagerComputationBroadphase, STATGROUP_SpatialNet);
DECLARE_CYCLE_STAT(TEXT("InterestManagerComputeBox"), STAT_InterestManagerComputationBox, STATGROUP_SpatialNet);
DECLARE_CYCLE_STAT(TEXT("InterestManagerComputeBoxSSE"), STAT_InterestManagerComputationBoxSSE, STATGROUP_SpatialNet);
DECLARE_CYCLE_STAT(TEXT("InterestManagerComputeSort"), STAT_InterestManagerComputationSort, STATGROUP_SpatialNet);

DECLARE_CYCLE_STAT(TEXT("InterestManagerComputePlayer"), STAT_InterestManagerComputePlayer, STATGROUP_SpatialNet);
DECLARE_CYCLE_STAT(TEXT("InterestManagerComputePlayerSort"), STAT_InterestManagerComputePlayerSort, STATGROUP_SpatialNet);
DECLARE_CYCLE_STAT(TEXT("InterestManagerComputePlayerNaive"), STAT_InterestManagerComputePlayerNaive, STATGROUP_SpatialNet);
DECLARE_CYCLE_STAT(TEXT("InterestManagerComputePlayerBroadphase"), STAT_InterestManagerComputePlayerBroadphase, STATGROUP_SpatialNet);

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

	void Alloc()
	{
#ifdef HAS_BULLET_PHYSICS
		uint32 Slot = Nodes.Num();
		btVector3 pos(0.0, 0.0, 0.0);
		auto box = btDbvtVolume::FromCE(pos, btVector3(0.1, 0.1, 0.1));

		btDbvtNode* node = Tree.insert(box, (void*)Slot);
		node->data = (void*)Slot;

		int32 TreeId = Nodes.Num();

		Nodes.Push(node);
		Positions.push_back(pos);
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

	void MoveEntry(int32 SlotSrc, int32 SlotDest)
	{
#ifdef HAS_BULLET_PHYSICS
		Positions[SlotSrc], Positions[SlotDest];
		Volumes[SlotSrc], Volumes[SlotDest];
		Nodes[SlotSrc], Nodes[SlotDest];
#endif
	}

	void Remove()
	{
#ifdef HAS_BULLET_PHYSICS
		Tree.remove(Nodes.Last());
		Positions.pop_back();
		Volumes.pop_back();
		Nodes.Pop();
#endif
	}

	void ComputeBoxVisibility(const TArray<FBox2D>& Regions, uint64* OutVisibility) {}

#ifdef HAS_BULLET_PHYSICS
	struct VisitStackElem
	{
		VisitStackElem(btDbvtNode* iNode, uint32 iListEnd)
			: node(iNode)
			, objListEnd(iListEnd)
		{
		}

		btDbvtNode* node;
		uint32 objListEnd;
		int32 curChild = -1;
	};
#endif

	void ComputeVisibility_Mask(TArray<int32> Viewers, uint64* Out, float SearchRadius)
	{
#ifdef HAS_BULLET_PHYSICS
		SearchRadius *= 0.01;

		const uint32 NumViewers = Viewers.Num();

		TArray<uint32> ViewerMaskSlot;
		ViewerMaskSlot.Reserve(NumViewers);
		for (uint32 i = 0; i < NumViewers; ++i)
		{
			ViewerMaskSlot.Add(i);
		}
		int32* ViewersPtr = Viewers.GetData();
		uint32* ViewerMaskSlotPtr = ViewerMaskSlot.GetData();

		TArray<VisitStackElem> nodes;
		nodes.Reserve(64);

		nodes.Add(VisitStackElem(Tree.m_root, NumViewers));

		btVector3* posArray = &Positions[0];
		btDbvtVolume* volumesArray = &Volumes[0];

		// btVector3 searchOffset = TO_BTVECT(iForwardOffset);
		// float inflateRadius = iRadiusSearch + searchOffset.length();
		float const sqRadius = SearchRadius * SearchRadius;
		;

		while (nodes.Num() > 0)
		{
			auto& curNode = nodes.Last();
			if (curNode.curChild == 1 || curNode.objListEnd <= 0)
			{
				nodes.Pop();
			}
			else
			{
				curNode.curChild++;

				btDbvtNode* child = curNode.node->childs[curNode.curChild];

				btDbvtVolume nodeExpandedVolume = child->volume;
				nodeExpandedVolume.Expand(btVector3(SearchRadius, SearchRadius, SearchRadius));

				if (!child->isleaf())
				{
					// Cull object list by children expanded volume
					nodes.Add(VisitStackElem(child, curNode.objListEnd));

					auto& stackElem = nodes.Last();

					for (int i = 0; i < (int)stackElem.objListEnd; ++i)
					{
						btDbvtVolume const& objVolume = *(volumesArray + ViewersPtr[i]);
						if (!Intersect(nodeExpandedVolume, objVolume))
						{
							Swap(ViewersPtr[i], ViewersPtr[stackElem.objListEnd - 1]);
							Swap(ViewerMaskSlot[i], ViewerMaskSlot[stackElem.objListEnd - 1]);
							--stackElem.objListEnd;
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
					Slot.ptr = child->data;
					uint32 otherObj = Slot.u32;
					for (uint32 i = 0; i < curNode.objListEnd; ++i)
					{
						uint32 curObject = ViewersPtr[i];
						if (curObject != otherObj && posArray[curObject].distance2(posArray[otherObj]) < sqRadius)
						{
							Out[otherObj] |= 1ull << ViewerMaskSlotPtr[i];
						}
					}
				}
			}
		}
#endif
	}

	void ComputeVisibility(TArray<int32> Viewers, TArray<TArray<uint32>>& Out, float SearchRadius)
	{
#ifdef HAS_BULLET_PHYSICS

		SearchRadius *= 0.01;

		const uint32 NumViewers = Viewers.Num();
		int32* ViewersPtr = Viewers.GetData();

		TArray<VisitStackElem> nodes;
		nodes.Reserve(64);

		nodes.Add(VisitStackElem(Tree.m_root, NumViewers));

		btVector3* posArray = &Positions[0];
		btDbvtVolume* volumesArray = &Volumes[0];

		// btVector3 searchOffset = TO_BTVECT(iForwardOffset);
		// float inflateRadius = iRadiusSearch + searchOffset.length();
		float const sqRadius = SearchRadius * SearchRadius;
		;

		while (nodes.Num() > 0)
		{
			auto& curNode = nodes.Last();
			if (curNode.curChild == 1 || curNode.objListEnd <= 0)
			{
				nodes.Pop();
			}
			else
			{
				curNode.curChild++;
				btDbvtNode* child = curNode.node->childs[curNode.curChild];

				btDbvtVolume nodeExpandedVolume = child->volume;
				nodeExpandedVolume.Expand(btVector3(SearchRadius, SearchRadius, SearchRadius));

				if (!child->isleaf() && curNode.objListEnd > 0)
				{
					// Cull object list by children expanded volume
					nodes.Add(VisitStackElem(child, curNode.objListEnd));

					auto& stackElem = nodes.Last();

					for (int i = 0; i < (int)stackElem.objListEnd; ++i)
					{
						btDbvtVolume const& objVolume = *(volumesArray + ViewersPtr[i]);
						if (!Intersect(nodeExpandedVolume, objVolume))
						{
							Swap(ViewersPtr[i], ViewersPtr[stackElem.objListEnd - 1]);
							--stackElem.objListEnd;
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
					Slot.ptr = child->data;
					uint32 otherObj = Slot.u32;
					for (uint32 i = 0; i < curNode.objListEnd; ++i)
					{
						uint32 curObject = ViewersPtr[i];
						if (curObject != otherObj && posArray[curObject].distance2(posArray[otherObj]) < sqRadius)
						{
							Out[curObject].Add(otherObj);
						}
					}
				}
			}
		}
#endif
	}

#ifdef HAS_BULLET_PHYSICS
	btDbvt Tree;
	TArray<btDbvtNode*> Nodes;
	btAlignedObjectArray<btVector3> Positions;
	btAlignedObjectArray<btDbvtVolume> Volumes;
#endif
};

void SendPartitionRequest(FCommandsHandler& CommandsHandler, Worker_EntityId Entity, ISpatialOSWorker& Connection,
						  TFunction<void(const Worker_EntityQueryResponseOp&)> QueryResponse)
{
	Worker_ComponentId PartitionAssignmentCompId = SpatialConstants::PARTITION_COMPONENT_ID;
	Worker_EntityQuery PartitionAssignmentQuery;
	PartitionAssignmentQuery.constraint.constraint_type = WORKER_CONSTRAINT_TYPE_ENTITY_ID;
	PartitionAssignmentQuery.constraint.constraint.entity_id_constraint.entity_id = Entity;

	PartitionAssignmentQuery.snapshot_result_type_component_set_id_count = 0;
	PartitionAssignmentQuery.snapshot_result_type_component_id_count = 1;
	PartitionAssignmentQuery.snapshot_result_type_component_ids = &PartitionAssignmentCompId;

	Worker_RequestId Request = Connection.SendEntityQueryRequest(EntityQuery(PartitionAssignmentQuery));
	CommandsHandler.AddRequest(Request, QueryResponse);
};

TFunction<void(const Worker_EntityQueryResponseOp& Op)> FInterestManager::BuildQueryResponse(Worker_EntityId Entity,
																							 ISpatialOSWorker& Connection)
{
	return [this, Entity, &Connection](const Worker_EntityQueryResponseOp& Op) {
		if (Op.status_code != WORKER_STATUS_CODE_SUCCESS)
		{
			// Some error happened, no need to retry
			return;
		}
		if (Op.result_count == 0)
		{
			// PlayerController disappeared, likely player disconnected
			return;
		}
		if (Op.results[0].component_count == 0)
		{
			// Partition not assigned yet? Retry.
			return SendPartitionRequest(CommandsHandler, Entity, Connection, BuildQueryResponse(Entity, Connection));
		}
		Schema_Object* Obj = Schema_GetComponentDataFields(Op.results[0].components[0].schema_type);
		Worker_EntityId SystemWorkerEntity = Schema_GetEntityId(Obj, 1);
		if (ensureAlways(SystemWorkerEntity != 0))
		{
			PlayerInfo NewPlayer;
			NewPlayer.PlayerControllerId = Entity;
			NewPlayer.PlayerWorkerSystemEntityId = SystemWorkerEntity;
			Players.Add(Entity, NewPlayer);
		}
	};
}

FInterestManager::~FInterestManager() = default;

FInterestManager::FInterestManager(InterestFactory& InInterestF, FSpatialPositionStorage& InPositions,
								   FAlwaysRelevantStorage& InAlwaysRelevant, FServerAlwaysRelevantStorage& InServerAlwaysRelevant,
								   FPlayerControllerTagStorage& InPlayerController)
	: InterestF(InInterestF)
	, Positions(InPositions)
	, AlwaysRelevant(InAlwaysRelevant)
	, ServerAlwaysRelevant(InServerAlwaysRelevant)
	, PlayerController(InPlayerController)
{
	SlotMap.Reserve(1 << 16);
	Entities.Reserve(1 << 16);
	EntityPosition.Reserve(1 << 16);
	EntityFlags.Reserve(1 << 16);
	CachedServerVisibility.Reserve(1 << 16);
	ActiveTimestamp.Reserve(1 << 16);

#ifdef HAS_BULLET_PHYSICS
	Broadphase = MakeUnique<FBroadphaseImpl>();
#endif
}

void FInterestManager::Advance(ISpatialOSWorker& Connection, const TSet<Worker_EntityId_Key>& DeletedEntities)
{
	CommandsHandler.ProcessOps(Connection.GetWorkerMessages());

	for (Worker_EntityId Entity : DeletedEntities)
	{
		if (int32* SlotPtr = SlotMap.Find(Entity))
		{
			Remove(*SlotPtr);
			SlotMap.Remove(Entity);
		}
		Players.Remove(Entity);
	}

	for (Worker_EntityId Entity : Positions.GetModifiedEntities())
	{
		int32 Slot;
		int32* SlotPtr = SlotMap.Find(Entity);
		const FVector& Pos = Positions.GetPositions()[Entity];
		if (SlotPtr != nullptr)
		{
			Slot = *SlotPtr;
		}
		else
		{
			// Assume these flags are available immediately and never change.
			uint32 Flags = 0;
			if (AlwaysRelevant.GetObjects().Contains(Entity))
			{
				Flags |= AlwaysRelevantFlag;
			}
			if (ServerAlwaysRelevant.GetObjects().Contains(Entity))
			{
				Flags |= ServerAlwaysRelevantFlag;
			}
			if (PlayerController.GetObjects().Contains(Entity))
			{
#ifndef BENCH_INTEREST_PERF
				SendPartitionRequest(CommandsHandler, Entity, Connection, BuildQueryResponse(Entity, Connection));
#else
				PlayerInfo NewPlayer;
				NewPlayer.PlayerControllerId = Entity;
				NewPlayer.PlayerWorkerSystemEntityId = 0;
				Players.Add(Entity, NewPlayer);
#endif
			}
			Slot = Allocate();
			SlotMap.Add(Entity, Slot);
			Entities[Slot] = Entity;
			EntityFlags[Slot] = Flags;
		}

		EntityPosition[Slot] = FVector2D(Pos.X, Pos.Y);
		EntityPositionX[Slot] = Pos.X;
		EntityPositionY[Slot] = Pos.Y;
		ActiveTimestamp[Slot] = FPlatformTime::Cycles64();

		if (Broadphase)
		{
			Broadphase->Update(Slot, Pos);
		}

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
	EntityPositionX.AddDefaulted();
	EntityPositionY.AddDefaulted();
	EntityFlags.AddDefaulted();
	CachedServerVisibility.AddDefaulted();
	ActiveTimestamp.AddDefaulted();
	VisibilityScratchpad.AddDefaulted();

	if (Broadphase)
	{
		Broadphase->Alloc();
	}

	++ActiveEntities;

	int32 NumEntities = Entities.Num();
	if (ActiveEntities != NumEntities)
	{
		MoveEntry(ActiveEntities - 1, NumEntities - 1);
	}

	return ActiveEntities - 1;
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
	EntityPositionX.Pop(false);
	EntityPositionY.Pop(false);
	EntityFlags.Pop(false);
	CachedServerVisibility.Pop(false);
	VisibilityScratchpad.Pop(false);

	if (Broadphase)
	{
		Broadphase->Remove();
	}
}

void FInterestManager::MoveEntry(int32 SlotSrc, int32 SlotDest)
{
	Worker_EntityId EntitySrc = Entities[SlotSrc];

	SlotMap[EntitySrc] = SlotDest;

	Entities[SlotDest] = Entities[SlotSrc];
	EntityPosition[SlotDest] = EntityPosition[SlotSrc];
	EntityPositionX[SlotDest] = EntityPositionX[SlotSrc];
	EntityPositionY[SlotDest] = EntityPositionY[SlotSrc];
	EntityFlags[SlotDest] = EntityFlags[SlotSrc];
	ActiveTimestamp[SlotDest] = ActiveTimestamp[SlotSrc];
	CachedServerVisibility[SlotDest] = CachedServerVisibility[SlotSrc];

	if (Broadphase)
	{
		Broadphase->MoveEntry(SlotSrc, SlotDest);
	}
}

void FInterestManager::SwapEntries(int32 Slot1, int32 Slot2)
{
	Worker_EntityId Entity1 = Entities[Slot1];
	Worker_EntityId Entity2 = Entities[Slot2];

	SlotMap[Entity1] = Slot2;
	SlotMap[Entity2] = Slot1;

	Swap(Entities[Slot1], Entities[Slot2]);
	Swap(EntityPosition[Slot1], EntityPosition[Slot2]);
	Swap(EntityPositionX[Slot1], EntityPositionX[Slot2]);
	Swap(EntityPositionY[Slot1], EntityPositionY[Slot2]);
	Swap(EntityFlags[Slot1], EntityFlags[Slot2]);
	Swap(ActiveTimestamp[Slot1], ActiveTimestamp[Slot2]);
	Swap(CachedServerVisibility[Slot1], CachedServerVisibility[Slot2]);

	if (Broadphase)
	{
		Broadphase->SwapEntries(Slot1, Slot2);
	}
}

void BuildDiffArrays(const TArray<Worker_EntityId_Key>& CurVisible, TArray<Worker_EntityId_Key>& PrevVisible,
					 TArray<Worker_EntityId_Key>& Added)
{
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

uint64 GetRegionMask(uint8 NumRegions)
{
	// To handle the 64 regions case, shifting by 64 could be a nop;
	uint64 Mask = 1ull << (NumRegions - 1);
	Mask <<= 1;
	return Mask - 1ull;
}

void FInterestManager::ComputeInterest(ISpatialOSWorker& Connection, const TArray<Worker_EntityId> Workers, const TArray<FBox2D>& Regions)
{
	SCOPE_CYCLE_COUNTER(STAT_InterestManagerComputation);

	if (Entities.Num() == 0)
	{
		return;
	}
	if (Regions.Num() == 0)
	{
		return;
	}

	// Set an entity that has not moved since 5 secs as inactive. It will not be checked against the regions.
	// This assumes that the bits allocation against workers remains constant, which is not valid anymore when changing/reassigning regions.
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

	constexpr bool bUseBroadphaseForRegions = false;

	if (Broadphase && bUseBroadphaseForRegions)
	{
		// 60ms, 200k entities, 64 regions, 5% overlap
		// Worse than the non-spatialized version, which is not surprising, since we are visiting all the entities anyway,
		// hence visiting the tree before testing the entities is just more data to process.
		// Have to test it for the player interest since the tree culling might actually do something in that case.

		// Since the regions are a partition of the entire space in the server case, a better strategy would be to find the region in the
		// partition an entity belongs to and test the position against the segment boundaries with the interest border. The region's
		// connectivity would be needed to interpret these results.
		SCOPE_CYCLE_COUNTER(STAT_InterestManagerComputationBroadphase);
		const uint32 NumActiveEntities = ActiveEntities;

		uint64* VisibilityPtr = CachedServerVisibility.GetData();
		const uint32* Flags = EntityFlags.GetData();
		for (uint32 i = 0; i < NumActiveEntities; ++i)
		{
			const uint64 Mask = UINT64_MAX;
			*VisibilityPtr = ((uint64)(*Flags != 0)) * Mask;
			++VisibilityPtr;
			++Flags;
		}

		Broadphase->ComputeBoxVisibility(Regions, CachedServerVisibility.GetData());
	}

	constexpr bool bVectorizeRegions = true;
	constexpr bool bVectorizeEntities = false;

	if (!bVectorizeRegions && !bVectorizeEntities)
	{
		// Not-so naively iterate over the region
		// 24ms, 200k entities, 64 regions
		SCOPE_CYCLE_COUNTER(STAT_InterestManagerComputationBox);
		const FVector2D* Pos = EntityPosition.GetData();
		const uint32* Flags = EntityFlags.GetData();
		uint64* VisibilityPtr = CachedServerVisibility.GetData();

		const uint32 NumActiveEntities = ActiveEntities;

		for (uint32 i = 0; i < NumActiveEntities; ++i)
		{
			const uint64 AllRegionsMask = UINT64_MAX;
			*VisibilityPtr = ((uint64)(*Flags != 0)) * AllRegionsMask;

			// Avoid using operator[] as the range-check could expand to a sizable chunk of code.
			const FBox2D* RegionBox = Regions.GetData();

			for (uint32 j = 0; j < NumRegions; ++j)
			{
				const uint64 Mask = 1ull << j;
				// Setting individual bits is useful when doing a batch test over several regions in one go
				*VisibilityPtr |= ((uint64)RegionBox->IsInside(*Pos)) * Mask;
				++RegionBox;
			}
			++Pos;
			++Flags;
			++VisibilityPtr;
		}
	}

	const uint64 AllRegionsMask = GetRegionMask(NumRegions);

	if (bVectorizeRegions)
	{
		SCOPE_CYCLE_COUNTER(STAT_InterestManagerComputationBoxSSE);

		// SSE version, processing 4 regions at a time.
		// 6ms, 200k entities, 64 regions
		const uint32 Num4Regions = (NumRegions + 3) / 4;
		const uint32 NextMultipleOf4 = Num4Regions * 4;

		TArray<float, TAlignedHeapAllocator<16>> BoxesMinX;
		TArray<float, TAlignedHeapAllocator<16>> BoxesMinY;
		TArray<float, TAlignedHeapAllocator<16>> BoxesMaxX;
		TArray<float, TAlignedHeapAllocator<16>> BoxesMaxY;
		BoxesMinX.Reserve(NextMultipleOf4);
		BoxesMinY.Reserve(NextMultipleOf4);
		BoxesMaxX.Reserve(NextMultipleOf4);
		BoxesMaxY.Reserve(NextMultipleOf4);
		for (uint32 j = 0; j < NumRegions; ++j)
		{
			BoxesMinX.Add(Regions[j].Min.X);
			BoxesMinY.Add(Regions[j].Min.Y);
			BoxesMaxX.Add(Regions[j].Max.X);
			BoxesMaxY.Add(Regions[j].Max.Y);
		}
		for (uint32 j = NumRegions; j < NextMultipleOf4; ++j)
		{
			BoxesMinX.Add(0);
			BoxesMinY.Add(0);
			BoxesMaxX.Add(0);
			BoxesMaxY.Add(0);
		}

		const uint32* Flags = EntityFlags.GetData();
		uint64* VisibilityPtr = CachedServerVisibility.GetData();
		const FVector2D* Pos = EntityPosition.GetData();
		for (uint32 i = 0; i < NumEntities; ++i)
		{
			*VisibilityPtr = ((uint64)(*Flags != 0)) * AllRegionsMask;

			const float* BoxesMinXPtr = BoxesMinX.GetData();
			const float* BoxesMinYPtr = BoxesMinY.GetData();
			const float* BoxesMaxXPtr = BoxesMaxX.GetData();
			const float* BoxesMaxYPtr = BoxesMaxY.GetData();

			VectorRegister RegisterX = VectorSetFloat1(Pos->X);
			VectorRegister RegisterY = VectorSetFloat1(Pos->Y);

			for (uint32 j = 0; j < NextMultipleOf4 / 4; ++j)
			{
				VectorRegister BoxTest = VectorLoadAligned(BoxesMinXPtr);
				VectorRegister TestResult = VectorCompareGT(RegisterX, BoxTest);

				BoxTest = VectorLoadAligned(BoxesMaxXPtr);
				TestResult = VectorBitwiseAnd(TestResult, VectorCompareGT(BoxTest, RegisterX));

				BoxTest = VectorLoadAligned(BoxesMinYPtr);
				TestResult = VectorBitwiseAnd(TestResult, VectorCompareGT(RegisterY, BoxTest));

				BoxTest = VectorLoadAligned(BoxesMaxYPtr);
				TestResult = VectorBitwiseAnd(TestResult, VectorCompareGT(BoxTest, RegisterY));

				uint64 Mask = VectorMaskBits(TestResult);
				*VisibilityPtr |= Mask << (j * 4);

				BoxesMinXPtr += 4;
				BoxesMinYPtr += 4;
				BoxesMaxXPtr += 4;
				BoxesMaxYPtr += 4;
			}
			++Pos;
			++Flags;
			++VisibilityPtr;
		}
	}
// Disabled until we find how to make it work on linux.
#if 0
	if (bVectorizeEntities)
	{
		SCOPE_CYCLE_COUNTER(STAT_InterestManagerComputationBoxSSE);

		// SSE version, processing 8 positions at a time.
		// 4ms, 200k entities, 64 regions
		// ==> NB : Do not mix mm256 and mm128 operations
		const uint32 Num8Entities = ActiveEntities / 8;
		const uint32 Rem8Entities = ActiveEntities % 8;

		const uint32* Flags = EntityFlags.GetData();
		uint64* VisibilityPtr = Visibility.GetData();
		const float* PosX = EntityPositionX.GetData();
		const float* PosY = EntityPositionY.GetData();

		__m256i Zero256 = _mm256_set1_epi64x(0);
		__m256i One256 = _mm256_set1_epi64x(UINT64_MAX);
		__m256i AllRegions256 = _mm256_set1_epi64x(AllRegionsMask);

		for (uint32 i = 0; i < Num8Entities; ++i)
		{
			// Load flags, sign extended to 64 bits integers
			__m256i FlagsRegLo = _mm256_cvtepi32_epi64(_mm_loadu_epi32(Flags));
			__m256i FlagsRegHi = _mm256_cvtepi32_epi64(_mm_loadu_epi32(Flags + 4));

			// Visibility flags for the next 8 entities.
			// Compare to zero to check if any bit is set (could be a different mask).
			__m256i VisibilityRegLo = _mm256_cmpeq_epi64(Zero256, FlagsRegLo);
			__m256i VisibilityRegHi = _mm256_cmpeq_epi64(Zero256, FlagsRegHi);

			// Xor the result to get a full mask if any bit was set.
			VisibilityRegLo = _mm256_xor_si256(One256, VisibilityRegLo);
			VisibilityRegLo = _mm256_and_si256(AllRegions256, VisibilityRegLo);
			VisibilityRegHi = _mm256_xor_si256(One256, VisibilityRegHi);
			VisibilityRegHi = _mm256_and_si256(AllRegions256, VisibilityRegHi);

			const FBox2D* RegionBox = Regions.GetData();

			// Load the X and Y coordinates of the next 8 entities.
			__m256 RegisterX = _mm256_load_ps(PosX);
			__m256 RegisterY = _mm256_load_ps(PosY);

			for (uint32 j = 0; j < NumRegions; ++j)
			{
				const uint64 Mask = 1ull << j;

				__m256 BoxTest = _mm256_set1_ps(RegionBox->Min.X);
				__m256 TestResult = _mm256_cmp_ps(RegisterX, BoxTest, _CMP_GT_OQ);

				BoxTest = _mm256_set1_ps(RegionBox->Max.X);
				TestResult = _mm256_and_ps(_mm256_cmp_ps(BoxTest, RegisterX, _CMP_GT_OQ), TestResult);

				BoxTest = _mm256_set1_ps(RegionBox->Min.Y);
				TestResult = _mm256_and_ps(_mm256_cmp_ps(RegisterY, BoxTest, _CMP_GT_OQ), TestResult);

				BoxTest = _mm256_set1_ps(RegionBox->Max.Y);
				TestResult = _mm256_and_ps(_mm256_cmp_ps(BoxTest, RegisterY, _CMP_GT_OQ), TestResult);

				// The test result is 0xFFFFFFFF for each entity which is in the box.
				// Convert it to 64bit, once again with sign extension, to get UINT64_MAX or 0 in order to AND it with the region's mask
				__m256i TestResult256Lo = _mm256_cvtepi32_epi64(_mm256_extracti128_si256(_mm256_castps_si256(TestResult), 0));
				__m256i TestResult256Hi = _mm256_cvtepi32_epi64(_mm256_extracti128_si256(_mm256_castps_si256(TestResult), 1));

				// Combine the current result with the entity's visibility mask
				VisibilityRegLo = _mm256_or_si256(VisibilityRegLo, _mm256_and_si256(_mm256_set1_epi64x(Mask), TestResult256Lo));
				VisibilityRegHi = _mm256_or_si256(VisibilityRegHi, _mm256_and_si256(_mm256_set1_epi64x(Mask), TestResult256Hi));

				++RegionBox;
			}
			// Store the result.
			_mm256_storeu_epi64(VisibilityPtr, VisibilityRegLo);
			_mm256_storeu_epi64(VisibilityPtr + 4, VisibilityRegHi);

			PosX += 8;
			PosY += 8;
			Flags += 8;
			VisibilityPtr += 8;
		}

		// Handle the tail entities.
		const FVector2D* Pos = EntityPosition.GetData() + 8 * Num8Entities;
		for (uint32 i = 0; i < Rem8Entities; ++i)
		{
			*VisibilityPtr = ((uint64)(*Flags != 0)) * AllRegionsMask;

			const FBox2D* RegionBox = Regions.GetData();

			for (uint32 j = 0; j < NumRegions; ++j)
			{
				const uint64 Mask = 1ull << j;
				*VisibilityPtr |= ((uint64)RegionBox->IsInside(*Pos)) * Mask;

				++RegionBox;
			}
			++Pos;
			++Flags;
			++VisibilityPtr;
		}
	}
#endif
	{
		// 8ms 200k entities, 64 regions, 5% overlap
		SCOPE_CYCLE_COUNTER(STAT_InterestManagerComputationSort);

		const uint64* VisibilityPtr = CachedServerVisibility.GetData();

		// Reset region visibility.
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
				// _tzcnt_u64 -> Count trailing zeros, <==> position of the first set bit
				// equivalent of peeling one set region at a time
				const uint32 j = _tzcnt_u64(VisMask);
				CachedServerInterest[0][j].Add(Entities[i]);
				VisMask &= ~(1ull << j);
			}
			++VisibilityPtr;
		}

		for (uint32 j = 0; j < NumRegions; ++j)
		{
			// If several regions are assigned to a single worker, the lists need to be merged and
			// duplicates eliminated before computing the add/remove lists.

			auto& CurVisible = CachedServerInterest[0][j];
			auto& PrevVisible = CachedServerInterest[2][j];
			auto& Added = CachedServerInterest[1][j];
			Added.SetNum(0, false);
			CurVisible.Sort();
			BuildDiffArrays(CurVisible, PrevVisible, Added);
		}
	}

#ifndef BENCH_INTEREST_PERF
	{
		for (uint32 j = 0; j < NumRegions; ++j)
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
#endif
}

void FInterestManager::ComputePlayersInterest(ISpatialOSWorker& Connection, float InterestRadius, float NNInterestRadius)
{
	SCOPE_CYCLE_COUNTER(STAT_InterestManagerComputePlayer);
	if (Entities.Num() == 0)
	{
		return;
	}
	if (Players.Num() == 0)
	{
		return;
	}

	TArray<int32> PlayerSlots;
	PlayerSlots.Reserve(Players.Num());

	TArray<TArray<Worker_EntityId_Key>*> PlayerArrays;

	const uint32 NumPlayers = Players.Num();

	for (auto& PlayerEntry : Players)
	{
		PlayerInfo& Player = PlayerEntry.Value;
		Swap(Player.CachedInterest[2], Player.CachedInterest[0]);
		Player.CachedInterest[0].SetNum(0, false);
		PlayerArrays.Add(&Player.CachedInterest[0]);
		PlayerSlots.Add(SlotMap.FindChecked(Player.PlayerControllerId));
	}

	if (Broadphase)
	{
		SCOPE_CYCLE_COUNTER(STAT_InterestManagerComputePlayerBroadphase);
		if (false)
		{
			TArray<TArray<uint32>> PVisibility;
			PVisibility.SetNum(NumPlayers);

			// 50 ms, 210k entities, 1k players, 2.10e5 area square, 2.10e3 interest radius
			// That's approx 64 entities per player.
			Broadphase->ComputeVisibility(PlayerSlots, PVisibility, InterestRadius);

			// The always relevant entities are missing though

			uint32 PlayerIndex = 0;
			TArray<uint32>* SeenSlots = PVisibility.GetData();
			for (auto& PlayerEntry : Players)
			{
				PlayerInfo& Player = PlayerEntry.Value;
				for (auto Slot : *SeenSlots)
				{
					Player.CachedInterest[0].Add(Entities[Slot]);
				}
				++SeenSlots;
			}
		}

		const float RadiusSq = InterestRadius * InterestRadius;
		const uint32 NumEntities = Entities.Num();

		const uint32 NumPlayers64 = (NumPlayers + 63) / 64;
		const int32* PlayerSlotsPtr = PlayerSlots.GetData();
		TArray<Worker_EntityId_Key>** PlayerArraysPtr = PlayerArrays.GetData();

		for (uint32_t j = 0; j < NumPlayers64; ++j)
		{
			uint32 CurrentBegin = j * 64;
			uint32 CurrentEnd = FMath::Min(((j + 1) * 64), NumPlayers);
			const uint32 NumPlayersToProcess = CurrentEnd - CurrentBegin;

			if (true)
			{
				uint64* VisibilityPtr = VisibilityScratchpad.GetData();
				const uint32* Flags = EntityFlags.GetData();
				for (uint32 i = 0; i < NumEntities; ++i)
				{
					const uint64 Mask = UINT64_MAX;
					*VisibilityPtr = ((uint64)(*Flags & AlwaysRelevantFlag)) * Mask;
					++VisibilityPtr;
					++Flags;
				}

				VisibilityPtr = VisibilityScratchpad.GetData();

				TArray<int32> Viewers(PlayerSlotsPtr, CurrentEnd - CurrentBegin);
				Broadphase->ComputeVisibility_Mask(Viewers, VisibilityPtr, InterestRadius);
			}
			else
			{
				// 200 ms, 210k entities, 1k players, 2.10e5 area square, 2.10e3 interest radius
				const uint32 Num4Entities = NumEntities / 4;
				const uint32 Rem4Entities = NumEntities % 4;

				const uint32* Flags = EntityFlags.GetData();
				uint64* VisibilityPtr = VisibilityScratchpad.GetData();
				const float* PosX = EntityPositionX.GetData();
				const float* PosY = EntityPositionY.GetData();

				__m128i AlwaysRelevantMask = _mm_set1_epi64x(AlwaysRelevantFlag);
				__m128i AllPlayers128 = _mm_set1_epi64x(GetRegionMask(NumPlayersToProcess));

				for (uint32 i = 0; i < Num4Entities; ++i)
				{
					__m128i FlagsRegLo = _mm_cvtepi32_epi64(_mm_loadu_epi32(Flags));
					__m128i FlagsRegHi = _mm_cvtepi32_epi64(_mm_loadu_epi32(Flags + 4));

					// Visibility flags for the next 4 entities.
					// Compare to zero to check if any bit is set (could be a different mask).
					__m128i VisibilityRegLo = _mm_cmpeq_epi64(AlwaysRelevantMask, _mm_and_si128(FlagsRegLo, AlwaysRelevantMask));
					__m128i VisibilityRegHi = _mm_cmpeq_epi64(AlwaysRelevantMask, _mm_and_si128(FlagsRegHi, AlwaysRelevantMask));

					VisibilityRegLo = _mm_and_si128(AllPlayers128, VisibilityRegLo);
					VisibilityRegHi = _mm_and_si128(AllPlayers128, VisibilityRegHi);

					VectorRegister RegisterX = VectorLoadAligned(PosX);
					VectorRegister RegisterY = VectorLoadAligned(PosY);

					for (uint32 k = 0; k < NumPlayersToProcess; ++k)
					{
						const uint64 Mask = 1ull << k;
						uint32 PlayerSlot = PlayerSlotsPtr[k];
						FVector2D const& PlayerPos = EntityPosition[PlayerSlot];
						VectorRegister PlayerPosX = VectorSetFloat1(PlayerPos.X);
						VectorRegister PlayerPosY = VectorSetFloat1(PlayerPos.Y);

						VectorRegister DistX = VectorSubtract(PlayerPosX, RegisterX);
						VectorRegister DistY = VectorSubtract(PlayerPosY, RegisterY);
						DistX = VectorMultiply(DistX, DistX);
						DistY = VectorMultiply(DistY, DistY);
						VectorRegister SquaredDist = VectorAdd(DistX, DistY);
						VectorRegister TestResult = VectorCompareLT(SquaredDist, VectorSetFloat1(RadiusSq));

						__m128i TestResult256Lo = _mm_cvtepi32_epi64(_mm_set1_epi64x(_mm_extract_epi64(_mm_castps_si128(TestResult), 0)));
						__m128i TestResult256Hi = _mm_cvtepi32_epi64(_mm_set1_epi64x(_mm_extract_epi64(_mm_castps_si128(TestResult), 1)));

						// Combine the current result with the entity's visibility mask
						VisibilityRegLo = _mm_or_si128(VisibilityRegLo, _mm_and_si128(_mm_set1_epi64x(Mask), TestResult256Lo));
						VisibilityRegHi = _mm_or_si128(VisibilityRegHi, _mm_and_si128(_mm_set1_epi64x(Mask), TestResult256Hi));
					}
					_mm_storeu_epi64(VisibilityPtr, VisibilityRegLo);
					_mm_storeu_epi64(VisibilityPtr + 2, VisibilityRegHi);
					PosX += 4;
					PosY += 4;
					Flags += 4;
					VisibilityPtr += 4;
				}
			}

			for (uint32 k = 0; k < NumPlayersToProcess; ++k)
			{
				// Prevent players from checking themselves.
				VisibilityScratchpad[PlayerSlots[k]] &= ~(1ull << k);
			}

			const uint64* VisibilityPtr = VisibilityScratchpad.GetData();
			for (uint32 i = 0; i < NumEntities; ++i)
			{
				uint64 VisMask = *VisibilityPtr;
				while (VisMask != 0)
				{
					// _tzcnt_u64 -> Count trailing zeros, <==> position of the first set bit
					// equivalent of peeling one set region at a time
					const uint32 PlayerSlot = _tzcnt_u64(VisMask);
					PlayerArraysPtr[PlayerSlot]->Add(Entities[i]);
					VisMask &= ~(1ull << PlayerSlot);
				}

				++VisibilityPtr;
			}

			PlayerSlotsPtr += 64;
			PlayerArraysPtr += 64;
		}
	}

	if (false)
	{
		SCOPE_CYCLE_COUNTER(STAT_InterestManagerComputePlayerNaive);
		// 460 ms, 200k entities, 1k players, 2.10e5 area square, 2.10e2 interest radius (~1 entity/client)
		// Not workable.
		const float RadiusSq = InterestRadius * InterestRadius;

		const FVector2D* Pos = EntityPosition.GetData();
		const uint32* Flags = EntityFlags.GetData();
		const Worker_EntityId* EntityPtr = Entities.GetData();

		const uint32 NumEntities = Entities.Num();

		for (uint32 i = 0; i < NumEntities; ++i)
		{
			bool AlwaysVisible = (*Flags & AlwaysRelevantFlag) != 0;

			Worker_EntityId CurEntity = *EntityPtr;

			int32* PlayerSlotsPtr = PlayerSlots.GetData();
			for (uint32 j = 0; j < NumPlayers; ++j)
			{
				uint32 CurPlayerSlot = *PlayerSlotsPtr;
				if (CurPlayerSlot == i)
				{
					continue;
				}
				if (AlwaysVisible || FVector2D::DistSquared(EntityPosition[CurPlayerSlot], *Pos) < RadiusSq)
				{
					PlayerArrays[j]->Add(CurEntity);
				}

				++PlayerSlotsPtr;
			}
			++Pos;
			++Flags;
			++EntityPtr;
		}
	}

	for (auto& PlayerEntry : Players)
	{
		// 2.3 ms, 1k players, 64 entities per player.
		SCOPE_CYCLE_COUNTER(STAT_InterestManagerComputePlayerSort);
		PlayerInfo& Player = PlayerEntry.Value;
		auto& CurVisible = Player.CachedInterest[0];
		auto& PrevVisible = Player.CachedInterest[2];
		auto& Added = Player.CachedInterest[1];
		Added.SetNum(0, false);
		CurVisible.Sort();
		BuildDiffArrays(CurVisible, PrevVisible, Added);
	}
#ifndef BENCH_INTEREST_PERF
	for (auto& PlayerEntry : Players)
	{
		PlayerInfo& Player = PlayerEntry.Value;
		auto& Added = Player.CachedInterest[1];
		auto& Removed = Player.CachedInterest[2];

		const int32 NumAdded = Added.Num();
		const int32 NumRemoved = Removed.Num();

		if (NumAdded > 0 || NumRemoved > 0)
		{
			ChangeInterestRequest Request;
			Request.SystemEntityId = Player.PlayerWorkerSystemEntityId;
			Request.bOverwrite = false;

			if (NumAdded > 0)
			{
				ChangeInterestQuery QueryAdd;
				QueryAdd.Components = InterestF.GetClientNonAuthInterestResultType().ComponentIds;
				QueryAdd.ComponentSets = InterestF.GetClientNonAuthInterestResultType().ComponentSetsIds;
				QueryAdd.Entities = MoveTemp(Added);
				Request.QueriesToAdd.Add(MoveTemp(QueryAdd));
			}

			if (NumRemoved > 0)
			{
				ChangeInterestQuery QueryRemove;
				QueryRemove.Components = InterestF.GetClientNonAuthInterestResultType().ComponentIds;
				QueryRemove.ComponentSets = InterestF.GetClientNonAuthInterestResultType().ComponentSetsIds;
				QueryRemove.Entities = MoveTemp(Removed);
				Request.QueriesToRemove.Add(MoveTemp(QueryRemove));
			}

			Request.SendRequest(Connection);
		}
	}
#endif
}

} // namespace SpatialGDK
