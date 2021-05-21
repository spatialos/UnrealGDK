// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "LoadBalancing/LoadBalancingTypes.h"
#include "SpatialCommonTypes.h"
#include "SpatialConstants.h"

namespace SpatialGDK
{
class FPartitionManager;
class FLBDataStorage;

class SPATIALGDK_API FLoadBalancingStrategy
{
public:
	virtual ~FLoadBalancingStrategy() = default;

	virtual void Init(TArray<FLBDataStorage*>& OutLoadBalancingData) {}

	virtual void OnWorkersConnected(TArrayView<FLBWorker> ConnectedWorkers) {}
	virtual void OnWorkersDisconnected(TArrayView<FLBWorker> DisconnectedWorkers) {}
	virtual void TickPartitions(FPartitionManager& Partitions) {}
	virtual void CollectEntitiesToMigrate(FMigrationContext& Ctx) {}
};

} // namespace SpatialGDK
