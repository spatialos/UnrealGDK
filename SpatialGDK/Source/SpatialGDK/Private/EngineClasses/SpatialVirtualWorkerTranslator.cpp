// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "EngineClasses/SpatialVirtualWorkerTranslator.h"
#include "SpatialConstants.h"

DEFINE_LOG_CATEGORY(LogSpatialVirtualWorkerTranslator);

void USpatialVirtualWorkerTranslator::Init()
{
	TArray<ZoneId> ZoneIds;
	GetZones(ZoneIds);

	TArray<VirtualWorkerId> VirtualWorkerIds;
	GetVirtualWorkers(VirtualWorkerIds);

	TArray<WorkerId> WorkerIds;
	GetWorkers(WorkerIds);

	// The zone -> virtual worker counts don't strictly need to match, but for first pass we'll enforce this
	check(ZoneIds.Num() == VirtualWorkerIds.Num());
	check(VirtualWorkerIds.Num() == WorkerIds.Num());

	for (int i = 0; i < ZoneIds.Num(); ++i)
	{
		ZoneToVirtualWorkerMap.Add(ZoneIds[i], VirtualWorkerIds[i]);
	}

	for (int i = 0; i < VirtualWorkerIds.Num(); ++i)
	{
		VirtualWorkerToWorkerMap.Add(VirtualWorkerIds[i], WorkerIds[i]);
	}
}

void USpatialVirtualWorkerTranslator::GetZones(TArray<ZoneId>& ZoneIds)
{
	ZoneIds.Add("Mountains");
	ZoneIds.Add("Plains");
	ZoneIds.Add("Wetlands");
	ZoneIds.Add("Coastal");
}

void USpatialVirtualWorkerTranslator::GetVirtualWorkers(TArray<VirtualWorkerId>& VirtualWorkerIds)
{
	// TODO: replace this with data supplied from Unreal (the Zone -> VirtualWorkerId map)
	VirtualWorkerIds.Add(TEXT("A"));
	VirtualWorkerIds.Add(TEXT("B"));
	VirtualWorkerIds.Add(TEXT("C"));
	VirtualWorkerIds.Add(TEXT("D"));
}

void USpatialVirtualWorkerTranslator::GetWorkers(TArray<WorkerId>& WorkerIds)
{
	// TODO: this won't actually work until we replace with real worker ids from the worker entities
	//WorkerIds.Add(TEXT("UnrealWorker0"));
	//WorkerIds.Add(TEXT("UnrealWorker1"));
	//WorkerIds.Add(TEXT("UnrealWorker2"));
	//WorkerIds.Add(TEXT("UnrealWorker3"));
}

bool USpatialVirtualWorkerTranslator::HandlesComponent(const Worker_ComponentId ComponentId) const
{
	return ComponentId == SpatialConstants::WORKER_COMPONENT_LISTENER_COMPONENT_ID;
}

void USpatialVirtualWorkerTranslator::AuthorityChanged(const Worker_AuthorityChangeOp& AuthOp)
{
	UE_LOG(LogSpatialVirtualWorkerTranslator, Verbose, TEXT("Authority over the VirtualWorkerTranslator component %d has changed. This worker %s authority."), AuthOp.component_id,
		AuthOp.authority == WORKER_AUTHORITY_AUTHORITATIVE ? TEXT("now has") : TEXT("does not have"));

	if (AuthOp.authority != WORKER_AUTHORITY_AUTHORITATIVE)
	{
		return;
	}

	switch (AuthOp.component_id)
	{
		case SpatialConstants::WORKER_COMPONENT_LISTENER_COMPONENT_ID:
		{
			//GlobalStateManagerEntityId = AuthOp.entity_id;
			//SetAcceptingPlayers(true);
			//break;
		}
		default:
		{
			break;
		}
	}
}

void USpatialVirtualWorkerTranslator::ApplyWorkerComponentListenerData(const Worker_ComponentData& Data)
{

}
