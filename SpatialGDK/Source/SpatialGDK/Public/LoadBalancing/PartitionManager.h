// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "LoadBalancing/LoadBalancingTypes.h"
#include "SpatialCommonTypes.h"
#include "SpatialConstants.h"
#include "SpatialView/SubView.h"

DECLARE_LOG_CATEGORY_EXTERN(LogSpatialPartitionManager, Log, All)

class SpatialOSWorkerInterface;
class SpatialVirtualWorkerTranslator;

namespace SpatialGDK
{
struct QueryConstraint;
class InterestFactory;
class ViewCoordinator;

class FPartitionManager
{
public:
	FPartitionManager(Worker_EntityId InStrategyWorkerEntityId, ViewCoordinator& Coordinator, SpatialVirtualWorkerTranslator& InTranslator,
					  TUniquePtr<InterestFactory>&& InterestF);
	~FPartitionManager();

	void Init(SpatialOSWorkerInterface* Connection, uint32 ExpectedWorkers);

	bool IsReady();

	TOptional<Worker_PartitionId> GetPartitionId(FPartitionHandle);
	FPartitionHandle CreatePartition(void* UserData, const SpatialGDK::QueryConstraint& Interest);
	void SetPartitionInterest(FPartitionHandle Partition, const SpatialGDK::QueryConstraint& NewInterest);
	void AssignPartitionTo(FPartitionHandle Partition, VirtualWorkerId Worker);
	void SetPartitionMetadata(FPartitionHandle /*???*/);

	void AdvanceView(SpatialOSWorkerInterface* Connection);
	void Flush(SpatialOSWorkerInterface* Connection);

	TArray<FLBWorker> GetConnectedWorkers();
	TArray<FLBWorker> GetDisconnectedWorkers();

private:
	struct Impl;
	TUniquePtr<Impl> m_Impl;
};
} // namespace SpatialGDK
