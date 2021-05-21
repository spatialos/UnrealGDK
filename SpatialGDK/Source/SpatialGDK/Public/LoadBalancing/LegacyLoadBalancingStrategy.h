// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "LoadBalancing/LoadBalancingStrategy.h"

class UAbstractLBStrategy;

namespace SpatialGDK
{
class FSpatialPositionStorage;
class FActorGroupStorage;
class FLoadBalancingCalculator;

class FLegacyLoadBalancing : public FLoadBalancingStrategy
{
public:
	FLegacyLoadBalancing(UAbstractLBStrategy& LegacyLBStrat);
	~FLegacyLoadBalancing();

	virtual void Init(TArray<FLBDataStorage*>& OutLoadBalancingData) override;

	virtual void OnWorkersConnected(TArrayView<FLBWorker> ConnectedWorkers) override;
	virtual void OnWorkersDisconnected(TArrayView<FLBWorker> DisconnectedWorkers) override;
	virtual void TickPartitions(FPartitionManager& Partitions) override;
	virtual void CollectEntitiesToMigrate(FMigrationContext& Ctx) override;

protected:
	TUniquePtr<FSpatialPositionStorage> PositionStorage;
	TUniquePtr<FActorGroupStorage> GroupStorage;
	TUniquePtr<FLoadBalancingCalculator> Calculator;
	TArray<FPartitionHandle> Partitions;
	uint32 NumWorkers = 0;
	uint32 ExpectedWorkers = 0;
	bool bCreatedPartitions = false;
};

} // namespace SpatialGDK
