#pragma once

#include "EngineClasses/SpatialPartitionSystem.h"
#include "LoadBalancing/LBDataStorage.h"
#include "LoadBalancing/LegacyLoadbalancingComponents.h"

#include "LegacyLoadBalancingPartitionSystem.generated.h"

UCLASS()
class ULegacyPartitionSystem : public USpatialPartitionSystem
{
	GENERATED_BODY()
public:
	ULegacyPartitionSystem();

	TArray<SpatialGDK::FLBDataStorage*> GetData() override;

	void Initialize(FSubsystemCollectionBase&) override;

	void Tick();

	const TMap<Worker_EntityId_Key, SpatialGDK::LegacyLB_GridCell>& GetGridCells() const { return GridCells.GetObjects(); }
	const TMap<Worker_EntityId_Key, SpatialGDK::LegacyLB_Layer>& GetLayers() const { return Layers.GetObjects(); }
	const TMap<Worker_EntityId_Key, SpatialGDK::LegacyLB_VirtualWorkerAssignment>& GetVirtualWorkerIds() const
	{
		return VirtualWorkerIds.GetObjects();
	}

protected:
	TSet<Worker_RequestId_Key> Partitions;
	SpatialGDK::TLBDataStorage<SpatialGDK::LegacyLB_GridCell> GridCells;
	SpatialGDK::TLBDataStorage<SpatialGDK::LegacyLB_Layer> Layers;
	SpatialGDK::TLBDataStorage<SpatialGDK::LegacyLB_VirtualWorkerAssignment> VirtualWorkerIds;
};
