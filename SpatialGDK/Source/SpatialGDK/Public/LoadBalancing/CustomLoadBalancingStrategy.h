#pragma once

#include "Algo/Copy.h"
#include "Algo/RemoveIf.h"
#include "Containers/Array.h"
#include "Interop/SpatialCommandsHandler.h"
#include "Interop/Startup/SpatialStartupCommon.h"
#include "LegacyLoadBalancingStrategy.h"
#include "LoadBalancing/LoadBalancingStrategy.h"

namespace SpatialGDK
{
class ISpatialOSWorker;

class FCustomLoadBalancingStrategy : public FLoadBalancingStrategy
{
public:
	FCustomLoadBalancingStrategy(FTicker& Ticker);
	~FCustomLoadBalancingStrategy();

	virtual void Init(ISpatialOSWorker& Connection, FLoadBalancingSharedData InSharedData, TArray<FLBDataStorage*>& OutLoadBalancingData,
					  TArray<FLBDataStorage*>& OutServerWorkerData) override;
	virtual void CollectEntitiesToMigrate(FMigrationContext& Ctx) override;

private:
	void OnEntityAdded(Worker_EntityId EntityId);

	TOptional<FLoadBalancingSharedData> SharedData;

	FPartitionHandle FindEntityPartition(const Worker_EntityId EntityId) const;
	int64 GetBucketId(FPartitionHandle Partition) const;

public:
	virtual void OnWorkersConnected(TArrayView<FLBWorkerHandle> InConnectedWorkers) override;

	virtual void OnWorkersDisconnected(TArrayView<FLBWorkerHandle> InDisconnectedWorkers) override
	{
		for (const FLBWorkerHandle& DisconnectedWorker : InDisconnectedWorkers)
		{
			ConnectedWorkers.Remove(DisconnectedWorker);
		}
	}
	virtual void Advance(ISpatialOSWorker& Connection) override;
	virtual bool IsReady() override;
	virtual void TickPartitions() override;
	virtual void OnSkeletonManifestReceived(Worker_EntityId, FSkeletonEntityManifest) override;
	virtual void Flush(ISpatialOSWorker& Connection) override;

private:
	TFunction<void()> OnDestroyed;
	FCommandsHandler CommandsHandler;
	TOptional<FStartupExecutor> Executor;
	TMap<int64, FPartitionHandle> PartitionBucketIndexToPartitionHandle;
	TArray<FLBWorkerHandle> ConnectedWorkers;
	TSet<FPartitionHandle> AssignedBucketPartitions;
	TMap<FLBWorkerHandle, FPartitionHandle> WorkerPartitions;
	int8 NextPartitionAssignmentWorkerIndex = 0;
	TSet<Worker_EntityId_Key> ToRefresh;

	TMap<Worker_EntityId_Key, int32> Assignments;

	TOptional<FSkeletonEntityLoadBalancing> SkeletonLoadBalancing;

	TUniquePtr<FSpatialPositionStorage> PositionStorage;
};
} // namespace SpatialGDK
