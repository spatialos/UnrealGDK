// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "LoadBalancing/LoadBalancingTypes.h"
#include "SpatialCommonTypes.h"
#include "SpatialConstants.h"
#include "SpatialView/EntityView.h"

namespace SpatialGDK
{
class FPartitionManager;
class FSpatialPositionStorage;
class FActorGroupStorage;

class SPATIALGDK_API FLoadBalancingCalculator
{
public:
	virtual ~FLoadBalancingCalculator() = default;

	virtual void CollectPartitionsToAdd(FPartitionManager& Partitions, TArray<FPartitionHandle>& OutPartitions) {}
	virtual void CollectEntitiesToMigrate(FMigrationContext& Ctx) {}

protected:
};

class SPATIALGDK_API FGridBalancingCalculator : public FLoadBalancingCalculator
{
public:
	FGridBalancingCalculator(uint32 GridX, uint32 GridY, float Height, float Width);

	void SetPositionData(FSpatialPositionStorage const& iStorage) { DataStorage = &iStorage; }

	virtual void CollectPartitionsToAdd(FPartitionManager& Partitions, TArray<FPartitionHandle>& OutPartitions) override;

	virtual void CollectEntitiesToMigrate(FMigrationContext& Ctx) override;

protected:
	uint32 Rows;
	uint32 Cols;
	float WorldWidth;
	float WorldHeight;

	TArray<FPartitionHandle> Partitions;
	TArray<FBox2D> Cells;

	FSpatialPositionStorage const* DataStorage = nullptr;
	TSet<Worker_EntityId_Key> ToRefresh;
	TMap<Worker_EntityId_Key, int32> Assignment;
};

class SPATIALGDK_API FLayerLoadBalancingCalculator : public FLoadBalancingCalculator
{
public:
	FLayerLoadBalancingCalculator(TArray<FName> LayerNames, TArray<TUniquePtr<FLoadBalancingCalculator>>&& Layers);
	FLayerLoadBalancingCalculator(const FLayerLoadBalancingCalculator&) = delete;
	FLayerLoadBalancingCalculator& operator=(const FLayerLoadBalancingCalculator&) = delete;

	virtual void CollectPartitionsToAdd(FPartitionManager& Partitions, TArray<FPartitionHandle>& OutPartitions) override;

	virtual void CollectEntitiesToMigrate(FMigrationContext& Ctx) override;

	void SetGroupData(FActorGroupStorage const& iStorage) { GroupStorage = &iStorage; }

protected:
	FActorGroupStorage const* GroupStorage;
	TArray<FName> LayerNames;
	TArray<TUniquePtr<FLoadBalancingCalculator>> Layers;
};
} // namespace SpatialGDK
