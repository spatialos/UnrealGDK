// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "LoadBalancing/AbstractLBStrategy.h"
#include "LoadBalancing/LoadBalancingStrategy.h"

class SpatialVirtualWorkerTranslator;

namespace SpatialGDK
{
class FSpatialPositionStorage;
class FActorGroupStorage;
class FDirectAssignmentStorage;
class FLoadBalancingCalculator;

class FLegacyLoadBalancing : public FLoadBalancingStrategy
{
public:
	FLegacyLoadBalancing(UAbstractLBStrategy& LegacyLBStrat, SpatialVirtualWorkerTranslator& InTranslator);
	~FLegacyLoadBalancing();

	virtual void Init(TArray<FLBDataStorage*>& OutLoadBalancingData) override;

	virtual void Advance(ISpatialOSWorker& Connection) override;
	virtual void Flush(ISpatialOSWorker& Connection) override;

	virtual void OnWorkersConnected(TArrayView<FLBWorkerHandle> ConnectedWorkers) override;
	virtual void OnWorkersDisconnected(TArrayView<FLBWorkerHandle> DisconnectedWorkers) override;
	virtual void TickPartitions(FPartitionManager& Partitions) override;
	virtual void CollectEntitiesToMigrate(FMigrationContext& Ctx) override;

protected:
	void QueryTranslation(ISpatialOSWorker& Connection);

	// +++ Data Storage +++
	TUniquePtr<FSpatialPositionStorage> PositionStorage;
	TUniquePtr<FActorGroupStorage> GroupStorage;
	TUniquePtr<FDirectAssignmentStorage> AssignmentStorage;
	// --- Data Storage ---

	// +++ Partition Assignment +++
	TArray<FPartitionHandle> Partitions;
	TArray<FLBWorkerHandle> VirtualWorkerIdToHandle;
	TSet<FLBWorkerHandle> ConnectedWorkers;
	SpatialVirtualWorkerTranslator& Translator;
	TOptional<Worker_RequestId> WorkerTranslationRequest;
	uint32 ExpectedWorkers = 0;
	bool bCreatedPartitions = false;
	bool bTranslatorIsReady = false;
	// --- Partition Assignment ---

	// +++ Load Balancing +++
	FLegacyLBContext LBContext;
	TSet<Worker_EntityId_Key> ToRefresh;
	TMap<Worker_EntityId_Key, int32> Assignment;
	bool bDirectAssignment = false;
	// --- Load Balancing ---
};

} // namespace SpatialGDK
