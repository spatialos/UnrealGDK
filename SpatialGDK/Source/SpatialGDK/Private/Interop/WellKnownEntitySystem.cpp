// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "Interop/WellKnownEntitySystem.h"

#include "Interop/SpatialReceiver.h"

DEFINE_LOG_CATEGORY(LogWellKnownEntitySystem);

namespace SpatialGDK
{
WellKnownEntitySystem::WellKnownEntitySystem(const FSubView& InGDKSubView, const FSubView& InSystemEntitySubView,
											 USpatialWorkerConnection* InConnection, const int InNumberOfWorkers,
											 SpatialVirtualWorkerTranslator& InVirtualWorkerTranslator,
											 UGlobalStateManager& InGlobalStateManager)
	: GDKSubView(&InGDKSubView)
	, SystemEntitySubView(&InSystemEntitySubView)
	, VirtualWorkerTranslator(&InVirtualWorkerTranslator)
	, GlobalStateManager(&InGlobalStateManager)
	, Connection(InConnection)
	, NumberOfWorkers(InNumberOfWorkers)
	, SnapshotPartitionServerSystemEntity(SpatialConstants::INVALID_ENTITY_ID)
	, bSnapshotPartitionAuthServerCrashInProgress(false)
{
}

void WellKnownEntitySystem::Advance()
{
	const FSubViewDelta& GDKSubViewDelta = GDKSubView->GetViewDelta();
	for (const EntityDelta& Delta : GDKSubViewDelta.EntityDeltas)
	{
		switch (Delta.Type)
		{
		case EntityDelta::ADD:
			ProcessEntityAdd(Delta.EntityId);
			break;
		case EntityDelta::UPDATE:
		{
			for (const ComponentChange& Change : Delta.ComponentUpdates)
			{
				ProcessComponentUpdate(Delta.EntityId, Change.ComponentId, Change.Update);
			}
			for (const ComponentChange& Change : Delta.ComponentsAdded)
			{
				ProcessComponentAdd(Delta.EntityId, Change.ComponentId, Change.Data);
			}
			for (const AuthorityChange& Change : Delta.AuthorityGained)
			{
				ProcessAuthorityGain(Delta.EntityId, Change.ComponentSetId);
			}
			break;
		}
		default:
			break;
		}
	}

	if (VirtualWorkerTranslationManager.IsValid())
	{
		VirtualWorkerTranslationManager->Advance(*GDKSubView->GetViewDelta().WorkerMessages);
	}

	const FSubViewDelta& SubViewDelta = SystemEntitySubView->GetViewDelta();
	for (const EntityDelta& Delta : SubViewDelta.EntityDeltas)
	{
		switch (Delta.Type)
		{
		case EntityDelta::REMOVE:
			// If the snapshot partition authoritative server crashed, every other server checks to see if
			// it should be the new leader, and thus claim the snapshot partition.
			if (SnapshotPartitionServerSystemEntity == Delta.EntityId)
			{
				UE_LOG(LogWellKnownEntitySystem, Warning, TEXT("Server detected that snapshot partition auth server crashed."));
				bSnapshotPartitionAuthServerCrashInProgress = true;
				MaybeClaimSnapshotPartition();
			}
			// If we're the VTM auth server, and some other worker disconnected, we should check if it's a server
			// worker, and if so, update the translation accordingly (so we're ready for the server to restart).
			else if (VirtualWorkerTranslationManager != nullptr)
			{
				VirtualWorkerTranslationManager->OnSystemEntityRemoved(Delta.EntityId);
			}
		default:
			break;
		}
	}
}

void WellKnownEntitySystem::ProcessComponentUpdate(const Worker_EntityId EntityId, const Worker_ComponentId ComponentId,
												   Schema_ComponentUpdate* Update)
{
	switch (ComponentId)
	{
	case SpatialConstants::PARTITION_COMPONENT_ID:
		// Whenever we change snapshot partition auth server, every server stores the information which is used in
		// the event of the snapshot partition auth server crashing.
		if (EntityId == SpatialConstants::INITIAL_SNAPSHOT_PARTITION_ENTITY_ID)
		{
			SaveSnapshotPartitionAuthServerSystemEntity();
		}
		break;
	case SpatialConstants::VIRTUAL_WORKER_TRANSLATION_COMPONENT_ID:
		VirtualWorkerTranslator->ApplyVirtualWorkerTranslation(Update);
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

void WellKnownEntitySystem::ProcessComponentAdd(const Worker_EntityId EntityId, const Worker_ComponentId ComponentId,
												Schema_ComponentData* Data)
{
	switch (ComponentId)
	{
	case SpatialConstants::PARTITION_COMPONENT_ID:
		if (EntityId == SpatialConstants::INITIAL_SNAPSHOT_PARTITION_ENTITY_ID)
		{
			SaveSnapshotPartitionAuthServerSystemEntity();
		}
		break;
	case SpatialConstants::VIRTUAL_WORKER_TRANSLATION_COMPONENT_ID:
		UE_LOG(LogWellKnownEntitySystem, Log, TEXT("VIRTUAL_WORKER_TRANSLATION_COMPONENT_ID received %lld"), EntityId);
		VirtualWorkerTranslator->ApplyVirtualWorkerTranslation(VirtualWorkerTranslation(Data));
		break;
	case SpatialConstants::DEPLOYMENT_MAP_COMPONENT_ID:
		GlobalStateManager->ApplyDeploymentMapData(Data);
		break;
	case SpatialConstants::SNAPSHOT_VERSION_COMPONENT_ID:
		GlobalStateManager->ApplySnapshotVersionData(Data);
		break;
	case SpatialConstants::STARTUP_ACTOR_MANAGER_COMPONENT_ID:
		GlobalStateManager->ApplyStartupActorManagerData(Data);
		break;
	case SpatialConstants::SERVER_WORKER_COMPONENT_ID:
		// We only want servers to consider claiming the new partition when a new server spawns at the start of
		// a deployment. If the snapshot partition authoritative server crashes, we reassign the snapshot partition
		// auth as soon as the crash is detected (rather than waiting for the replacement server to start).
		if (!VirtualWorkerTranslator->IsReady())
		{
			MaybeClaimSnapshotPartition();
		}

		// If the translator is ready, this server worker component is a restarted server.
		// If we're authoritative over the translation manager, we need to claim the partition for the restarted worker.
		if (VirtualWorkerTranslationManager.IsValid() && VirtualWorkerTranslator->IsReady())
		{
			VirtualWorkerTranslationManager->TryClaimPartitionForRecoveredWorker(EntityId, Data);
		}
		break;
	default:
		break;
	}
}

void WellKnownEntitySystem::SaveSnapshotPartitionAuthServerSystemEntity()
{
	const ComponentData* SnapshotPartitionData =
		GDKSubView->GetView()[SpatialConstants::INITIAL_SNAPSHOT_PARTITION_ENTITY_ID].Components.FindByPredicate([](const auto& CompData) {
			return CompData.GetComponentId() == SpatialConstants::PARTITION_COMPONENT_ID;
		});

	if (SnapshotPartitionData == nullptr)
	{
		UE_LOG(LogWellKnownEntitySystem, Error,
			   TEXT("Failed to store snapshot partition auth server. If that server crashes, we won't be able to restart it."));
		return;
	}

	SnapshotPartitionServerSystemEntity = Schema_GetEntityId(Schema_GetComponentDataFields(SnapshotPartitionData->GetUnderlying()),
															 SpatialConstants::PARTITION_SYSTEM_ENTITY_ID_FIELD);
	bSnapshotPartitionAuthServerCrashInProgress = false;
}

void WellKnownEntitySystem::ProcessAuthorityGain(const Worker_EntityId EntityId, const Worker_ComponentSetId ComponentSetId)
{
	GlobalStateManager->AuthorityChanged({ EntityId, ComponentSetId, WORKER_AUTHORITY_AUTHORITATIVE });

	if (GDKSubView->GetView()[EntityId].Components.ContainsByPredicate(
			SpatialGDK::ComponentIdEquality{ SpatialConstants::SERVER_WORKER_COMPONENT_ID }))
	{
		GlobalStateManager->WorkerEntityReady();
		GlobalStateManager->TrySendWorkerReadyToBeginPlay();
	}

	if (GDKSubView->GetView()[EntityId].Components.ContainsByPredicate(
			SpatialGDK::ComponentIdEquality{ SpatialConstants::VIRTUAL_WORKER_TRANSLATION_COMPONENT_ID }))
	{
		InitializeVirtualWorkerTranslationManager();
		VirtualWorkerTranslationManager->SetKnownServerSystemEntities(GetAllServerSystemEntities());
		VirtualWorkerTranslationManager->AuthorityChanged({ EntityId, ComponentSetId, WORKER_AUTHORITY_AUTHORITATIVE });
	}
}

TArray<Worker_EntityId> WellKnownEntitySystem::GetAllServerSystemEntities() const
{
	TArray<Worker_EntityId> ServerSystemEntities;
	ServerSystemEntities.Reserve(SystemEntitySubView->GetView().Num());

	for (const auto& Iter : SystemEntitySubView->GetView())
	{
		const Worker_EntityId EntityId = Iter.Key;
		const SpatialGDK::EntityViewElement& Element = Iter.Value;

		// Get all worker system entities.
		const ComponentData* ServerSystemWorkerData = Element.Components.FindByPredicate([](const auto& CompData) {
			return CompData.GetComponentId() == SpatialConstants::WORKER_COMPONENT_ID;
		});

		if (ServerSystemWorkerData == nullptr)
		{
			continue;
		}

		Worker WorkerComponent = Worker(ServerSystemWorkerData->GetUnderlying());
		if (WorkerComponent.WorkerType.Equals(SpatialConstants::DefaultServerWorkerType.ToString()))
		{
			ServerSystemEntities.Emplace(EntityId);
		}
	}

	return ServerSystemEntities;
}

void WellKnownEntitySystem::ProcessEntityAdd(const Worker_EntityId EntityId)
{
	const EntityViewElement& Element = GDKSubView->GetView()[EntityId];
	for (const ComponentData& ComponentData : Element.Components)
	{
		ProcessComponentAdd(EntityId, ComponentData.GetComponentId(), ComponentData.GetUnderlying());
	}
	for (const Worker_ComponentSetId ComponentId : Element.Authority)
	{
		ProcessAuthorityGain(EntityId, ComponentId);
	}
}

void WellKnownEntitySystem::OnMapLoaded() const
{
	if (GlobalStateManager != nullptr && !GlobalStateManager->GetCanBeginPlay()
		&& GDKSubView->HasAuthority(GlobalStateManager->GlobalStateManagerEntityId,
									SpatialConstants::GDK_KNOWN_ENTITY_AUTH_COMPONENT_SET_ID))
	{
		// ServerTravel - Increment the session id, so users don't rejoin the old game.
		GlobalStateManager->TriggerBeginPlay();
		GlobalStateManager->SetDeploymentState();
		GlobalStateManager->SetAcceptingPlayers(true);
		GlobalStateManager->IncrementSessionID();
	}
}

// This is only called if this worker has been selected by SpatialOS to be authoritative
// for the TranslationManager, otherwise the manager will never be instantiated.
void WellKnownEntitySystem::InitializeVirtualWorkerTranslationManager()
{
	VirtualWorkerTranslationManager = MakeUnique<SpatialVirtualWorkerTranslationManager>(Connection, VirtualWorkerTranslator);
	VirtualWorkerTranslationManager->SetNumberOfVirtualWorkers(NumberOfWorkers);
}

void WellKnownEntitySystem::MaybeClaimSnapshotPartition()
{
	// Perform a naive leader election where we wait for the correct number of server workers to be present in the deployment, and then
	// whichever server has the lowest server worker entity ID becomes the leader and claims the snapshot partition.
	const Worker_EntityId LocalServerWorkerEntityId = GlobalStateManager->GetLocalServerWorkerEntityId();
	checkf(LocalServerWorkerEntityId != SpatialConstants::INVALID_ENTITY_ID,
		   TEXT("MaybeClaimSnapshotPartition aborted due to lack of local server worker entity"));

	Worker_EntityId LowestEntityId = LocalServerWorkerEntityId;

	int ServerCount = 0;
	for (const auto& Iter : GDKSubView->GetView())
	{
		const Worker_EntityId EntityId = Iter.Key;
		const SpatialGDK::EntityViewElement& Element = Iter.Value;

		const ComponentData* ServerWorkerEntityData = Element.Components.FindByPredicate([](const ComponentData& CompData) {
			return CompData.GetComponentId() == SpatialConstants::SERVER_WORKER_COMPONENT_ID;
		});

		if (ServerWorkerEntityData != nullptr)
		{
			Schema_Object* ServerWorkerComponentFields = Schema_GetComponentDataFields(ServerWorkerEntityData->GetUnderlying());
			const Worker_EntityId SystemEntityID =
				Schema_GetEntityId(ServerWorkerComponentFields, SpatialConstants::SERVER_WORKER_SYSTEM_ENTITY_ID);

			// This logic is for handling what happens if the snapshot partition auth server crashes.
			// If a different server crashes, the VTM auth (snapshot partition auth) server will delete the crashed
			// server entity, clean up translation data structure, and wait for the the recovered server.
			// If the VTM auth server crashes, we need
			// 1) the crashed server worker entity to be deleted - which we find here
			// 2) choose a new server to claim the snapshot partition - which we do here by doing a continue; which
			//    ignores the crashed VTM auth server entity ID in the lowest check.
			if (bSnapshotPartitionAuthServerCrashInProgress && SystemEntityID == SnapshotPartitionServerSystemEntity)
			{
				continue;
			}

			ServerCount++;

			if (EntityId < LowestEntityId)
			{
				LowestEntityId = EntityId;
			}
		}
	}

	// If we're the new snapshot partition auth server
	if (LocalServerWorkerEntityId == LowestEntityId)
	{
		if (bSnapshotPartitionAuthServerCrashInProgress)
		{
			UE_LOG(LogWellKnownEntitySystem, Warning, TEXT("Server reclaiming snapshot partition auth from crashed server"));
		}
		else if (ServerCount < NumberOfWorkers)
		{
			// We're starting up a fresh deployment but not all servers have started yet.
			return;
		}
		else if (ServerCount > NumberOfWorkers)
		{
			UE_LOG(LogWellKnownEntitySystem, Warning,
				   TEXT("MaybeClaimSnapshotPartition found too many server worker entities, expected %d got %d. Did you launch too many "
						"servers?"),
				   NumberOfWorkers, ServerCount);
		}

		GlobalStateManager->ClaimSnapshotPartition();
	}
}

} // Namespace SpatialGDK
