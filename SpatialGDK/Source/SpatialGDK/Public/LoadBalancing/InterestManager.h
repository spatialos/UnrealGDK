#pragma once

#include "LoadBalancing/LBDataStorage.h"
namespace SpatialGDK
{
class InterestFactory;
class FTagComponentStorage : public FLBDataStorage
{
public:
	FTagComponentStorage(Worker_ComponentId InComponentId);

	virtual void OnComponentAdded(Worker_EntityId EntityId, Worker_ComponentId ComponentId, Schema_ComponentData* Data) override;
	virtual void OnRemoved(Worker_EntityId EntityId) override;
	virtual void OnUpdate(Worker_EntityId EntityId, Worker_ComponentId InComponentId, Schema_ComponentUpdate* Update) override;
	const TSet<Worker_EntityId_Key>& GetObjects() const { return TaggedObjects; }

protected:
	TSet<Worker_EntityId_Key> TaggedObjects;
	const Worker_ComponentId TagComponentId;
};

class FAlwaysRelevantStorage : public FTagComponentStorage
{
public:
	FAlwaysRelevantStorage()
		: FTagComponentStorage(SpatialConstants::ALWAYS_RELEVANT_COMPONENT_ID)
	{
	}
};

class FServerAlwaysRelevantStorage : public FTagComponentStorage
{
public:
	FServerAlwaysRelevantStorage()
		: FTagComponentStorage(SpatialConstants::SERVER_ONLY_ALWAYS_RELEVANT_COMPONENT_ID)
	{
	}
};

class FInterestManager
{
	static constexpr uint32 AlwaysRelevantFlag = 1 << 0;
	static constexpr uint32 ServerAlwaysRelevantFlag = 1 << 1;
	static constexpr uint32 LastActiveMask = 16;

	struct FBroadphaseImpl;

public:
	FInterestManager(const InterestFactory& InterestF, const FSpatialPositionStorage& Positions,
					 const FAlwaysRelevantStorage& AlwaysRelevant, const FServerAlwaysRelevantStorage& ServerAlwaysRelevant);
	~FInterestManager();

	void Advance(const TSet<Worker_EntityId_Key>& DeletedEntities);

	void ComputeInterest(ISpatialOSWorker& Connection, const TArray<Worker_EntityId> Workers, const TArray<FBox2D>& Regions);

	const FSpatialPositionStorage& GetPositions() { return Positions; }

protected:
	int32 Allocate();
	void Remove(int32);

	void SwapEntries(int32 Slot1, int32 Slot2);
	void MoveEntry(int32 SlotSrc, int32 SlotDest);

	const InterestFactory& InterestF;

	const FSpatialPositionStorage& Positions;
	const FAlwaysRelevantStorage& AlwaysRelevant;
	const FServerAlwaysRelevantStorage& ServerAlwaysRelevant;

	TMap<Worker_EntityId_Key, int32> SlotMap;

	TArray<Worker_EntityId> Entities;
	TArray<FVector2D> EntityPosition;
	TArray<float, TAlignedHeapAllocator<16>> EntityPositionX;
	TArray<float, TAlignedHeapAllocator<16>> EntityPositionY;
	TArray<uint32, TAlignedHeapAllocator<16>> EntityFlags;
	TArray<uint64, TAlignedHeapAllocator<16>> Visibility;
	TArray<uint64> ActiveTimestamp;
	int32 ActiveEntities = 0;

	TUniquePtr<FBroadphaseImpl> Broadphase;

	// 3 buckets - Curr Visible, Prev Visible, Newly Added
	TArray<TArray<Worker_EntityId_Key>> CachedServerInterest[3];
};
} // namespace SpatialGDK
