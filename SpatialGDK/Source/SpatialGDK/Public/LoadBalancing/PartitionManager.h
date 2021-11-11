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
	FPartitionManager(const FSubView& InServerWorkerView, Worker_EntityId InStrategyWorkerEntityId, ViewCoordinator& Coordinator,
					  TUniquePtr<InterestFactory>&& InterestF);
	~FPartitionManager();

	void Init(ISpatialOSWorker& Connection);

	bool IsReady();

	TOptional<Worker_PartitionId> GetPartitionId(FPartitionHandle);
	FPartitionHandle CreatePartition(FString DisplayName, const SpatialGDK::QueryConstraint& Interest, TArray<ComponentData> MetaData);
	void SetPartitionInterest(FPartitionHandle Partition, const SpatialGDK::QueryConstraint& NewInterest);
	void AssignPartitionTo(FPartitionHandle Partition, FLBWorkerHandle Worker);
	void UpdatePartitionMetadata(FPartitionHandle, TArray<ComponentUpdate>);

	void AdvanceView(ISpatialOSWorker& Connection);
	void Flush(ISpatialOSWorker& Connection);

	TArray<FLBWorkerHandle> GetConnectedWorkers();
	TArray<FLBWorkerHandle> GetDisconnectedWorkers();

	Worker_EntityId GetServerWorkerEntityIdForWorker(FLBWorkerHandle);
	Worker_EntityId GetSystemWorkerEntityIdForWorker(FLBWorkerHandle);
	FLBWorkerHandle GetWorkerForServerWorkerEntity(Worker_EntityId);

	struct Impl;

private:
	TUniquePtr<Impl> m_Impl;
};
} // namespace SpatialGDK
