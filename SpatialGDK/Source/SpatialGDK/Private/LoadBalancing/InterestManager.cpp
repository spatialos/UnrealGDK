#include "LoadBalancing/InterestManager.h"
#include "Schema/ChangeInterest.h"
#include "Utils/InterestFactory.h"

// Try to grab improbable's phTree thingy

#include <algorithm>
#include <cmath>

#include <immintrin.h>

DECLARE_CYCLE_STAT(TEXT("InterestManagerCompute"), STAT_InterestManagerComputation, STATGROUP_SpatialNet);
DECLARE_CYCLE_STAT(TEXT("InterestManagerComputeBroadphase"), STAT_InterestManagerComputationBroadphase, STATGROUP_SpatialNet);
DECLARE_CYCLE_STAT(TEXT("InterestManagerComputeBox"), STAT_InterestManagerComputationBox, STATGROUP_SpatialNet);
DECLARE_CYCLE_STAT(TEXT("InterestManagerComputeBoxSSE"), STAT_InterestManagerComputationBoxSSE, STATGROUP_SpatialNet);
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

	void Add(uint32 Slot, FVector Position) {}

	void Add(uint32 Slot, FBox2D Box) {}

	void Update(uint32 Slot, FVector Position) {}

	void SwapEntries(int32 Slot1, int32 Slot2) {}

	void Remove(uint32 Slot) {}

	void ComputeBoxVisibility(const TArray<FBox2D>& Regions, uint64* OutVisibility) {}
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
		EntityPositionX[Slot] = Pos.X;
		EntityPositionY[Slot] = Pos.Y;
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
	EntityPositionX.AddDefaulted();
	EntityPositionY.AddDefaulted();
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
	Visibility.Pop(false);
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
	Visibility[SlotDest] = Visibility[SlotSrc];
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
	Swap(Visibility[Slot1], Visibility[Slot2]);

	if (Broadphase)
	{
		Broadphase->SwapEntries(Slot1, Slot2);
	}
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

		uint64* VisibilityPtr = Visibility.GetData();
		const uint32* Flags = EntityFlags.GetData();
		for (uint32 i = 0; i < NumActiveEntities; ++i)
		{
			const uint64 Mask = UINT64_MAX;
			*VisibilityPtr = ((uint64)(*Flags != 0)) * Mask;
			++VisibilityPtr;
			++Flags;
		}

		Broadphase->ComputeBoxVisibility(Regions, Visibility.GetData());
	}

	constexpr bool bVectorizeRegions = false;
	constexpr bool bVectorizeEntities = true;

	if (!bVectorizeRegions && !bVectorizeEntities)
	{
		// Not-so naively iterate over the region
		// 24ms, 200k entities, 64 regions
		SCOPE_CYCLE_COUNTER(STAT_InterestManagerComputationBox);
		const FVector2D* Pos = EntityPosition.GetData();
		const uint32* Flags = EntityFlags.GetData();
		uint64* VisibilityPtr = Visibility.GetData();

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

	const uint64 AllRegionsMask = [NumRegions] {
		// To handle the 64 regions case, shifting by 64 could be a nop;
		uint64 Mask = 1ull << (NumRegions - 1);
		Mask <<= 1;
		return Mask - 1ull;
	}();

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
		uint64* VisibilityPtr = Visibility.GetData();
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

	{
		// 8ms 200k entities, 64 regions, 5% overlap
		SCOPE_CYCLE_COUNTER(STAT_InterestManagerComputationSort);

		const uint64* VisibilityPtr = Visibility.GetData();

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
}
} // namespace SpatialGDK
