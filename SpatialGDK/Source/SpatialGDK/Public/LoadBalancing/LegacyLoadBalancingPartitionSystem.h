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

	const TMap<Worker_EntityId_Key, SpatialGDK::LegacyLB_GridCell>& GetGridCells() const { return GridCells.GetObjects(); }
	const TMap<Worker_EntityId_Key, SpatialGDK::LegacyLB_Layer>& GetLayers() const { return Layers.GetObjects(); }
	const TMap<Worker_EntityId_Key, SpatialGDK::LegacyLB_VirtualWorkerAssignment>& GetVirtualWorkerIds() const
	{
		return VirtualWorkerIds.GetObjects();
	}

protected:
	virtual TArray<SpatialGDK::FLBDataStorage*> GetData() override;

	virtual void Initialize(FSubsystemCollectionBase&) override;
	virtual void Deinitialize() override;

	void Tick();

	void OnWorldChanged(UWorld* OldWorld, UWorld* NewWorld);

	FDelegateHandle WorldChangedDelegate;
	FDelegateHandle PostTickDispatchDelegate;

	TSet<Worker_RequestId_Key> Partitions;
	SpatialGDK::TLBDataStorage<SpatialGDK::LegacyLB_GridCell> GridCells;
	SpatialGDK::TLBDataStorage<SpatialGDK::LegacyLB_Layer> Layers;
	SpatialGDK::TLBDataStorage<SpatialGDK::LegacyLB_VirtualWorkerAssignment> VirtualWorkerIds;
};
