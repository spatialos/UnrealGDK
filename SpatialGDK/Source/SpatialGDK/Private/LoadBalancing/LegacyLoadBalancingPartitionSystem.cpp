#include "LoadBalancing/LegacyLoadBalancingPartitionSystem.h"
#include "LoadBalancing/LegacyLoadBalancingCommon.h"

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
	TSet<Worker_EntityId_Key> NewPartitions;

	for (auto Event : ConsumePartitionEvents())
	{
		Worker_EntityId Partition = Event.PartitionId;
		if (Event.Event == SpatialGDK::FPartitionEvent::Created)
		{
			NewPartitions.Add(Partition);
			UE_LOG(LogSpatialLegacyLoadBalancing, Log, TEXT("New partition : %llu"), Partition);
			if (auto* GridData = GetGridCells().Find(Partition))
			{
				UE_LOG(LogSpatialLegacyLoadBalancing, Log, TEXT("Grid pos : %f, %f"), GridData->Center.X, GridData->Center.Y);
			}
			if (auto* LayerData = GetLayers().Find(Partition))
			{
				UE_LOG(LogSpatialLegacyLoadBalancing, Log, TEXT("Layer : %i"), LayerData->Layer);
			}
			if (auto* VirtualWorkerIdData = GetVirtualWorkerIds().Find(Partition))
			{
				UE_LOG(LogSpatialLegacyLoadBalancing, Log, TEXT("Virtual WorkerId : %i"), VirtualWorkerIdData->Virtual_worker_id);
			}
		}
		if (Event.Event == SpatialGDK::FPartitionEvent::Delegated)
		{
			UE_LOG(LogSpatialLegacyLoadBalancing, Log, TEXT("Partition %llu delegated to the local worker"), Partition);
		}
		if (Event.Event == SpatialGDK::FPartitionEvent::DelegationLost)
		{
			UE_LOG(LogSpatialLegacyLoadBalancing, Log, TEXT("Partition %llu delegation lost"), Partition);
		}
		if (Event.Event == SpatialGDK::FPartitionEvent::Deleted)
		{
			UE_LOG(LogSpatialLegacyLoadBalancing, Log, TEXT("Partition %llu deleted"), Partition);
		}
	}

	GridCells.ClearModified();
	Layers.ClearModified();
	VirtualWorkerIds.ClearModified();
	Partitions.Append(NewPartitions);
}
