// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "LoadBalancing/LegacyLoadBalancingStrategy.h"
#include "LoadBalancing/LBDataStorage.h"
#include "LoadBalancing/LoadBalancingCalculator.h"

#include "LoadBalancing/AbstractLBStrategy.h"

namespace SpatialGDK
{
FLegacyLoadBalancing::FLegacyLoadBalancing(UAbstractLBStrategy& LegacyLBStrat)
{
	ExpectedWorkers = LegacyLBStrat.GetMinimumRequiredWorkers();
	PositionStorage = MakeUnique<SpatialGDK::FSpatialPositionStorage>();
	GroupStorage = MakeUnique<SpatialGDK::FActorGroupStorage>();

	FLegacyLBContext LBContext;
	Calculator = LegacyLBStrat.CreateLoadBalancingCalculator(LBContext);

	if (LBContext.Layers != nullptr)
	{
		LBContext.Layers->SetGroupData(*GroupStorage);
	}
	for (auto* Grid : LBContext.Grid)
	{
		Grid->SetPositionData(*PositionStorage);
	}
}

FLegacyLoadBalancing::~FLegacyLoadBalancing() {}

void FLegacyLoadBalancing::Init(TArray<FLBDataStorage*>& OutLoadBalancingData)
{
	OutLoadBalancingData.Add(PositionStorage.Get());
	OutLoadBalancingData.Add(GroupStorage.Get());
}

void FLegacyLoadBalancing::OnWorkersConnected(TArrayView<FLBWorker> ConnectedWorkers)
{
	NumWorkers += ConnectedWorkers.Num();
}

void FLegacyLoadBalancing::OnWorkersDisconnected(TArrayView<FLBWorker> DisconnectedWorkers)
{
	// Not handled
}

void FLegacyLoadBalancing::TickPartitions(FPartitionManager& PartitionMgr)
{
	if (bCreatedPartitions)
	{
		return;
	}
	if (NumWorkers == ExpectedWorkers)
	{
		bCreatedPartitions = true;
		Calculator->CollectPartitionsToAdd(PartitionMgr, Partitions);
	}
}

void FLegacyLoadBalancing::CollectEntitiesToMigrate(FMigrationContext& Ctx)
{
	if (bCreatedPartitions)
	{
		Calculator->CollectEntitiesToMigrate(Ctx);
	}
}
} // namespace SpatialGDK
