// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "LoadBalancing/LoadBalancingStrategy.h"

class UAbstractLBStrategy;
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

	virtual void Advance(SpatialOSWorkerInterface* Connection) override;
	virtual void Flush(SpatialOSWorkerInterface* Connection) override;

	virtual void OnWorkersConnected(TArrayView<FLBWorkerHandle> ConnectedWorkers) override;
	virtual void OnWorkersDisconnected(TArrayView<FLBWorkerHandle> DisconnectedWorkers) override;
	virtual void TickPartitions(FPartitionManager& Partitions) override;
	virtual void CollectEntitiesToMigrate(FMigrationContext& Ctx) override;

protected:
	void QueryTranslation(SpatialOSWorkerInterface* Connection);

	TUniquePtr<FSpatialPositionStorage> PositionStorage;
	TUniquePtr<FActorGroupStorage> GroupStorage;
	TUniquePtr<FDirectAssignmentStorage> AssignmentStorage;
	TUniquePtr<FLoadBalancingCalculator> Calculator;
	TArray<FPartitionHandle> Partitions;
	TArray<FLBWorkerHandle> VirtualWorkerIdToHandle;

	TSet<FLBWorkerHandle> ConnectedWorkers;

	SpatialVirtualWorkerTranslator& Translator;
	TOptional<Worker_RequestId> WorkerTranslationRequest;
	bool bTranslatorIsReady = false;

	uint32 ExpectedWorkers = 0;
	bool bCreatedPartitions = false;
};

} // namespace SpatialGDK
