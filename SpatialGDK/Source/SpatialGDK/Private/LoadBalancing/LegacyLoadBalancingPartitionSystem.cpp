#include "LoadBalancing/LegacyLoadBalancingPartitionSystem.h"

ULegacyPartitionSystem::ULegacyPartitionSystem() {}

TArray<SpatialGDK::FLBDataStorage*> ULegacyPartitionSystem::GetData()
{
	TArray<SpatialGDK::FLBDataStorage*> Storages;
	Storages.Add(&GridCells);
	Storages.Add(&Layers);
	Storages.Add(&VirtualWorkerIds);

	return Storages;
}

void ULegacyPartitionSystem::Initialize(FSubsystemCollectionBase&)
{
	GetGameInstance()->GetWorld()->OnPostTickDispatch().AddUObject(this, &ULegacyPartitionSystem::Tick);
}

void ULegacyPartitionSystem::Tick()
{
	TSet<Worker_EntityId_Key> Modified = GridCells.GetModifiedEntities();
	Modified = Modified.Union(Layers.GetModifiedEntities());
	Modified = Modified.Union(VirtualWorkerIds.GetModifiedEntities());

	TSet<Worker_EntityId_Key> NewPartitions = Modified.Difference(Partitions);

	for (auto Partition : NewPartitions)
	{
		UE_LOG(LogTemp, Log, TEXT("New partition : %llu"), Partition);
		if (auto* GridData = GetGridCells().Find(Partition))
		{
			UE_LOG(LogTemp, Log, TEXT("Grid pos : %f, %f"), GridData->Center.X, GridData->Center.Y);
		}
		if (auto* LayerData = GetLayers().Find(Partition))
		{
			UE_LOG(LogTemp, Log, TEXT("Layer : %i"), LayerData->Layer);
		}
		if (auto* VirtualWorkerIdData = GetVirtualWorkerIds().Find(Partition))
		{
			UE_LOG(LogTemp, Log, TEXT("Virtual WorkerId : %i"), VirtualWorkerIdData->Virtual_worker_id);
		}
	}

	GridCells.ClearModified();
	Layers.ClearModified();
	VirtualWorkerIds.ClearModified();
	Partitions.Append(NewPartitions);
}
