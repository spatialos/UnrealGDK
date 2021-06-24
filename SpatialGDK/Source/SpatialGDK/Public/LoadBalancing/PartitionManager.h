// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "LoadBalancing/LoadBalancingTypes.h"
#include "SpatialCommonTypes.h"
#include "SpatialConstants.h"
#include "SpatialView/SubView.h"

DECLARE_LOG_CATEGORY_EXTERN(LogSpatialPartitionManager, Log, All)

namespace SpatialGDK
{
struct QueryConstraint;
class ISpatialOSWorker;
class InterestFactory;
class ViewCoordinator;

class FPartitionManager
{
public:
	FPartitionManager(Worker_EntityId InStrategyWorkerEntityId, ViewCoordinator& Coordinator, TUniquePtr<InterestFactory>&& InterestF);
	~FPartitionManager();

	void Init(ISpatialOSWorker& Connection);

	bool IsReady();

	TOptional<Worker_PartitionId> GetPartitionId(FPartitionHandle);
	FPartitionHandle CreatePartition(FString DisplayName, void* UserData, const SpatialGDK::QueryConstraint& Interest);
	void SetPartitionInterest(FPartitionHandle Partition, const SpatialGDK::QueryConstraint& NewInterest);
	void AssignPartitionTo(FPartitionHandle Partition, FLBWorkerHandle Worker);
	void SetPartitionMetadata(FPartitionHandle /*???*/);

	void AdvanceView(ISpatialOSWorker& Connection);
	void Flush(ISpatialOSWorker& Connection);

	TArray<FLBWorkerHandle> GetConnectedWorkers();
	TArray<FLBWorkerHandle> GetDisconnectedWorkers();

	Worker_EntityId GetServerWorkerEntityIdForWorker(FLBWorkerHandle);

private:
	struct Impl;
	TUniquePtr<Impl> m_Impl;
};
} // namespace SpatialGDK
