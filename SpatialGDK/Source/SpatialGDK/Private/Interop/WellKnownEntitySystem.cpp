// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "Interop/WellKnownEntitySystem.h"

#include "Interop/SpatialReceiver.h"

DEFINE_LOG_CATEGORY(LogWellKnownEntitySystem);

namespace SpatialGDK
{
WellKnownEntitySystem::WellKnownEntitySystem(const FSubView& SubView, USpatialReceiver* InReceiver, USpatialWorkerConnection* InConnection,
											 const int InNumberOfWorkers, SpatialVirtualWorkerTranslator& InVirtualWorkerTranslator,
											 UGlobalStateManager& InGlobalStateManager)
	: SubView(&SubView)
	, Receiver(InReceiver)
	, VirtualWorkerTranslator(&InVirtualWorkerTranslator)
	, GlobalStateManager(&InGlobalStateManager)
	, Connection(InConnection)
	, NumberOfWorkers(InNumberOfWorkers)
{
}

void WellKnownEntitySystem::Advance()
{
	const FSubViewDelta& SubViewDelta = SubView->GetViewDelta();
	for (const EntityDelta& Delta : SubViewDelta.EntityDeltas)
	{
		switch (Delta.Type)
		{
		case EntityDelta::UPDATE:
		{
			for (const ComponentChange& Change : Delta.ComponentUpdates)
			{
				ProcessComponentUpdate(Change.ComponentId, Change.Update);
			}
			for (const ComponentChange& Change : Delta.ComponentsAdded)
			{
				ProcessComponentAdd(Change.ComponentId, Change.Data);
			}
			for (const AuthorityChange& Change : Delta.AuthorityGained)
			{
				ProcessAuthorityGain(Delta.EntityId, Change.ComponentId);
			}
			break;
		}
		case EntityDelta::ADD:
			ProcessEntityAdd(Delta.EntityId);
			break;
		default:
			break;
		}
	}
}

void WellKnownEntitySystem::ProcessComponentUpdate(const Worker_ComponentId ComponentId, Schema_ComponentUpdate* Update)
{
	switch (ComponentId)
	{
	case SpatialConstants::VIRTUAL_WORKER_TRANSLATION_COMPONENT_ID:
		VirtualWorkerTranslator->ApplyVirtualWorkerManagerData(Schema_GetComponentUpdateFields(Update));
		break;
	case SpatialConstants::DEPLOYMENT_MAP_COMPONENT_ID:
		GlobalStateManager->ApplyDeploymentMapUpdate(Update);
		break;
	case SpatialConstants::STARTUP_ACTOR_MANAGER_COMPONENT_ID:
		GlobalStateManager->ApplyStartupActorManagerUpdate(Update);
		break;
	case SpatialConstants::GSM_SHUTDOWN_COMPONENT_ID:
#if WITH_EDITOR
		GlobalStateManager->OnShutdownComponentUpdate(Update);
#endif // WITH_EDITOR
		break;
	default:
		break;
	}
}

void WellKnownEntitySystem::ProcessComponentAdd(const Worker_ComponentId ComponentId, Schema_ComponentData* Data)
{
	switch (ComponentId)
	{
	case SpatialConstants::VIRTUAL_WORKER_TRANSLATION_COMPONENT_ID:
		VirtualWorkerTranslator->ApplyVirtualWorkerManagerData(Schema_GetComponentDataFields(Data));
		break;
	case SpatialConstants::DEPLOYMENT_MAP_COMPONENT_ID:
		GlobalStateManager->ApplyDeploymentMapData(Data);
		break;
	case SpatialConstants::STARTUP_ACTOR_MANAGER_COMPONENT_ID:
		GlobalStateManager->ApplyStartupActorManagerData(Data);
		break;
	case SpatialConstants::SERVER_WORKER_COMPONENT_ID:
		MaybeClaimSnapshotPartition();
		break;
	default:
		break;
	}
}

void WellKnownEntitySystem::ProcessAuthorityGain(const Worker_EntityId EntityId, const Worker_ComponentSetId ComponentSetId)
{
	// if (GlobalStateManager->HandlesComponent(ComponentSetId))
	{
		GlobalStateManager->AuthorityChanged({ EntityId, ComponentSetId, WORKER_AUTHORITY_AUTHORITATIVE });
	}

	// if (ComponentSetId == SpatialConstants::SERVER_WORKER_COMPONENT_ID)
	if (SubView->GetView()[EntityId].Components.ContainsByPredicate([](const SpatialGDK::ComponentData& CompData) {
			return CompData.GetComponentId() == SpatialConstants::SERVER_WORKER_COMPONENT_ID;
		}))
	{
		GlobalStateManager->TrySendWorkerReadyToBeginPlay();
	}

	// if (ComponentSetId == SpatialConstants::VIRTUAL_WORKER_TRANSLATION_COMPONENT_ID)
	if (SubView->GetView()[EntityId].Components.ContainsByPredicate([](const SpatialGDK::ComponentData& CompData) {
			return CompData.GetComponentId() == SpatialConstants::VIRTUAL_WORKER_TRANSLATION_COMPONENT_ID;
		}))
	{
		InitializeVirtualWorkerTranslationManager();
		VirtualWorkerTranslationManager->AuthorityChanged({ EntityId, ComponentSetId, WORKER_AUTHORITY_AUTHORITATIVE });
	}
}

void WellKnownEntitySystem::ProcessEntityAdd(const Worker_EntityId EntityId)
{
	const EntityViewElement& Element = SubView->GetView()[EntityId];
	for (const ComponentData& ComponentData : Element.Components)
	{
		ProcessComponentAdd(ComponentData.GetComponentId(), ComponentData.GetUnderlying());
	}
	for (const Worker_ComponentSetId ComponentId : Element.Authority)
	{
		ProcessAuthorityGain(EntityId, ComponentId);
	}
}

// This is only called if this worker has been selected by SpatialOS to be authoritative
// for the TranslationManager, otherwise the manager will never be instantiated.
void WellKnownEntitySystem::InitializeVirtualWorkerTranslationManager()
{
	VirtualWorkerTranslationManager = MakeUnique<SpatialVirtualWorkerTranslationManager>(Receiver, Connection, VirtualWorkerTranslator);
	VirtualWorkerTranslationManager->SetNumberOfVirtualWorkers(NumberOfWorkers);
}

void WellKnownEntitySystem::MaybeClaimSnapshotPartition()
{
	// Perform a naive leader election where we wait for the correct number of server workers to be present in the deployment, and then
	// whichever server has the lowest server worker entity ID becomes the leader and claims the snapshot partition.
	const Worker_EntityId LocalServerWorkerEntityId = GlobalStateManager->GetLocalServerWorkerEntityId();

	if (LocalServerWorkerEntityId == SpatialConstants::INVALID_ENTITY_ID)
	{
		UE_LOG(LogWellKnownEntitySystem, Warning, TEXT("MaybeClaimSnapshotPartition aborted due to lack of local server worker entity"));
		return;
	}

	Worker_EntityId LowestEntityId = LocalServerWorkerEntityId;

	int ServerCount = 0;
	for (const auto& Iter : SubView->GetView())
	{
		const Worker_EntityId EntityId = Iter.Key;
		const SpatialGDK::EntityViewElement& Element = Iter.Value;
		if (Element.Components.ContainsByPredicate([](const SpatialGDK::ComponentData& CompData) {
				return CompData.GetComponentId() == SpatialConstants::SERVER_WORKER_COMPONENT_ID;
			}))
		{
			ServerCount++;

			if (EntityId < LowestEntityId)
			{
				LowestEntityId = EntityId;
			}
		}
	}

	if (LocalServerWorkerEntityId == LowestEntityId && ServerCount >= NumberOfWorkers)
	{
		UE_LOG(LogWellKnownEntitySystem, Log, TEXT("MaybeClaimSnapshotPartition claiming snapshot partition"));
		GlobalStateManager->ClaimSnapshotPartition();
	}

	if (ServerCount > NumberOfWorkers)
	{
		UE_LOG(LogWellKnownEntitySystem, Warning,
			   TEXT("MaybeClaimSnapshotPartition found too many server worker entities, expected %d got %d."), NumberOfWorkers,
			   ServerCount);
	}
}

} // Namespace SpatialGDK
