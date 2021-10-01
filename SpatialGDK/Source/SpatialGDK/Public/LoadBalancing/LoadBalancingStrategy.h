// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "LoadBalancing/LoadBalancingTypes.h"
#include "SpatialCommonTypes.h"
#include "SpatialConstants.h"

namespace SpatialGDK
{
class ISpatialOSWorker;
class FPartitionManager;
class FLBDataStorage;

class SPATIALGDK_API FLoadBalancingStrategy
{
public:
	virtual ~FLoadBalancingStrategy() = default;

	virtual void Init(FLoadBalancingSharedData InSharedData, TArray<FLBDataStorage*>& OutLoadBalancingData,
					  TArray<FLBDataStorage*>& OutServerWorkerData)
	{
	}

	virtual void Advance(ISpatialOSWorker& Connection, const TSet<Worker_EntityId_Key>& DeletedEntities) {}
	virtual void Flush(ISpatialOSWorker& Connection) {}

	virtual void OnWorkersConnected(TArrayView<FLBWorkerHandle> ConnectedWorkers) {}
	virtual void OnWorkersDisconnected(TArrayView<FLBWorkerHandle> DisconnectedWorkers) {}
	virtual void TickPartitions() {}
	virtual void CollectEntitiesToMigrate(FMigrationContext& Ctx) {}
};

} // namespace SpatialGDK
