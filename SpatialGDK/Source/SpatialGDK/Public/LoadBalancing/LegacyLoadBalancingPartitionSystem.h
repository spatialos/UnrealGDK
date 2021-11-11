#pragma once

#include "EngineClasses/SpatialPartitionSystem.h"
#include "LoadBalancing/LBDataStorage.h"
#include "LoadBalancing/LegacyLoadbalancingComponents.h"

#include "LegacyLoadBalancingPartitionSystem.generated.h"

UCLASS()
class SPATIALGDK_API ULegacyPartitionSystem : public USpatialPartitionSystem
{
	GENERATED_BODY()
public:
	struct PartitionInfo
	{
		bool bDelegated = false;
	};

	ULegacyPartitionSystem();

	const TMap<Worker_EntityId_Key, SpatialGDK::LegacyLB_GridCell>& GetGridCells() const { return GridCells.GetObjects(); }
	const TMap<Worker_EntityId_Key, SpatialGDK::LegacyLB_Layer>& GetLayers() const { return Layers.GetObjects(); }
	const TMap<Worker_EntityId_Key, SpatialGDK::LegacyLB_VirtualWorkerAssignment>& GetVirtualWorkerIds() const
	{
		return VirtualWorkerIds.GetObjects();
	}

	const TMap<Worker_RequestId_Key, PartitionInfo>& GetPartitions() { return Partitions; }

protected:
	virtual TArray<SpatialGDK::FLBDataStorage*> GetData() override;

	virtual void Initialize(FSubsystemCollectionBase&) override;
	virtual void Deinitialize() override;

	void Tick();

	void OnWorldChanged(UWorld* OldWorld, UWorld* NewWorld);

	FDelegateHandle WorldChangedDelegate;
	FDelegateHandle PostTickDispatchDelegate;

	TMap<Worker_RequestId_Key, PartitionInfo> Partitions;
	SpatialGDK::TLBDataStorage<SpatialGDK::LegacyLB_GridCell> GridCells;
	SpatialGDK::TLBDataStorage<SpatialGDK::LegacyLB_Layer> Layers;
	SpatialGDK::TLBDataStorage<SpatialGDK::LegacyLB_VirtualWorkerAssignment> VirtualWorkerIds;
};
