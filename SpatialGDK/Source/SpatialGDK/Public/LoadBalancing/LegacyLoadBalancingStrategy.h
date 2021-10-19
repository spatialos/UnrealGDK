// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "Interop/SkeletonEntities.h"
#include "Interop/SpatialCommandsHandler.h"
#include "Interop/Startup/SpatialStartupCommon.h"
#include "LoadBalancing/AbstractLBStrategy.h"
#include "LoadBalancing/LoadBalancingStrategy.h"

class SpatialVirtualWorkerTranslator;

namespace SpatialGDK
{
class FSpatialPositionStorage;
class FActorGroupStorage;
class FDirectAssignmentStorage;
class FDebugComponentStorage;
class FCustomWorkerAssignmentStorage;
class FActorSetSystem;

class FSkeletonEntityLoadBalancing
{
public:
	explicit FSkeletonEntityLoadBalancing(FLoadBalancingSharedData InSharedData, TFunction<int32(FPartitionHandle)> InPartitionToId)
		: SharedData(InSharedData)
		, PartitionToId(InPartitionToId)
	{
	}
	void OnSkeletonManifestReceived(Worker_EntityId EntityId, FSkeletonEntityManifest ManifestData,
									const TMap<Worker_EntityId_Key, int32>& EntityToWorkerMapping);
	void Flush(ISpatialOSWorker& Connection, const TMap<int32, Worker_PartitionId>& PartitionIds);
	void ApplySkeletonsToMigrate(FMigrationContext& Ctx);

private:
	struct ManifestProcessing
	{
		FSkeletonEntityManifest ManifestData;
		TMap<int32, TSet<Worker_EntityId_Key>> InProgressManifests;
		TArray<FManifestCreationHandle> PublishedManifests;
		uint32 ProcessedEntities = 0;
	};

	TMap<Worker_EntityId_Key, ManifestProcessing> ReceivedManifests;
	FLoadBalancingSharedData SharedData;
	TFunction<int32(FPartitionHandle)> PartitionToId;
};

class FLegacyLoadBalancing : public FLoadBalancingStrategy
{
public:
	FLegacyLoadBalancing(UAbstractLBStrategy& LegacyLBStrat, SpatialVirtualWorkerTranslator& InTranslator);
	~FLegacyLoadBalancing();

	virtual void Init(ISpatialOSWorker& Connection, FLoadBalancingSharedData InSharedData, TArray<FLBDataStorage*>& OutLoadBalancingData,
					  TArray<FLBDataStorage*>& OutServerWorkerData) override;

	virtual void Advance(ISpatialOSWorker& Connection) override;
	virtual void Flush(ISpatialOSWorker& Connection) override;
	virtual bool IsReady() override { return !StartupExecutor.IsSet(); }

	virtual void OnWorkersConnected(TArrayView<FLBWorkerHandle> ConnectedWorkers) override;
	virtual void OnWorkersDisconnected(TArrayView<FLBWorkerHandle> DisconnectedWorkers) override;
	virtual void TickPartitions() override;
	virtual void CollectEntitiesToMigrate(FMigrationContext& Ctx) override;
	virtual void OnSkeletonManifestReceived(Worker_EntityId, FSkeletonEntityManifest) override;

protected:
	void EvaluateDebugComponent(Worker_EntityId, FMigrationContext& Ctx);
	TOptional<TPair<Worker_EntityId, uint32>> EvaluateDebugComponentWithSet(Worker_EntityId);
	TOptional<uint32> EvaluateDebugComponent(Worker_EntityId);

	TOptional<FStartupExecutor> StartupExecutor;
	void CreateAndAssignPartitions();

	// +++ Data Storage +++
	TUniquePtr<FSpatialPositionStorage> PositionStorage;
	TUniquePtr<FActorGroupStorage> GroupStorage;
	TUniquePtr<FDirectAssignmentStorage> AssignmentStorage;
	TUniquePtr<FDebugComponentStorage> DebugCompStorage;

	TUniquePtr<FCustomWorkerAssignmentStorage> ServerWorkerCustomAssignment;
	// --- Data Storage ---

	// +++ Partition Assignment +++
	TArray<FPartitionHandle> Partitions;
	TArray<FLBWorkerHandle> VirtualWorkerIdToHandle;
	TSet<FLBWorkerHandle> ConnectedWorkers;
	FCommandsHandler CommandsHandler;
	uint32 ExpectedWorkers = 0;
	bool bCreatedPartitions = false;
	// --- Partition Assignment ---

	// +++ Load Balancing +++
	TOptional<FLoadBalancingSharedData> SharedData;
	FLegacyLBContext LBContext;
	TSet<Worker_EntityId_Key> ToRefresh;
	TMap<Worker_EntityId_Key, int32> Assignment;
	bool bDirectAssignment = false;

	Worker_EntityId WorkerForCustomAssignment = SpatialConstants::INVALID_ENTITY_ID;
	// --- Load Balancing ---

	// +++ Skeleton entity processing +++
	TOptional<FSkeletonEntityLoadBalancing> SkeletonLoadBalancing;
	// --- Skeleton entity processing ---
};

} // namespace SpatialGDK
