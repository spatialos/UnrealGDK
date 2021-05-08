// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "SpatialCommonTypes.h"
#include "SpatialConstants.h"
#include "SpatialView/EntityView.h"

class FLoadBalancingCalculator;

// May need some RTTI. In Unreal space? Should think about a pure C++ StrategyWorker
struct FPartitionDeclaration : TSharedFromThis<FPartitionDeclaration>
{
	const FPartitionDeclaration* ParentPartition;
	FLoadBalancingCalculator* Calculator;
	uint32_t CalculatorIndex;
	FString Name;
	bool bActive;
};

// RTTI ?
struct MigrationContext
{
	TSet<Worker_EntityId> MigratingEntities;
	TMap<Worker_EntityId, TSharedPtr<FPartitionDeclaration>> EntitiesToMigrate;
};

class SPATIALGDK_API FLoadBalancingCalculator
{
public:
	virtual ~FLoadBalancingCalculator() = default;

	virtual void OnAdded(Worker_EntityId EntityId, SpatialGDK::EntityViewElement const& Element)
	{
		for (const auto& Component : Element.Components)
		{
			if (ComponentsToInspect.Contains(Component.GetComponentId()))
			{
				OnAdded_ReadComponent(EntityId, Component.GetComponentId(), Component.GetUnderlying());
			}
		}
	}
	virtual void OnAdded_ReadComponent(Worker_EntityId EntityId, Worker_ComponentId ComponentId, Schema_ComponentData* Data) {}
	virtual void OnRemoved(Worker_EntityId EntityId) {}
	virtual void OnUpdate(Worker_EntityId EntityId, Worker_ComponentId ComponentId, Schema_ComponentUpdate* Update) {}

	virtual void CollectPartitionsToAdd(const FPartitionDeclaration* Parent, TArray<TSharedPtr<FPartitionDeclaration>>& OutPartitions) {}
	virtual void CollectPartitionsToRemove(const FPartitionDeclaration* Parent, TArray<TSharedPtr<FPartitionDeclaration>>& OutPartitions) {}
	// virtual void CollectComponentsToInspect(TMultiMap<Worker_ComponentId, FLoadBalancingCalculator*>& OutMap)
	//{
	//	for (auto Component : ComponentsToInspect)
	//	{
	//		OutMap.Add(Component, this);
	//	}
	//}
	virtual void CollectComponentsToInspect(TSet<Worker_ComponentId>& OutMap)
	{
		for (auto Component : ComponentsToInspect)
		{
			OutMap.Add(Component);
		}
	}

	virtual void CollectEntitiesToMigrate(MigrationContext& Ctx) {}

protected:
	TSet<Worker_ComponentId> ComponentsToInspect;
};

class SPATIALGDK_API FGridBalancingCalculator : public FLoadBalancingCalculator
{
public:
	FGridBalancingCalculator(uint32 GridX, uint32 GridY, float Height, float Width);

	virtual void OnAdded_ReadComponent(Worker_EntityId EntityId, Worker_ComponentId ComponentId, Schema_ComponentData* Data) override;
	virtual void OnRemoved(Worker_EntityId EntityId) override;
	virtual void OnUpdate(Worker_EntityId EntityId, Worker_ComponentId ComponentId, Schema_ComponentUpdate* Update) override;

	virtual void CollectPartitionsToAdd(const FPartitionDeclaration* Parent,
										TArray<TSharedPtr<FPartitionDeclaration>>& OutPartitions) override;

	virtual void CollectEntitiesToMigrate(MigrationContext& Ctx) override;

protected:
	uint32 Rows;
	uint32 Cols;
	float WorldWidth;
	float WorldHeight;

	TArray<TSharedPtr<FPartitionDeclaration>> Partitions;
	TArray<FBox2D> Cells;

	TSet<Worker_EntityId_Key> Modified;

	struct EntityData
	{
		FVector Position;
		int32 Assignment = -1;
	};

	TMap<Worker_EntityId_Key, EntityData> Positions;
};

class SPATIALGDK_API FLayerLoadBalancingCalculator : public FLoadBalancingCalculator
{
public:
	FLayerLoadBalancingCalculator(TArray<FName> LayerNames, TArray<TUniquePtr<FLoadBalancingCalculator>>&& Layers);
	FLayerLoadBalancingCalculator(const FLayerLoadBalancingCalculator&) = delete;
	FLayerLoadBalancingCalculator& operator=(const FLayerLoadBalancingCalculator&) = delete;
	virtual void OnAdded(Worker_EntityId EntityId, SpatialGDK::EntityViewElement const& Element) override;
	virtual void OnAdded_ReadComponent(Worker_EntityId EntityId, Worker_ComponentId ComponentId, Schema_ComponentData* Data) override;
	virtual void OnRemoved(Worker_EntityId EntityId) override;
	virtual void OnUpdate(Worker_EntityId EntityId, Worker_ComponentId ComponentId, Schema_ComponentUpdate* Update) override;

	virtual void CollectPartitionsToAdd(const FPartitionDeclaration* Parent,
										TArray<TSharedPtr<FPartitionDeclaration>>& OutPartitions) override;

	virtual void CollectEntitiesToMigrate(MigrationContext& Ctx) override;
	virtual void CollectComponentsToInspect(TSet<Worker_ComponentId>& OutSet) override;

protected:
	TArray<FName> LayerNames;
	TArray<TUniquePtr<FLoadBalancingCalculator>> Layers;
	TArray<TSharedPtr<FPartitionDeclaration>> VirtualPartitions;

	TMap<Worker_EntityId_Key, uint32> GroupMembership;
};
