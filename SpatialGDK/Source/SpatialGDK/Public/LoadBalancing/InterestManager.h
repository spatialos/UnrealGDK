#pragma once

#include "Interop/SpatialCommandsHandler.h"
#include "LoadBalancing/LBDataStorage.h"

//#define BENCH_INTEREST_PERF

namespace SpatialGDK
{
class ISpatialOSWorker;
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
	Worker_ComponentId TagComponentId;
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

class FPlayerControllerTagStorage : public FTagComponentStorage
{
public:
	FPlayerControllerTagStorage()
		: FTagComponentStorage(SpatialConstants::PLAYER_CONTROLLER_COMPONENT_ID)
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
	FInterestManager(InterestFactory& InterestF, FSpatialPositionStorage& Positions, FAlwaysRelevantStorage& AlwaysRelevant,
					 FServerAlwaysRelevantStorage& ServerAlwaysRelevant, FPlayerControllerTagStorage& PlayerController);
	~FInterestManager();

	void Advance(ISpatialOSWorker& Connection, const TSet<Worker_EntityId_Key>& DeletedEntities);

	void ComputeInterest(ISpatialOSWorker& Connection, const TArray<Worker_EntityId> Workers, const TArray<FBox2D>& Regions);

	void ComputePlayersInterest(ISpatialOSWorker& Connection, float InterestRadius, float NNInterestRadius);

	FSpatialPositionStorage& GetPositions() { return Positions; }

	float PlayerInterestRadius;

protected:
	int32 Allocate();
	void Remove(int32);

	void SwapEntries(int32 Slot1, int32 Slot2);
	void MoveEntry(int32 SlotSrc, int32 SlotDest);

	InterestFactory& InterestF;

	FSpatialPositionStorage& Positions;
	FAlwaysRelevantStorage& AlwaysRelevant;
	FServerAlwaysRelevantStorage& ServerAlwaysRelevant;
	FPlayerControllerTagStorage& PlayerController;

	TMap<Worker_EntityId_Key, int32> SlotMap;

	TArray<Worker_EntityId> Entities;
	TArray<FVector2D> EntityPosition;
	TArray<float, TAlignedHeapAllocator<16>> EntityPositionX;
	TArray<float, TAlignedHeapAllocator<16>> EntityPositionY;
	TArray<uint32, TAlignedHeapAllocator<16>> EntityFlags;
	TArray<uint64, TAlignedHeapAllocator<16>> CachedServerVisibility;
	TArray<uint64, TAlignedHeapAllocator<16>> VisibilityScratchpad;
	TArray<uint64> ActiveTimestamp;
	int32 ActiveEntities = 0;

	TUniquePtr<FBroadphaseImpl> Broadphase;

	TArray<TArray<Worker_EntityId_Key>> CachedServerInterest[3];

	struct PlayerInfo
	{
		Worker_EntityId PlayerControllerId;
		Worker_EntityId PlayerWorkerSystemEntityId;
		TArray<Worker_EntityId_Key> CachedInterest[3];
	};
	TMap<Worker_EntityId_Key, PlayerInfo> Players;

	TFunction<void(const Worker_EntityQueryResponseOp& Op)> BuildQueryResponse(Worker_EntityId PlayerControllerId,
																			   ISpatialOSWorker& Connection);
	FCommandsHandler CommandsHandler;
};
} // namespace SpatialGDK
