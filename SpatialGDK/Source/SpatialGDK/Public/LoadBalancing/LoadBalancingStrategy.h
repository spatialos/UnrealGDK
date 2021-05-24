// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "LoadBalancing/LoadBalancingTypes.h"
#include "SpatialCommonTypes.h"
#include "SpatialConstants.h"

class SpatialOSWorkerInterface;

namespace SpatialGDK
{
class FPartitionManager;
class FLBDataStorage;

class SPATIALGDK_API FLoadBalancingStrategy
{
public:
	virtual ~FLoadBalancingStrategy() = default;

	virtual void Init(TArray<FLBDataStorage*>& OutLoadBalancingData) {}

	virtual void Advance(SpatialOSWorkerInterface* Connection) {}
	virtual void Flush(SpatialOSWorkerInterface* Connection) {}

	virtual void OnWorkersConnected(TArrayView<FLBWorkerHandle> ConnectedWorkers) {}
	virtual void OnWorkersDisconnected(TArrayView<FLBWorkerHandle> DisconnectedWorkers) {}
	virtual void TickPartitions(FPartitionManager& Partitions) {}
	virtual void CollectEntitiesToMigrate(FMigrationContext& Ctx) {}
};

} // namespace SpatialGDK
