// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "Interop/SpatialReceiver.h"

#include "Engine/Engine.h"
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/PlayerState.h"
#include "Kismet/GameplayStatics.h"
#include "TimerManager.h"

#include "EngineClasses/SpatialActorChannel.h"
#include "EngineClasses/SpatialFastArrayNetSerialize.h"
#include "EngineClasses/SpatialLoadBalanceEnforcer.h"
#include "EngineClasses/SpatialNetConnection.h"
#include "EngineClasses/SpatialNetDriverDebugContext.h"
#include "EngineClasses/SpatialPackageMapClient.h"
#include "EngineClasses/SpatialVirtualWorkerTranslator.h"
#include "Interop/Connection/SpatialEventTracer.h"
#include "Interop/Connection/SpatialTraceEventBuilder.h"
#include "Interop/Connection/SpatialWorkerConnection.h"
#include "Interop/GlobalStateManager.h"
#include "Interop/SpatialPlayerSpawner.h"
#include "Interop/SpatialSender.h"
#include "Schema/DynamicComponent.h"
#include "Schema/RPCPayload.h"
#include "Schema/SpawnData.h"
#include "Schema/Tombstone.h"
#include "Schema/UnrealMetadata.h"
#include "SpatialConstants.h"
#include "Utils/ComponentReader.h"
#include "Utils/ErrorCodeRemapping.h"
#include "Utils/GDKPropertyMacros.h"
#include "Utils/RepLayoutUtils.h"
#include "Utils/SpatialDebugger.h"
#include "Utils/SpatialMetrics.h"

DEFINE_LOG_CATEGORY(LogSpatialReceiver);

DECLARE_CYCLE_STAT(TEXT("PendingOpsOnChannel"), STAT_SpatialPendingOpsOnChannel, STATGROUP_SpatialNet);

DECLARE_CYCLE_STAT(TEXT("Receiver LeaveCritSection"), STAT_ReceiverLeaveCritSection, STATGROUP_SpatialNet);
DECLARE_CYCLE_STAT(TEXT("Receiver RemoveEntity"), STAT_ReceiverRemoveEntity, STATGROUP_SpatialNet);
DECLARE_CYCLE_STAT(TEXT("Receiver AddComponent"), STAT_ReceiverAddComponent, STATGROUP_SpatialNet);
DECLARE_CYCLE_STAT(TEXT("Receiver ComponentUpdate"), STAT_ReceiverComponentUpdate, STATGROUP_SpatialNet);
DECLARE_CYCLE_STAT(TEXT("Receiver ApplyData"), STAT_ReceiverApplyData, STATGROUP_SpatialNet);
DECLARE_CYCLE_STAT(TEXT("Receiver ApplyHandover"), STAT_ReceiverApplyHandover, STATGROUP_SpatialNet);
DECLARE_CYCLE_STAT(TEXT("Receiver HandleRPC"), STAT_ReceiverHandleRPC, STATGROUP_SpatialNet);
DECLARE_CYCLE_STAT(TEXT("Receiver CommandRequest"), STAT_ReceiverCommandRequest, STATGROUP_SpatialNet);
DECLARE_CYCLE_STAT(TEXT("Receiver CommandResponse"), STAT_ReceiverCommandResponse, STATGROUP_SpatialNet);
DECLARE_CYCLE_STAT(TEXT("Receiver AuthorityChange"), STAT_ReceiverAuthChange, STATGROUP_SpatialNet);
DECLARE_CYCLE_STAT(TEXT("Receiver ReserveEntityIds"), STAT_ReceiverReserveEntityIds, STATGROUP_SpatialNet);
DECLARE_CYCLE_STAT(TEXT("Receiver CreateEntityResponse"), STAT_ReceiverCreateEntityResponse, STATGROUP_SpatialNet);
DECLARE_CYCLE_STAT(TEXT("Receiver EntityQueryResponse"), STAT_ReceiverEntityQueryResponse, STATGROUP_SpatialNet);
DECLARE_CYCLE_STAT(TEXT("Receiver FlushRemoveComponents"), STAT_ReceiverFlushRemoveComponents, STATGROUP_SpatialNet);
DECLARE_CYCLE_STAT(TEXT("Receiver ReceiveActor"), STAT_ReceiverReceiveActor, STATGROUP_SpatialNet);
DECLARE_CYCLE_STAT(TEXT("Receiver RemoveActor"), STAT_ReceiverRemoveActor, STATGROUP_SpatialNet);
DECLARE_CYCLE_STAT(TEXT("Receiver ApplyRPC"), STAT_ReceiverApplyRPC, STATGROUP_SpatialNet);
using namespace SpatialGDK;

void USpatialReceiver::Init(USpatialNetDriver* InNetDriver, FTimerManager* InTimerManager, SpatialRPCService* InRPCService,
							SpatialGDK::SpatialEventTracer* InEventTracer)
{
	NetDriver = InNetDriver;
	StaticComponentView = InNetDriver->StaticComponentView;
	Sender = InNetDriver->Sender;
	PackageMap = InNetDriver->PackageMap;
	ClassInfoManager = InNetDriver->ClassInfoManager;
	GlobalStateManager = InNetDriver->GlobalStateManager;
	TimerManager = InTimerManager;
	RPCService = InRPCService;
	EventTracer = InEventTracer;
}

void USpatialReceiver::OnCriticalSection(bool InCriticalSection)
{
	if (InCriticalSection)
	{
		EnterCriticalSection();
	}
	else
	{
		LeaveCriticalSection();
	}
}

void USpatialReceiver::EnterCriticalSection()
{
	UE_LOG(LogSpatialReceiver, Verbose, TEXT("Entering critical section."));
	check(!bInCriticalSection);
	bInCriticalSection = true;
}

void USpatialReceiver::LeaveCriticalSection()
{
	SCOPE_CYCLE_COUNTER(STAT_ReceiverLeaveCritSection);

	UE_LOG(LogSpatialReceiver, Verbose, TEXT("Leaving critical section."));
	check(bInCriticalSection);

	for (Worker_EntityId& PendingAddEntity : PendingAddActors)
	{
		ReceiveActor(PendingAddEntity);
		if (!IsEntityWaitingForAsyncLoad(PendingAddEntity))
		{
			OnEntityAddedDelegate.Broadcast(PendingAddEntity);
		}

		PendingAddComponents.RemoveAll([PendingAddEntity](const PendingAddComponentWrapper& Component) {
			return Component.EntityId == PendingAddEntity && Component.ComponentId != SpatialConstants::GDK_DEBUG_COMPONENT_ID;
		});
	}

	// The reason the AuthorityChange processing is split according to authority is to avoid cases
	// where we receive data while being authoritative, as that could be unintuitive to the game devs.
	// We process Lose Auth -> Add Components -> Gain Auth. A common thing that happens is that on handover we get
	// ComponentData -> Gain Auth, and with this split you receive data as if you were a client to get the most up-to-date state,
	// and then gain authority. Similarly, you first lose authority, and then receive data, in the opposite situation.
	for (Worker_ComponentSetAuthorityChangeOp& PendingAuthorityChange : PendingAuthorityChanges)
	{
		if (PendingAuthorityChange.authority != WORKER_AUTHORITY_AUTHORITATIVE)
		{
			HandleActorAuthority(PendingAuthorityChange);
		}
	}

	for (PendingAddComponentWrapper& PendingAddComponent : PendingAddComponents)
	{
		if (ClassInfoManager->IsGeneratedQBIMarkerComponent(PendingAddComponent.ComponentId))
		{
			continue;
		}

		if (PendingAddComponent.ComponentId == SpatialConstants::GDK_DEBUG_COMPONENT_ID)
		{
			if (NetDriver->DebugCtx != nullptr)
			{
				NetDriver->DebugCtx->OnDebugComponentUpdateReceived(PendingAddComponent.EntityId);
			}
			continue;
		}

		USpatialActorChannel* Channel = NetDriver->GetActorChannelByEntityId(PendingAddComponent.EntityId);
		if (Channel == nullptr)
		{
			UE_LOG(LogSpatialReceiver, Error,
				   TEXT("Got an add component for an entity that doesn't have an associated actor channel."
						" Entity id: %lld, component id: %d."),
				   PendingAddComponent.EntityId, PendingAddComponent.ComponentId);
			continue;
		}
		if (Channel->bCreatedEntity)
		{
			// Allows servers to change state if they are going to be authoritative, without us overwriting it with old data.
			// TODO: UNR-3457 to remove this workaround.
			continue;
		}

		UE_LOG(
			LogSpatialReceiver, Verbose,
			TEXT("Add component inside of a critical section, outside of an add entity, being handled: entity id %lld, component id %d."),
			PendingAddComponent.EntityId, PendingAddComponent.ComponentId);
		HandleIndividualAddComponent(PendingAddComponent.EntityId, PendingAddComponent.ComponentId, MoveTemp(PendingAddComponent.Data));
	}

	for (Worker_ComponentSetAuthorityChangeOp& PendingAuthorityChange : PendingAuthorityChanges)
	{
		if (PendingAuthorityChange.authority == WORKER_AUTHORITY_AUTHORITATIVE)
		{
			HandleActorAuthority(PendingAuthorityChange);
		}
	}

	// Mark that we've left the critical section.
	bInCriticalSection = false;
	PendingAddActors.Empty();
	PendingAddComponents.Empty();
	PendingAuthorityChanges.Empty();
}

void USpatialReceiver::OnAddEntity(const Worker_AddEntityOp& Op)
{
	UE_LOG(LogSpatialReceiver, Verbose, TEXT("AddEntity: %lld"), Op.entity_id);
}

void USpatialReceiver::OnAddComponent(const Worker_AddComponentOp& Op)
{
	SCOPE_CYCLE_COUNTER(STAT_ReceiverAddComponent);
	UE_LOG(LogSpatialReceiver, Verbose, TEXT("AddComponent component ID: %u entity ID: %lld"), Op.data.component_id, Op.entity_id);

	if (IsEntityWaitingForAsyncLoad(Op.entity_id))
	{
		QueueAddComponentOpForAsyncLoad(Op);
		return;
	}

	if (HasEntityBeenRequestedForDelete(Op.entity_id))
	{
		return;
	}

	// Remove all RemoveComponentOps that have already been received and have the same entityId and componentId as the AddComponentOp.
	// TODO: This can probably be removed when spatial view is added.
	QueuedRemoveComponentOps.RemoveAll([&Op](const Worker_RemoveComponentOp& RemoveComponentOp) {
		return RemoveComponentOp.entity_id == Op.entity_id && RemoveComponentOp.component_id == Op.data.component_id;
	});

	switch (Op.data.component_id)
	{
	case SpatialConstants::METADATA_COMPONENT_ID:
	case SpatialConstants::POSITION_COMPONENT_ID:
	case SpatialConstants::PERSISTENCE_COMPONENT_ID:
	case SpatialConstants::SPAWN_DATA_COMPONENT_ID:
	case SpatialConstants::PLAYER_SPAWNER_COMPONENT_ID:
	case SpatialConstants::INTEREST_COMPONENT_ID:
	case SpatialConstants::NOT_STREAMED_COMPONENT_ID:
	case SpatialConstants::GSM_SHUTDOWN_COMPONENT_ID:
	case SpatialConstants::HEARTBEAT_COMPONENT_ID:
	case SpatialConstants::NETMULTICAST_RPCS_COMPONENT_ID_LEGACY:
	case SpatialConstants::RPCS_ON_ENTITY_CREATION_ID:
	case SpatialConstants::DEBUG_METRICS_COMPONENT_ID:
	case SpatialConstants::ALWAYS_RELEVANT_COMPONENT_ID:
	case SpatialConstants::VISIBLE_COMPONENT_ID:
	case SpatialConstants::CLIENT_RPC_ENDPOINT_COMPONENT_ID_LEGACY:
	case SpatialConstants::SERVER_RPC_ENDPOINT_COMPONENT_ID_LEGACY:
	case SpatialConstants::SERVER_TO_SERVER_COMMAND_ENDPOINT_COMPONENT_ID:
	case SpatialConstants::CLIENT_ENDPOINT_COMPONENT_ID:
	case SpatialConstants::SERVER_ENDPOINT_COMPONENT_ID:
	case SpatialConstants::SPATIAL_DEBUGGING_COMPONENT_ID:
	case SpatialConstants::SERVER_WORKER_COMPONENT_ID:
	case SpatialConstants::AUTHORITY_INTENT_COMPONENT_ID:
	case SpatialConstants::COMPONENT_PRESENCE_COMPONENT_ID:
	case SpatialConstants::NET_OWNING_CLIENT_WORKER_COMPONENT_ID:
	case SpatialConstants::AUTHORITY_DELEGATION_COMPONENT_ID:
	case SpatialConstants::DEPLOYMENT_MAP_COMPONENT_ID:
	case SpatialConstants::STARTUP_ACTOR_MANAGER_COMPONENT_ID:
	case SpatialConstants::VIRTUAL_WORKER_TRANSLATION_COMPONENT_ID:
	case SpatialConstants::MULTICAST_RPCS_COMPONENT_ID:
		// We either don't care about processing these components or we only need to store
		// the data (which is handled by the SpatialStaticComponentView).
		return;
	case SpatialConstants::UNREAL_METADATA_COMPONENT_ID:
		// The UnrealMetadata component is used to indicate when an Actor needs to be created from the entity.
		// This means we need to be inside a critical section, otherwise we may not have all the requisite
		// information at the point of creating the Actor.
		check(bInCriticalSection);

		// PendingAddActor should only be populated with actors we actually want to check out.
		// So a local and ready actor should not be considered as an actor pending addition.
		// Nitty-gritty implementation side effect is also that we do not want to stomp
		// the local state of a not ready-actor, because it is locally authoritative and will remain
		// that way until it is marked as ready. Putting it in PendingAddActor has the side effect
		// that the received component data will get dropped (likely outdated data), and is
		// something we do not wish to happen for ready actor (likely new data received through
		// a component refresh on authority delegation).
		{
			AActor* EntityActor = Cast<AActor>(PackageMap->GetObjectFromEntityId(Op.entity_id));
			if (EntityActor == nullptr || !EntityActor->IsActorReady())
			{
				PendingAddActors.AddUnique(Op.entity_id);
			}
		}
		return;
	case SpatialConstants::WORKER_COMPONENT_ID:
		if (NetDriver->IsServer() && !WorkerConnectionEntities.Contains(Op.entity_id))
		{
			// Register system identity for a worker connection, to know when a player has disconnected.
			Worker* WorkerData = StaticComponentView->GetComponentData<Worker>(Op.entity_id);
			WorkerConnectionEntities.Add(Op.entity_id, WorkerData->WorkerId);
			UE_LOG(LogSpatialReceiver, Verbose, TEXT("Worker %s 's system identity was checked out."), *WorkerData->WorkerId);
		}
		return;
	case SpatialConstants::TOMBSTONE_COMPONENT_ID:
		RemoveActor(Op.entity_id);
		return;
	case SpatialConstants::DORMANT_COMPONENT_ID:
		if (USpatialActorChannel* Channel = NetDriver->GetActorChannelByEntityId(Op.entity_id))
		{
			NetDriver->AddPendingDormantChannel(Channel);
		}
		else
		{
			// This would normally get registered through the channel cleanup, but we don't have one for this entity
			NetDriver->RegisterDormantEntityId(Op.entity_id);
		}
		return;
	case SpatialConstants::GDK_DEBUG_COMPONENT_ID:
		if (NetDriver->DebugCtx != nullptr)
		{
			if (bInCriticalSection)
			{
				PendingAddComponents.AddUnique(
					PendingAddComponentWrapper(Op.entity_id, Op.data.component_id, MakeUnique<DynamicComponent>(Op.data)));
			}
			else
			{
				NetDriver->DebugCtx->OnDebugComponentUpdateReceived(Op.entity_id);
			}
		}
		return;
	}

	if (Op.data.component_id < SpatialConstants::MAX_RESERVED_SPATIAL_SYSTEM_COMPONENT_ID)
	{
		return;
	}

	if (ClassInfoManager->IsGeneratedQBIMarkerComponent(Op.data.component_id))
	{
		return;
	}

	if (bInCriticalSection)
	{
		PendingAddComponents.AddUnique(
			PendingAddComponentWrapper(Op.entity_id, Op.data.component_id, MakeUnique<DynamicComponent>(Op.data)));
	}
	else
	{
		HandleIndividualAddComponent(Op.entity_id, Op.data.component_id, MakeUnique<SpatialGDK::DynamicComponent>(Op.data));
	}
}

void USpatialReceiver::OnRemoveEntity(const Worker_RemoveEntityOp& Op)
{
	SCOPE_CYCLE_COUNTER(STAT_ReceiverRemoveEntity);

	// Stop tracking if the entity was deleted as a result of deleting the actor during creation.
	// This assumes that authority will be gained before interest is gained and lost.
	const int32 RetiredActorIndex = EntitiesToRetireOnAuthorityGain.IndexOfByPredicate([Op](const DeferredRetire& Retire) {
		return Op.entity_id == Retire.EntityId;
	});
	if (RetiredActorIndex != INDEX_NONE)
	{
		EntitiesToRetireOnAuthorityGain.RemoveAtSwap(RetiredActorIndex);
	}

	OnEntityRemovedDelegate.Broadcast(Op.entity_id);

	if (NetDriver->IsServer())
	{
		// Check to see if we are removing a system entity for a worker connection. If so clean up the ClientConnection to delete any and
		// all actors for this connection's controller.
		if (FString* WorkerName = WorkerConnectionEntities.Find(Op.entity_id))
		{
			TWeakObjectPtr<USpatialNetConnection> ClientConnectionPtr = NetDriver->FindClientConnectionFromWorkerEntityId(Op.entity_id);
			if (USpatialNetConnection* ClientConnection = ClientConnectionPtr.Get())
			{
				if (APlayerController* Controller = ClientConnection->GetPlayerController(/*InWorld*/ nullptr))
				{
					Worker_EntityId PCEntity = PackageMap->GetEntityIdFromObject(Controller);
					if (AuthorityPlayerControllerConnectionMap.Find(PCEntity))
					{
						UE_LOG(LogSpatialReceiver, Verbose, TEXT("Worker %s disconnected after its system identity was removed."),
							   *(*WorkerName));
						CloseClientConnection(ClientConnection, PCEntity);
					}
				}
			}
			WorkerConnectionEntities.Remove(Op.entity_id);
		}
	}
}

void USpatialReceiver::OnRemoveComponent(const Worker_RemoveComponentOp& Op)
{
	// We should exit early if we're receiving a duplicate RemoveComponent op. This can happen with dynamic
	// components enabled. We detect if the op is a duplicate via the queue of ops to be processed (duplicate
	// op receive in the same op list).
	if (QueuedRemoveComponentOps.ContainsByPredicate([&Op](const Worker_RemoveComponentOp& QueuedOp) {
			return QueuedOp.entity_id == Op.entity_id && QueuedOp.component_id == Op.component_id;
		}))
	{
		return;
	}

	if (Op.component_id == SpatialConstants::UNREAL_METADATA_COMPONENT_ID)
	{
		if (IsEntityWaitingForAsyncLoad(Op.entity_id))
		{
			// Pretend we never saw this actor.
			EntitiesWaitingForAsyncLoad.Remove(Op.entity_id);
		}
		else
		{
			if (bInCriticalSection)
			{
				PendingAddActors.Remove(Op.entity_id);
			}
			RemoveActor(Op.entity_id);
		}
	}

	if (bInCriticalSection)
	{
		// Cancel out the Pending adds to avoid getting errors if an actor is not created for these components when leaving the critical
		// section. Paired Add/Remove could happen, and processing the queued ops would happen too late to prevent it.
		PendingAddComponents.RemoveAll([&Op](const PendingAddComponentWrapper& PendingAdd) {
			return PendingAdd.EntityId == Op.entity_id && PendingAdd.ComponentId == Op.component_id;
		});
	}

	// We are queuing here because if an Actor is removed from your view, remove component ops will be
	// generated and sent first, and then the RemoveEntityOp will be sent. In this case, we only want
	// to delete the Actor and not delete the subobjects that the RemoveComponent relate to.
	// So we queue RemoveComponentOps then process the RemoveEntityOps normally, and then apply the
	// RemoveComponentOps in ProcessRemoveComponent. Any RemoveComponentOps that relate to delete entities
	// will be dropped in ProcessRemoveComponent.
	QueuedRemoveComponentOps.Add(Op);
}

void USpatialReceiver::FlushRemoveComponentOps()
{
	SCOPE_CYCLE_COUNTER(STAT_ReceiverFlushRemoveComponents);

	for (const auto& Op : QueuedRemoveComponentOps)
	{
		ProcessRemoveComponent(Op);
	}

	QueuedRemoveComponentOps.Empty();
}

void USpatialReceiver::DropQueuedRemoveComponentOpsForEntity(Worker_EntityId EntityId)
{
	// Drop any remove components ops for a removed entity because we cannot process them once we no longer see the entity.
	for (auto& RemoveComponentOp : QueuedRemoveComponentOps)
	{
		if (RemoveComponentOp.entity_id == EntityId)
		{
			// Zero component op to prevent array resize
			RemoveComponentOp = Worker_RemoveComponentOp{};
		}
	}
}

USpatialActorChannel* USpatialReceiver::GetOrRecreateChannelForDomantActor(AActor* Actor, Worker_EntityId EntityID)
{
	// Receive would normally create channel in ReceiveActor - this function is used to recreate the channel after waking up a dormant actor
	USpatialActorChannel* Channel = NetDriver->GetOrCreateSpatialActorChannel(Actor);
	if (Channel == nullptr)
	{
		return nullptr;
	}
	check(!Channel->bCreatingNewEntity);
	check(Channel->GetEntityId() == EntityID);

	NetDriver->RemovePendingDormantChannel(Channel);
	NetDriver->UnregisterDormantEntityId(EntityID);

	return Channel;
}

void USpatialReceiver::ProcessRemoveComponent(const Worker_RemoveComponentOp& Op)
{
	if (IsEntityWaitingForAsyncLoad(Op.entity_id))
	{
		QueueRemoveComponentOpForAsyncLoad(Op);
		return;
	}

	// We want to do nothing for RemoveComponent ops for which we never received a corresponding
	// AddComponent op. This can happen because of the worker SDK generating a RemoveComponent op
	// when a worker receives authority over a component without having already received the
	// AddComponent op. The generation is a known part of the worker SDK we need to tolerate for
	// enabling dynamic components, and having authority ACL entries without having the component
	// data present on an entity is permitted as part of our Unreal dynamic component implementation.
	if (!StaticComponentView->HasComponent(Op.entity_id, Op.component_id))
	{
		return;
	}

	if (AActor* Actor = Cast<AActor>(PackageMap->GetObjectFromEntityId(Op.entity_id).Get()))
	{
		FUnrealObjectRef ObjectRef(Op.entity_id, Op.component_id);
		if (Op.component_id == SpatialConstants::DORMANT_COMPONENT_ID)
		{
			GetOrRecreateChannelForDomantActor(Actor, Op.entity_id);
		}
		else if (UObject* Object = PackageMap->GetObjectFromUnrealObjectRef(ObjectRef).Get())
		{
			if (USpatialActorChannel* Channel = NetDriver->GetActorChannelByEntityId(Op.entity_id))
			{
				TWeakObjectPtr<UObject> WeakPtr(Object);
				Channel->OnSubobjectDeleted(ObjectRef, Object, WeakPtr);

				Actor->OnSubobjectDestroyFromReplication(Object);

				Object->PreDestroyFromReplication();
				Object->MarkPendingKill();

				PackageMap->RemoveSubobject(FUnrealObjectRef(Op.entity_id, Op.component_id));
			}
		}
	}

	StaticComponentView->OnRemoveComponent(Op);
}

void USpatialReceiver::UpdateShadowData(Worker_EntityId EntityId)
{
	USpatialActorChannel* ActorChannel = NetDriver->GetActorChannelByEntityId(EntityId);
	ActorChannel->UpdateShadowData();
}

void USpatialReceiver::OnAuthorityChange(const Worker_ComponentSetAuthorityChangeOp& Op)
{
	if (HasEntityBeenRequestedForDelete(Op.entity_id))
	{
		if (Op.authority == WORKER_AUTHORITY_AUTHORITATIVE && Op.component_set_id == SpatialConstants::WELL_KNOWN_COMPONENT_SET_ID)
		{
			HandleEntityDeletedAuthority(Op.entity_id);
		}
		return;
	}

	// Update this worker's view of authority. We do this here as this is when the worker is first notified of the authority change.
	// This way systems that depend on having non-stale state can function correctly.
	StaticComponentView->OnAuthorityChange(Op);

	SCOPE_CYCLE_COUNTER(STAT_ReceiverAuthChange);
	if (IsEntityWaitingForAsyncLoad(Op.entity_id))
	{
		QueueAuthorityOpForAsyncLoad(Op);
		return;
	}

	if (bInCriticalSection)
	{
		// The actor receiving flow requires authority to be handled after all components have been received, so buffer those if we
		// are in a critical section to be handled later.
		PendingAuthorityChanges.Add(Op);
		return;
	}

	HandleActorAuthority(Op);
}

void USpatialReceiver::HandlePlayerLifecycleAuthority(const Worker_ComponentSetAuthorityChangeOp& Op, APlayerController* PlayerController)
{
	UE_LOG(LogSpatialReceiver, Verbose, TEXT("HandlePlayerLifecycleAuthority for PlayerController %s."),
		   *AActor::GetDebugName(PlayerController));

	// Server initializes heartbeat logic based on its authority over the position component,
	// client does the same for heartbeat component
	if ((NetDriver->IsServer() && Op.component_set_id == SpatialConstants::WELL_KNOWN_COMPONENT_SET_ID)
		|| (!NetDriver->IsServer() && Op.component_set_id == SpatialConstants::HEARTBEAT_COMPONENT_ID))
	{
		if (Op.authority == WORKER_AUTHORITY_AUTHORITATIVE)
		{
			if (USpatialNetConnection* Connection = Cast<USpatialNetConnection>(PlayerController->GetNetConnection()))
			{
				if (NetDriver->IsServer())
				{
					AuthorityPlayerControllerConnectionMap.Add(Op.entity_id, Connection);
				}
				Connection->InitHeartbeat(TimerManager, Op.entity_id);
			}
		}
		else if (Op.authority == WORKER_AUTHORITY_NOT_AUTHORITATIVE)
		{
			if (NetDriver->IsServer())
			{
				AuthorityPlayerControllerConnectionMap.Remove(Op.entity_id);
			}
			if (USpatialNetConnection* Connection = Cast<USpatialNetConnection>(PlayerController->GetNetConnection()))
			{
				Connection->DisableHeartbeat();
			}
		}
	}
}

void USpatialReceiver::HandleActorAuthority(const Worker_ComponentSetAuthorityChangeOp& Op)
{
	if (NetDriver->SpatialDebugger != nullptr && Op.authority == WORKER_AUTHORITY_AUTHORITATIVE
		&& Op.component_set_id == SpatialConstants::AUTHORITY_INTENT_COMPONENT_ID)
	{
		NetDriver->SpatialDebugger->ActorAuthorityChanged(Op);
	}

	AActor* Actor = Cast<AActor>(NetDriver->PackageMap->GetObjectFromEntityId(Op.entity_id));
	if (Actor == nullptr)
	{
		return;
	}

	// TODO - Using bActorHadAuthority should be replaced with better tracking system to Actor entity creation [UNR-3960]
	const bool bActorHadAuthority = Actor->HasAuthority();

	USpatialActorChannel* Channel = NetDriver->GetActorChannelByEntityId(Op.entity_id);

	if (Channel != nullptr)
	{
		if (Op.component_set_id == SpatialConstants::WELL_KNOWN_COMPONENT_SET_ID)
		{
			Channel->SetServerAuthority(Op.authority == WORKER_AUTHORITY_AUTHORITATIVE);
		}
		else if (Op.component_set_id
				 == SpatialConstants::GetClientAuthorityComponent(GetDefault<USpatialGDKSettings>()->UseRPCRingBuffer()))
		{
			Channel->SetClientAuthority(Op.authority == WORKER_AUTHORITY_AUTHORITATIVE);
		}
	}

	if (APlayerController* PlayerController = Cast<APlayerController>(Actor))
	{
		HandlePlayerLifecycleAuthority(Op, PlayerController);
	}

	if (NetDriver->IsServer())
	{
		// TODO UNR-955 - Remove this once batch reservation of EntityIds are in.
		if (Op.authority == WORKER_AUTHORITY_AUTHORITATIVE)
		{
			Sender->ProcessUpdatesQueuedUntilAuthority(Op.entity_id, Op.component_set_id);
		}

		// If we became authoritative over the position component. set our role to be ROLE_Authority
		// and set our RemoteRole to be ROLE_AutonomousProxy if the actor has an owning connection.
		// Note: Pawn, PlayerController, and PlayerState for player-owned characters can arrive in
		// any order on non-authoritative servers, so it's possible that we don't yet know if a pawn
		// is player controlled when gaining authority over the pawn and need to wait for the player
		// state. Likewise, it's possible that the player state doesn't have a pointer to its pawn
		// yet, so we need to wait for the pawn to arrive.
		if (Op.component_set_id == SpatialConstants::WELL_KNOWN_COMPONENT_SET_ID)
		{
			if (Op.authority == WORKER_AUTHORITY_AUTHORITATIVE)
			{
				const bool bDormantActor = (Actor->NetDormancy >= DORM_DormantAll);

				if (IsValid(Channel) || bDormantActor)
				{
					Actor->Role = ROLE_Authority;
					Actor->RemoteRole = ROLE_SimulatedProxy;

					if (Actor->IsA<APlayerController>())
					{
						Actor->RemoteRole = ROLE_AutonomousProxy;
					}
					else if (APawn* Pawn = Cast<APawn>(Actor))
					{
						// The following check will return false on non-authoritative servers if the PlayerState hasn't been received yet.
						if (Pawn->IsPlayerControlled())
						{
							Pawn->RemoteRole = ROLE_AutonomousProxy;
						}
					}
					else if (const APlayerState* PlayerState = Cast<APlayerState>(Actor))
					{
						// The following check will return false on non-authoritative servers if the Pawn hasn't been received yet.
						if (APawn* PawnFromPlayerState = PlayerState->GetPawn())
						{
							if (PawnFromPlayerState->IsPlayerControlled() && PawnFromPlayerState->HasAuthority())
							{
								PawnFromPlayerState->RemoteRole = ROLE_AutonomousProxy;
							}
						}
					}

					if (!bDormantActor)
					{
						UpdateShadowData(Op.entity_id);
					}

					// TODO - Using bActorHadAuthority should be replaced with better tracking system to Actor entity creation [UNR-3960]
					// When receiving AuthorityGained from SpatialOS, the Actor role will be ROLE_Authority iff this
					// worker is receiving entity data for the 1st time after spawning the entity. In all other cases,
					// the Actor role will have been explicitly set to ROLE_SimulatedProxy previously during the
					// entity creation flow.
					if (bActorHadAuthority)
					{
						Actor->SetActorReady(true);
					}

					// We still want to call OnAuthorityGained if the Actor migrated to this worker or was loaded from a snapshot.
					Actor->OnAuthorityGained();
				}
				else
				{
					UE_LOG(LogSpatialReceiver, Verbose,
						   TEXT("Received authority over actor %s, with entity id %lld, which has no channel. This means it attempted to "
								"delete it earlier, when it had no authority. Retrying to delete now."),
						   *Actor->GetName(), Op.entity_id);
					Sender->RetireEntity(Op.entity_id, Actor->IsNetStartupActor());
				}
			}
			else if (Op.authority == WORKER_AUTHORITY_NOT_AUTHORITATIVE)
			{
				if (Channel != nullptr)
				{
					Channel->bCreatedEntity = false;
				}

				// With load-balancing enabled, we already set ROLE_SimulatedProxy and trigger OnAuthorityLost when we
				// set AuthorityDelegation to another server-side worker partition. This conditional exists to dodge
				// calling OnAuthorityLost twice.
				if (Actor->Role != ROLE_SimulatedProxy)
				{
					Actor->Role = ROLE_SimulatedProxy;
					Actor->RemoteRole = ROLE_Authority;

					Actor->OnAuthorityLost();
				}
			}
		}

		// Subobject Delegation
		TPair<Worker_EntityId_Key, Worker_ComponentSetId> EntityComponentPair =
			MakeTuple(static_cast<Worker_EntityId_Key>(Op.entity_id), Op.component_set_id);
		if (TSharedRef<FPendingSubobjectAttachment>* PendingSubobjectAttachmentPtr =
				PendingEntitySubobjectDelegations.Find(EntityComponentPair))
		{
			FPendingSubobjectAttachment& PendingSubobjectAttachment = PendingSubobjectAttachmentPtr->Get();

			PendingSubobjectAttachment.PendingAuthorityDelegations.Remove(Op.component_set_id);

			if (PendingSubobjectAttachment.PendingAuthorityDelegations.Num() == 0)
			{
				if (UObject* Object = PendingSubobjectAttachment.Subobject.Get())
				{
					if (IsValid(Channel))
					{
						// TODO: UNR-664 - We should track the bytes sent here and factor them into channel saturation.
						uint32 BytesWritten = 0;
						Sender->SendAddComponentForSubobject(Channel, Object, *PendingSubobjectAttachment.Info, BytesWritten);
					}
				}
			}

			PendingEntitySubobjectDelegations.Remove(EntityComponentPair);
		}
	}
	else if (Op.component_set_id == SpatialConstants::GetClientAuthorityComponent(GetDefault<USpatialGDKSettings>()->UseRPCRingBuffer()))
	{
		if (Channel != nullptr)
		{
			Channel->ClientProcessOwnershipChange(Op.authority == WORKER_AUTHORITY_AUTHORITATIVE);
		}

		// If we are a Pawn or PlayerController, our local role should be ROLE_AutonomousProxy. Otherwise ROLE_SimulatedProxy
		if (Actor->IsA<APawn>() || Actor->IsA<APlayerController>())
		{
			Actor->Role = (Op.authority == WORKER_AUTHORITY_AUTHORITATIVE) ? ROLE_AutonomousProxy : ROLE_SimulatedProxy;
		}
	}

	if (NetDriver->DebugCtx && Op.authority == WORKER_AUTHORITY_NOT_AUTHORITATIVE
		&& Op.component_set_id == SpatialConstants::GDK_DEBUG_COMPONENT_ID)
	{
		NetDriver->DebugCtx->OnDebugComponentAuthLost(Op.entity_id);
	}
}

bool USpatialReceiver::IsReceivedEntityTornOff(Worker_EntityId EntityId)
{
	// Check the pending add components, to find the root component for the received entity.
	for (PendingAddComponentWrapper& PendingAddComponent : PendingAddComponents)
	{
		if (PendingAddComponent.EntityId != EntityId
			|| ClassInfoManager->GetCategoryByComponentId(PendingAddComponent.ComponentId) != SCHEMA_Data)
		{
			continue;
		}

		UClass* Class = ClassInfoManager->GetClassByComponentId(PendingAddComponent.ComponentId);
		if (!Class->IsChildOf<AActor>())
		{
			continue;
		}

		const Worker_ComponentData ComponentData = PendingAddComponent.Data->Data.GetWorkerComponentData();
		Schema_Object* ComponentObject = Schema_GetComponentDataFields(ComponentData.schema_type);
		return GetBoolFromSchema(ComponentObject, SpatialConstants::ACTOR_TEAROFF_ID);
	}

	return false;
}

void USpatialReceiver::ReceiveActor(Worker_EntityId EntityId)
{
	SCOPE_CYCLE_COUNTER(STAT_ReceiverReceiveActor);

	checkf(NetDriver, TEXT("We should have a NetDriver whilst processing ops."));
	checkf(NetDriver->GetWorld(), TEXT("We should have a World whilst processing ops."));

	SpawnData* SpawnDataComp = StaticComponentView->GetComponentData<SpawnData>(EntityId);
	UnrealMetadata* UnrealMetadataComp = StaticComponentView->GetComponentData<UnrealMetadata>(EntityId);
	NetOwningClientWorker* NetOwningClientWorkerComp = StaticComponentView->GetComponentData<NetOwningClientWorker>(EntityId);

	// This function should only ever be called if we have received an unreal metadata component.
	check(UnrealMetadataComp != nullptr);

	const USpatialGDKSettings* SpatialGDKSettings = GetDefault<USpatialGDKSettings>();

	// Check if actor's class is loaded. If not, start async loading it and extract all data and
	// authority ops into a separate entry that will get processed once the loading is finished.
	const FString& ClassPath = UnrealMetadataComp->ClassPath;
	if (SpatialGDKSettings->bAsyncLoadNewClassesOnEntityCheckout && NeedToLoadClass(ClassPath))
	{
		StartAsyncLoadingClass(ClassPath, EntityId);
		return;
	}

	AActor* EntityActor = Cast<AActor>(PackageMap->GetObjectFromEntityId(EntityId));
	if (EntityActor != nullptr)
	{
		if (!EntityActor->IsActorReady())
		{
			UE_LOG(LogSpatialReceiver, Verbose, TEXT("%s: Entity %lld for Actor %s has been checked out on the worker which spawned it."),
				   *NetDriver->Connection->GetWorkerId(), EntityId, *EntityActor->GetName());
		}

		return;
	}

	UE_LOG(LogSpatialReceiver, Verbose,
		   TEXT("%s: Entity has been checked out on a worker which didn't spawn it. "
				"Entity ID: %lld"),
		   *NetDriver->Connection->GetWorkerId(), EntityId);

	UClass* Class = UnrealMetadataComp->GetNativeEntityClass();
	if (Class == nullptr)
	{
		UE_LOG(LogSpatialReceiver, Warning,
			   TEXT("The received actor with entity ID %lld couldn't be loaded. The actor (%s) will not be spawned."), EntityId,
			   *UnrealMetadataComp->ClassPath);
		return;
	}

	// Make sure ClassInfo exists
	const FClassInfo& ActorClassInfo = ClassInfoManager->GetOrCreateClassInfoByClass(Class);

	// If the received actor is torn off, don't bother spawning it.
	// (This is only needed due to the delay between tearoff and deleting the entity. See https://improbableio.atlassian.net/browse/UNR-841)
	if (IsReceivedEntityTornOff(EntityId))
	{
		UE_LOG(LogSpatialReceiver, Verbose,
			   TEXT("The received actor with entity ID %lld was already torn off. The actor will not be spawned."), EntityId);
		return;
	}

	EntityActor = TryGetOrCreateActor(UnrealMetadataComp, SpawnDataComp, NetOwningClientWorkerComp);

	if (EntityActor == nullptr)
	{
		// This could be nullptr if:
		// a stably named actor could not be found
		// the class couldn't be loaded
		return;
	}

	// RemoveActor immediately if we've received the tombstone component.
	if (NetDriver->StaticComponentView->HasComponent(EntityId, SpatialConstants::TOMBSTONE_COMPONENT_ID))
	{
		UE_LOG(LogSpatialReceiver, Verbose, TEXT("The received actor with entity ID %lld was tombstoned. The actor will not be spawned."),
			   EntityId);
		// We must first Resolve the EntityId to the Actor in order for RemoveActor to succeed.
		PackageMap->ResolveEntityActor(EntityActor, EntityId);
		RemoveActor(EntityId);
		return;
	}

	UNetConnection* Connection = NetDriver->GetSpatialOSNetConnection();

	if (NetDriver->IsServer())
	{
		if (APlayerController* PlayerController = Cast<APlayerController>(EntityActor))
		{
			// If entity is a PlayerController, create channel on the PlayerController's connection.
			Connection = PlayerController->NetConnection;
		}
	}

	if (Connection == nullptr)
	{
		UE_LOG(LogSpatialReceiver, Error,
			   TEXT("Unable to find SpatialOSNetConnection! Has this worker been disconnected from SpatialOS due to a timeout?"));
		return;
	}

	if (!PackageMap->ResolveEntityActor(EntityActor, EntityId))
	{
		UE_LOG(LogSpatialReceiver, Warning,
			   TEXT("Failed to resolve entity actor when receiving entity %lld. The actor (%s) will not be spawned."), EntityId,
			   *EntityActor->GetName());
		EntityActor->Destroy(true);
		return;
	}

	// Set up actor channel.
	USpatialActorChannel* Channel = NetDriver->GetActorChannelByEntityId(EntityId);
	if (Channel == nullptr)
	{
		Channel = Cast<USpatialActorChannel>(Connection->CreateChannelByName(
			NAME_Actor, NetDriver->IsServer() ? EChannelCreateFlags::OpenedLocally : EChannelCreateFlags::None));
	}

	if (Channel == nullptr)
	{
		UE_LOG(LogSpatialReceiver, Warning,
			   TEXT("Failed to create an actor channel when receiving entity %lld. The actor (%s) will not be spawned."), EntityId,
			   *EntityActor->GetName());
		EntityActor->Destroy(true);
		return;
	}

	if (Channel->Actor == nullptr)
	{
		Channel->SetChannelActor(EntityActor, ESetChannelActorFlags::None);
	}

	TArray<ObjectPtrRefPair> ObjectsToResolvePendingOpsFor;

	// Apply initial replicated properties.
	// This was moved to after FinishingSpawning because components existing only in blueprints aren't added until spawning is complete
	// Potentially we could split out the initial actor state and the initial component state
	for (PendingAddComponentWrapper& PendingAddComponent : PendingAddComponents)
	{
		if (ClassInfoManager->IsGeneratedQBIMarkerComponent(PendingAddComponent.ComponentId))
		{
			continue;
		}

		if (PendingAddComponent.EntityId == EntityId && PendingAddComponent.ComponentId != SpatialConstants::GDK_DEBUG_COMPONENT_ID)
		{
			ApplyComponentDataOnActorCreation(EntityId, PendingAddComponent.Data->Data.GetWorkerComponentData(), *Channel, ActorClassInfo,
											  ObjectsToResolvePendingOpsFor);
		}
	}

	// Resolve things like RepNotify or RPCs after applying component data.
	for (const ObjectPtrRefPair& ObjectToResolve : ObjectsToResolvePendingOpsFor)
	{
		ResolvePendingOperations(ObjectToResolve.Key, ObjectToResolve.Value);
	}

	if (!NetDriver->IsServer())
	{
		// Update interest on the entity's components after receiving initial component data (so Role and RemoteRole are properly set).

		// This is a bit of a hack unfortunately, among the core classes only PlayerController implements this function and it requires
		// a player index. For now we don't support split screen, so the number is always 0.
		if (EntityActor->IsA(APlayerController::StaticClass()))
		{
			uint8 PlayerIndex = 0;
			// FInBunch takes size in bits not bytes
			FInBunch Bunch(NetDriver->ServerConnection, &PlayerIndex, sizeof(PlayerIndex) * 8);
			EntityActor->OnActorChannelOpen(Bunch, NetDriver->ServerConnection);
		}
		else
		{
			FInBunch Bunch(NetDriver->ServerConnection);
			EntityActor->OnActorChannelOpen(Bunch, NetDriver->ServerConnection);
		}
	}

	// Any Actor created here will have been received over the wire as an entity so we can mark it ready.
	EntityActor->SetActorReady(false);

	// Taken from PostNetInit
	if (NetDriver->GetWorld()->HasBegunPlay() && !EntityActor->HasActorBegunPlay())
	{
		// The Actor role can be authority here if a PendingAddComponent processed above set the role.
		// Calling BeginPlay() with authority a 2nd time globally is always incorrect, so we set role
		// to SimulatedProxy and let the processing of authority ops (later in the LeaveCriticalSection
		// flow) take care of setting roles correctly.
		if (EntityActor->HasAuthority())
		{
			UE_LOG(LogSpatialReceiver, Error,
				   TEXT("Trying to unexpectedly spawn received network Actor with authority. Actor %s. Entity: %lld"),
				   *EntityActor->GetName(), EntityId);
			EntityActor->Role = ROLE_SimulatedProxy;
			EntityActor->RemoteRole = ROLE_Authority;
		}
		EntityActor->DispatchBeginPlay();
	}

	EntityActor->UpdateOverlaps();

	if (StaticComponentView->HasComponent(EntityId, SpatialConstants::DORMANT_COMPONENT_ID))
	{
		NetDriver->AddPendingDormantChannel(Channel);
	}
}

void USpatialReceiver::RemoveActor(Worker_EntityId EntityId)
{
	SCOPE_CYCLE_COUNTER(STAT_ReceiverRemoveActor);

	TWeakObjectPtr<UObject> WeakActor = PackageMap->GetObjectFromEntityId(EntityId);

	// Actor has not been resolved yet or has already been destroyed. Clean up surrounding bookkeeping.
	if (!WeakActor.IsValid())
	{
		DestroyActor(nullptr, EntityId);
		return;
	}

	AActor* Actor = Cast<AActor>(WeakActor.Get());

	UE_LOG(LogSpatialReceiver, Verbose, TEXT("Worker %s Remove Actor: %s %lld"), *NetDriver->Connection->GetWorkerId(),
		   Actor && !Actor->IsPendingKill() ? *Actor->GetName() : TEXT("nullptr"), EntityId);

	// Cleanup pending add components if any exist.
	if (USpatialActorChannel* ActorChannel = NetDriver->GetActorChannelByEntityId(EntityId))
	{
		// If we have any pending subobjects on the channel
		if (ActorChannel->PendingDynamicSubobjects.Num() > 0)
		{
			// Then iterate through all pending subobjects and remove entries relating to this entity.
			for (const auto& Pair : PendingDynamicSubobjectComponents)
			{
				if (Pair.Key.Key == EntityId)
				{
					PendingDynamicSubobjectComponents.Remove(Pair.Key);
				}
			}
		}
	}

	// Actor already deleted (this worker was most likely authoritative over it and deleted it earlier).
	if (Actor == nullptr || Actor->IsPendingKill())
	{
		if (USpatialActorChannel* ActorChannel = NetDriver->GetActorChannelByEntityId(EntityId))
		{
			UE_LOG(LogSpatialReceiver, Warning,
				   TEXT("RemoveActor: actor for entity %lld was already deleted (likely on the authoritative worker) but still has an open "
						"actor channel."),
				   EntityId);
			ActorChannel->ConditionalCleanUp(false, EChannelCloseReason::Destroyed);
		}
		return;
	}

	if (USpatialActorChannel* ActorChannel = NetDriver->GetActorChannelByEntityId(EntityId))
	{
		if (NetDriver->GetWorld() != nullptr && !NetDriver->GetWorld()->IsPendingKillOrUnreachable())
		{
			for (UObject* SubObject : ActorChannel->CreateSubObjects)
			{
				if (SubObject)
				{
					FUnrealObjectRef ObjectRef = FUnrealObjectRef::FromObjectPtr(SubObject, Cast<USpatialPackageMapClient>(PackageMap));
					// Unmap this object so we can remap it if it becomes relevant again in the future
					MoveMappedObjectToUnmapped(ObjectRef);
				}
			}

			FUnrealObjectRef ObjectRef = FUnrealObjectRef::FromObjectPtr(Actor, Cast<USpatialPackageMapClient>(PackageMap));
			// Unmap this object so we can remap it if it becomes relevant again in the future
			MoveMappedObjectToUnmapped(ObjectRef);
		}

		for (auto& ChannelRefs : ActorChannel->ObjectReferenceMap)
		{
			CleanupRepStateMap(ChannelRefs.Value);
		}

		ActorChannel->ObjectReferenceMap.Empty();

		// If the entity is to be deleted after having been torn off, ignore the request (but clean up the channel if it has not been
		// cleaned up already).
		if (Actor->GetTearOff())
		{
			ActorChannel->ConditionalCleanUp(false, EChannelCloseReason::TearOff);
			return;
		}
	}

	// Actor is a startup actor that is a part of the level.  If it's not Tombstone'd, then it
	// has just fallen out of our view and we should only remove the entity.
	if (Actor->IsFullNameStableForNetworking()
		&& StaticComponentView->HasComponent(EntityId, SpatialConstants::TOMBSTONE_COMPONENT_ID) == false)
	{
		PackageMap->ClearRemovedDynamicSubobjectObjectRefs(EntityId);
		if (USpatialActorChannel* Channel = NetDriver->GetActorChannelByEntityId(EntityId))
		{
			for (UObject* DynamicSubobject : Channel->CreateSubObjects)
			{
				FNetworkGUID SubobjectNetGUID = PackageMap->GetNetGUIDFromObject(DynamicSubobject);
				if (SubobjectNetGUID.IsValid())
				{
					FUnrealObjectRef SubobjectRef = PackageMap->GetUnrealObjectRefFromNetGUID(SubobjectNetGUID);

					if (SubobjectRef.IsValid() && IsDynamicSubObject(Actor, SubobjectRef.Offset))
					{
						PackageMap->AddRemovedDynamicSubobjectObjectRef(SubobjectRef, SubobjectNetGUID);
					}
				}
			}
		}
		// We can't call CleanupDeletedEntity here as we need the NetDriver to maintain the EntityId
		// to Actor Channel mapping for the DestroyActor to function correctly
		PackageMap->RemoveEntityActor(EntityId);
		return;
	}

	if (APlayerController* PC = Cast<APlayerController>(Actor))
	{
		// Force APlayerController::DestroyNetworkActorHandled to return false
		PC->Player = nullptr;
	}

	// Workaround for camera loss on handover: prevent UnPossess() (non-authoritative destruction of pawn, while being authoritative over
	// the controller)
	// TODO: Check how AI controllers are affected by this (UNR-430)
	// TODO: This should be solved properly by working sets (UNR-411)
	if (APawn* Pawn = Cast<APawn>(Actor))
	{
		AController* Controller = Pawn->Controller;

		if (Controller && Controller->HasAuthority())
		{
			Pawn->Controller = nullptr;
		}
	}

	DestroyActor(Actor, EntityId);
}

void USpatialReceiver::DestroyActor(AActor* Actor, Worker_EntityId EntityId)
{
	// Destruction of actors can cause the destruction of associated actors (eg. Character > Controller). Actor destroy
	// calls will eventually find their way into USpatialActorChannel::DeleteEntityIfAuthoritative() which checks if the entity
	// is currently owned by this worker before issuing an entity delete request. If the associated entity is still authoritative
	// on this server, we need to make sure this worker doesn't issue an entity delete request, as this entity is really
	// transitioning to the same server as the actor we're currently operating on, and is just a few frames behind.
	// We make the assumption that if we're destroying actors here (due to a remove entity op), then this is only due to two
	// situations;
	// 1. Actor's entity has been transitioned to another server
	// 2. The Actor was deleted on another server
	// In neither situation do we want to delete associated entities, so prevent them from being issued.
	// TODO: fix this with working sets (UNR-411)
	NetDriver->StartIgnoringAuthoritativeDestruction();

	// Clean up the actor channel. For clients, this will also call destroy on the actor.
	if (USpatialActorChannel* ActorChannel = NetDriver->GetActorChannelByEntityId(EntityId))
	{
		ActorChannel->ConditionalCleanUp(false, EChannelCloseReason::Destroyed);
	}
	else
	{
		if (NetDriver->IsDormantEntity(EntityId))
		{
			PackageMap->RemoveEntityActor(EntityId);
		}
		else
		{
			UE_LOG(LogSpatialReceiver, Verbose,
				   TEXT("Removing actor as a result of a remove entity op, which has a missing actor channel. Actor: %s EntityId: %lld"),
				   *GetNameSafe(Actor), EntityId);
		}
	}

	// It is safe to call AActor::Destroy even if the destruction has already started.
	if (Actor != nullptr && !Actor->Destroy(true))
	{
		UE_LOG(LogSpatialReceiver, Error, TEXT("Failed to destroy actor in RemoveActor %s %lld"), *Actor->GetName(), EntityId);
	}
	NetDriver->StopIgnoringAuthoritativeDestruction();

	check(PackageMap->GetObjectFromEntityId(EntityId) == nullptr);
}

AActor* USpatialReceiver::TryGetOrCreateActor(UnrealMetadata* UnrealMetadataComp, SpawnData* SpawnDataComp,
											  NetOwningClientWorker* NetOwningClientWorkerComp)
{
	if (UnrealMetadataComp->StablyNamedRef.IsSet())
	{
		if (NetDriver->IsServer() || UnrealMetadataComp->bNetStartup.GetValue())
		{
			// This Actor already exists in the map, get it from the package map.
			const FUnrealObjectRef& StablyNamedRef = UnrealMetadataComp->StablyNamedRef.GetValue();
			AActor* StaticActor = Cast<AActor>(PackageMap->GetObjectFromUnrealObjectRef(StablyNamedRef));
			// An unintended side effect of GetObjectFromUnrealObjectRef is that this ref
			// will be registered with this Actor. It can be the case that this Actor is not
			// stably named (due to bNetLoadOnClient = false) so we should let
			// SpatialPackageMapClient::ResolveEntityActor handle it properly.
			PackageMap->UnregisterActorObjectRefOnly(StablyNamedRef);

			return StaticActor;
		}
	}

	// Handle linking received unique Actors (e.g. game state, game mode) to instances already spawned on this worker.
	UClass* ActorClass = UnrealMetadataComp->GetNativeEntityClass();
	if (FUnrealObjectRef::IsUniqueActorClass(ActorClass) && NetDriver->IsServer())
	{
		return PackageMap->GetUniqueActorInstanceByClass(ActorClass);
	}

	return CreateActor(UnrealMetadataComp, SpawnDataComp, NetOwningClientWorkerComp);
}

// This function is only called for client and server workers who did not spawn the Actor
AActor* USpatialReceiver::CreateActor(UnrealMetadata* UnrealMetadataComp, SpawnData* SpawnDataComp,
									  NetOwningClientWorker* NetOwningClientWorkerComp)
{
	UClass* ActorClass = UnrealMetadataComp->GetNativeEntityClass();

	if (ActorClass == nullptr)
	{
		UE_LOG(LogSpatialReceiver, Error, TEXT("Could not load class %s when spawning entity!"), *UnrealMetadataComp->ClassPath);
		return nullptr;
	}

	UE_LOG(LogSpatialReceiver, Verbose, TEXT("Spawning a %s whilst checking out an entity."), *ActorClass->GetFullName());

	const bool bCreatingPlayerController = ActorClass->IsChildOf(APlayerController::StaticClass());

	FActorSpawnParameters SpawnInfo;
	SpawnInfo.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	SpawnInfo.bRemoteOwned = true;
	SpawnInfo.bNoFail = true;

	FVector SpawnLocation = FRepMovement::RebaseOntoLocalOrigin(SpawnDataComp->Location, NetDriver->GetWorld()->OriginLocation);

	AActor* NewActor = NetDriver->GetWorld()->SpawnActorAbsolute(ActorClass, FTransform(SpawnDataComp->Rotation, SpawnLocation), SpawnInfo);
	check(NewActor);

	if (NetDriver->IsServer() && bCreatingPlayerController)
	{
		// If we're spawning a PlayerController, it should definitely have a net-owning client worker ID.
		check(NetOwningClientWorkerComp->ClientPartitionId.IsSet());
		NetDriver->PostSpawnPlayerController(Cast<APlayerController>(NewActor));
	}

	// Imitate the behavior in UPackageMapClient::SerializeNewActor.
	const float Epsilon = 0.001f;
	if (!SpawnDataComp->Velocity.Equals(FVector::ZeroVector, Epsilon))
	{
		NewActor->PostNetReceiveVelocity(SpawnDataComp->Velocity);
	}
	if (!SpawnDataComp->Scale.Equals(FVector::OneVector, Epsilon))
	{
		NewActor->SetActorScale3D(SpawnDataComp->Scale);
	}

	// Don't have authority over Actor until SpatialOS delegates authority
	NewActor->Role = ROLE_SimulatedProxy;
	NewActor->RemoteRole = ROLE_Authority;

	return NewActor;
}

FTransform USpatialReceiver::GetRelativeSpawnTransform(UClass* ActorClass, FTransform SpawnTransform)
{
	FTransform NewTransform = SpawnTransform;
	if (AActor* Template = ActorClass->GetDefaultObject<AActor>())
	{
		if (USceneComponent* TemplateRootComponent = Template->GetRootComponent())
		{
			TemplateRootComponent->UpdateComponentToWorld();
			NewTransform = TemplateRootComponent->GetComponentToWorld().Inverse() * NewTransform;
		}
	}

	return NewTransform;
}

void USpatialReceiver::ApplyComponentDataOnActorCreation(Worker_EntityId EntityId, const Worker_ComponentData& Data,
														 USpatialActorChannel& Channel, const FClassInfo& ActorClassInfo,
														 TArray<ObjectPtrRefPair>& OutObjectsToResolve)
{
	AActor* Actor = Channel.GetActor();

	uint32 Offset = 0;
	bool bFoundOffset = ClassInfoManager->GetOffsetByComponentId(Data.component_id, Offset);
	if (!bFoundOffset)
	{
		UE_LOG(LogSpatialReceiver, Warning,
			   TEXT("Worker: %s EntityId: %lld, ComponentId: %d - Could not find offset for component id when applying component data to "
					"Actor %s!"),
			   *NetDriver->Connection->GetWorkerId(), EntityId, Data.component_id, *Actor->GetPathName());
		return;
	}

	FUnrealObjectRef TargetObjectRef(EntityId, Offset);
	TWeakObjectPtr<UObject> TargetObject = PackageMap->GetObjectFromUnrealObjectRef(TargetObjectRef);
	if (!TargetObject.IsValid())
	{
		if (!IsDynamicSubObject(Actor, Offset))
		{
			UE_LOG(LogSpatialReceiver, Verbose,
				   TEXT("Tried to apply component data on actor creation for a static subobject that's been deleted, will skip. Entity: "
						"%lld, Component: %d, Actor: %s"),
				   EntityId, Data.component_id, *Actor->GetPathName());
			return;
		}

		// If we can't find this subobject, it's a dynamically attached object. Check if we created previously.
		if (FNetworkGUID* SubobjectNetGUID = PackageMap->GetRemovedDynamicSubobjectNetGUID(TargetObjectRef))
		{
			if (UObject* DynamicSubobject = PackageMap->GetObjectFromNetGUID(*SubobjectNetGUID, false))
			{
				PackageMap->ResolveSubobject(DynamicSubobject, TargetObjectRef);
				ApplyComponentData(Channel, *DynamicSubobject, Data);

				OutObjectsToResolve.Add(ObjectPtrRefPair(DynamicSubobject, TargetObjectRef));
				return;
			}
		}

		// If the dynamically attached object was not created before. Create it now.
		TargetObject = NewObject<UObject>(Actor, ClassInfoManager->GetClassByComponentId(Data.component_id));

		Actor->OnSubobjectCreatedFromReplication(TargetObject.Get());

		PackageMap->ResolveSubobject(TargetObject.Get(), TargetObjectRef);

		Channel.CreateSubObjects.Add(TargetObject.Get());
	}

	FString TargetObjectPath = TargetObject->GetPathName();
	ApplyComponentData(Channel, *TargetObject, Data);

	if (TargetObject.IsValid())
	{
		OutObjectsToResolve.Add(ObjectPtrRefPair(TargetObject.Get(), TargetObjectRef));
	}
	else
	{
		// TODO: remove / downgrade this to a log after verifying we handle this properly - UNR-4379
		UE_LOG(LogSpatialReceiver, Warning, TEXT("Actor subobject got invalidated after applying component data! Subobject: %s"),
			   *TargetObjectPath);
	}
}

void USpatialReceiver::HandleIndividualAddComponent(Worker_EntityId EntityId, Worker_ComponentId ComponentId,
													TUniquePtr<SpatialGDK::DynamicComponent> Data)
{
	uint32 Offset = 0;
	bool bFoundOffset = ClassInfoManager->GetOffsetByComponentId(ComponentId, Offset);
	if (!bFoundOffset)
	{
		UE_LOG(LogSpatialReceiver, Warning,
			   TEXT("Could not find offset for component id when receiving dynamic AddComponent."
					" (EntityId %lld, ComponentId %d)"),
			   EntityId, ComponentId);
		return;
	}

	// Object already exists, we can apply data directly.
	if (UObject* Object = PackageMap->GetObjectFromUnrealObjectRef(FUnrealObjectRef(EntityId, Offset)).Get())
	{
		if (USpatialActorChannel* Channel = NetDriver->GetActorChannelByEntityId(EntityId))
		{
			ApplyComponentData(*Channel, *Object, Data->Data.GetWorkerComponentData());
		}
		return;
	}

	const FClassInfo& Info = ClassInfoManager->GetClassInfoByComponentId(ComponentId);
	AActor* Actor = Cast<AActor>(PackageMap->GetObjectFromEntityId(EntityId).Get());
	if (Actor == nullptr)
	{
		UE_LOG(LogSpatialReceiver, Warning,
			   TEXT("Received an add component op for subobject of type %s on entity %lld but couldn't find Actor!"),
			   *Info.Class->GetName(), EntityId);
		return;
	}

	// Check if this is a static subobject that's been destroyed by the receiver.
	if (!IsDynamicSubObject(Actor, Offset))
	{
		UE_LOG(LogSpatialReceiver, Verbose,
			   TEXT("Tried to apply component data on add component for a static subobject that's been deleted, will skip. Entity: %lld, "
					"Component: %d, Actor: %s"),
			   EntityId, ComponentId, *Actor->GetPathName());
		return;
	}

	// Otherwise this is a dynamically attached component. We need to make sure we have all related components before creation.
	PendingDynamicSubobjectComponents.Add(MakeTuple(static_cast<Worker_EntityId_Key>(EntityId), ComponentId),
										  PendingAddComponentWrapper(EntityId, ComponentId, MoveTemp(Data)));

	bool bReadyToCreate = true;
	ForAllSchemaComponentTypes([&](ESchemaComponentType Type) {
		Worker_ComponentId SchemaComponentId = Info.SchemaComponents[Type];

		if (SchemaComponentId == SpatialConstants::INVALID_COMPONENT_ID)
		{
			return;
		}

		if (!PendingDynamicSubobjectComponents.Contains(MakeTuple(static_cast<Worker_EntityId_Key>(EntityId), SchemaComponentId)))
		{
			bReadyToCreate = false;
		}
	});

	if (bReadyToCreate)
	{
		AttachDynamicSubobject(Actor, EntityId, Info);
	}
}

void USpatialReceiver::AttachDynamicSubobject(AActor* Actor, Worker_EntityId EntityId, const FClassInfo& Info)
{
	USpatialActorChannel* Channel = NetDriver->GetActorChannelByEntityId(EntityId);
	if (Channel == nullptr)
	{
		UE_LOG(LogSpatialReceiver, Verbose,
			   TEXT("Tried to dynamically attach subobject of type %s to entity %lld but couldn't find Channel!"), *Info.Class->GetName(),
			   EntityId);
		return;
	}

	UObject* Subobject = NewObject<UObject>(Actor, Info.Class.Get());

	Actor->OnSubobjectCreatedFromReplication(Subobject);

	FUnrealObjectRef SubobjectRef(EntityId, Info.SchemaComponents[SCHEMA_Data]);
	PackageMap->ResolveSubobject(Subobject, SubobjectRef);

	Channel->CreateSubObjects.Add(Subobject);

	ForAllSchemaComponentTypes([&](ESchemaComponentType Type) {
		Worker_ComponentId ComponentId = Info.SchemaComponents[Type];

		if (ComponentId == SpatialConstants::INVALID_COMPONENT_ID)
		{
			return;
		}

		TPair<Worker_EntityId_Key, Worker_ComponentId> EntityComponentPair =
			MakeTuple(static_cast<Worker_EntityId_Key>(EntityId), ComponentId);

		PendingAddComponentWrapper& AddComponent = PendingDynamicSubobjectComponents[EntityComponentPair];
		ApplyComponentData(*Channel, *Subobject, AddComponent.Data->Data.GetWorkerComponentData());
		PendingDynamicSubobjectComponents.Remove(EntityComponentPair);
	});

	// Resolve things like RepNotify or RPCs after applying component data.
	ResolvePendingOperations(Subobject, SubobjectRef);
}

struct USpatialReceiver::RepStateUpdateHelper
{
	RepStateUpdateHelper(USpatialActorChannel& Channel, UObject& TargetObject)
		: ObjectPtr(MakeWeakObjectPtr(&TargetObject))
		, ObjectRepState(Channel.ObjectReferenceMap.Find(ObjectPtr))
	{
	}

	~RepStateUpdateHelper() { check(bUpdatePerfomed); }

	FObjectReferencesMap& GetRefMap()
	{
		if (ObjectRepState)
		{
			return ObjectRepState->ReferenceMap;
		}
		return TempRefMap;
	}

	void Update(USpatialReceiver& Receiver, USpatialActorChannel& Channel, UObject& TargetObject, bool bReferencesChanged)
	{
		check(!bUpdatePerfomed);

		if (bReferencesChanged)
		{
			if (ObjectRepState == nullptr && TempRefMap.Num() > 0)
			{
				ObjectRepState =
					&Channel.ObjectReferenceMap.Add(ObjectPtr, FSpatialObjectRepState(FChannelObjectPair(&Channel, ObjectPtr)));
				ObjectRepState->ReferenceMap = MoveTemp(TempRefMap);
			}

			if (ObjectRepState)
			{
				ObjectRepState->UpdateRefToRepStateMap(Receiver.ObjectRefToRepStateMap);

				if (ObjectRepState->ReferencedObj.Num() == 0)
				{
					Channel.ObjectReferenceMap.Remove(ObjectPtr);
				}
			}
		}
#if DO_CHECK
		bUpdatePerfomed = true;
#endif
	}

	// TSet<FUnrealObjectRef> UnresolvedRefs;
private:
	FObjectReferencesMap TempRefMap;
	TWeakObjectPtr<UObject> ObjectPtr;
	FSpatialObjectRepState* ObjectRepState;
#if DO_CHECK
	bool bUpdatePerfomed = false;
#endif
};

void USpatialReceiver::ApplyComponentData(USpatialActorChannel& Channel, UObject& TargetObject, const Worker_ComponentData& Data)
{
	UClass* Class = ClassInfoManager->GetClassByComponentId(Data.component_id);
	checkf(Class, TEXT("Component %d isn't hand-written and not present in ComponentToClassMap."), Data.component_id);

	ESchemaComponentType ComponentType = ClassInfoManager->GetCategoryByComponentId(Data.component_id);

	if (ComponentType == SCHEMA_Data || ComponentType == SCHEMA_OwnerOnly)
	{
		if (ComponentType == SCHEMA_Data && TargetObject.IsA<UActorComponent>())
		{
			Schema_Object* ComponentObject = Schema_GetComponentDataFields(Data.schema_type);
			bool bReplicates = !!Schema_IndexBool(ComponentObject, SpatialConstants::ACTOR_COMPONENT_REPLICATES_ID, 0);
			if (!bReplicates)
			{
				return;
			}
		}
		RepStateUpdateHelper RepStateHelper(Channel, TargetObject);

		ComponentReader Reader(NetDriver, RepStateHelper.GetRefMap(), EventTracer);
		bool bOutReferencesChanged = false;
		Reader.ApplyComponentData(Data, TargetObject, Channel, /* bIsHandover */ false, bOutReferencesChanged);

		RepStateHelper.Update(*this, Channel, TargetObject, bOutReferencesChanged);
	}
	else if (ComponentType == SCHEMA_Handover)
	{
		RepStateUpdateHelper RepStateHelper(Channel, TargetObject);

		ComponentReader Reader(NetDriver, RepStateHelper.GetRefMap(), EventTracer);
		bool bOutReferencesChanged = false;
		Reader.ApplyComponentData(Data, TargetObject, Channel, /* bIsHandover */ true, bOutReferencesChanged);

		RepStateHelper.Update(*this, Channel, TargetObject, bOutReferencesChanged);
	}
	else
	{
		UE_LOG(LogSpatialReceiver, Verbose, TEXT("Entity: %d Component: %d - Skipping because RPC components don't have actual data."),
			   Channel.GetEntityId(), Data.component_id);
	}
}

void USpatialReceiver::OnComponentUpdate(const Worker_ComponentUpdateOp& Op)
{
	SCOPE_CYCLE_COUNTER(STAT_ReceiverComponentUpdate);
	if (IsEntityWaitingForAsyncLoad(Op.entity_id))
	{
		QueueComponentUpdateOpForAsyncLoad(Op);
		return;
	}

	switch (Op.update.component_id)
	{
	case SpatialConstants::METADATA_COMPONENT_ID:
	case SpatialConstants::POSITION_COMPONENT_ID:
	case SpatialConstants::PERSISTENCE_COMPONENT_ID:
	case SpatialConstants::INTEREST_COMPONENT_ID:
	case SpatialConstants::AUTHORITY_DELEGATION_COMPONENT_ID:
	case SpatialConstants::SPAWN_DATA_COMPONENT_ID:
	case SpatialConstants::PLAYER_SPAWNER_COMPONENT_ID:
	case SpatialConstants::UNREAL_METADATA_COMPONENT_ID:
	case SpatialConstants::NOT_STREAMED_COMPONENT_ID:
	case SpatialConstants::RPCS_ON_ENTITY_CREATION_ID:
	case SpatialConstants::DEBUG_METRICS_COMPONENT_ID:
	case SpatialConstants::ALWAYS_RELEVANT_COMPONENT_ID:
	case SpatialConstants::VISIBLE_COMPONENT_ID:
	case SpatialConstants::SPATIAL_DEBUGGING_COMPONENT_ID:
	case SpatialConstants::CLIENT_RPC_ENDPOINT_COMPONENT_ID_LEGACY:
	case SpatialConstants::SERVER_RPC_ENDPOINT_COMPONENT_ID_LEGACY:
	case SpatialConstants::NETMULTICAST_RPCS_COMPONENT_ID_LEGACY:
	case SpatialConstants::CLIENT_ENDPOINT_COMPONENT_ID:
	case SpatialConstants::SERVER_ENDPOINT_COMPONENT_ID:
	case SpatialConstants::MULTICAST_RPCS_COMPONENT_ID:
	case SpatialConstants::GSM_SHUTDOWN_COMPONENT_ID:
	case SpatialConstants::AUTHORITY_INTENT_COMPONENT_ID:
	case SpatialConstants::COMPONENT_PRESENCE_COMPONENT_ID:
	case SpatialConstants::NET_OWNING_CLIENT_WORKER_COMPONENT_ID:
	case SpatialConstants::VIRTUAL_WORKER_TRANSLATION_COMPONENT_ID:
	case SpatialConstants::DEPLOYMENT_MAP_COMPONENT_ID:
	case SpatialConstants::STARTUP_ACTOR_MANAGER_COMPONENT_ID:
		UE_LOG(LogSpatialReceiver, Verbose, TEXT("Entity: %d Component: %d - Skipping because this is hand-written Spatial component"),
			   Op.entity_id, Op.update.component_id);
		return;
	case SpatialConstants::HEARTBEAT_COMPONENT_ID:
		OnHeartbeatComponentUpdate(Op);
		return;
	case SpatialConstants::GDK_DEBUG_COMPONENT_ID:
		if (NetDriver->DebugCtx != nullptr)
		{
			NetDriver->DebugCtx->OnDebugComponentUpdateReceived(Op.entity_id);
		}
		return;
	}

	if (Op.update.component_id < SpatialConstants::MAX_RESERVED_SPATIAL_SYSTEM_COMPONENT_ID)
	{
		UE_LOG(LogSpatialReceiver, Verbose, TEXT("Entity: %d Component: %d - Skipping because this is a reserved spatial system component"),
			   Op.entity_id, Op.update.component_id);
		return;
	}

	// If this entity has a Tombstone component, abort all component processing
	if (const Tombstone* TombstoneComponent = StaticComponentView->GetComponentData<Tombstone>(Op.entity_id))
	{
		UE_LOG(LogSpatialReceiver, Warning,
			   TEXT("Received component update for Entity: %lld Component: %d after tombstone marked dead.  Aborting update."),
			   Op.entity_id, Op.update.component_id);
		return;
	}

	if (ClassInfoManager->IsGeneratedQBIMarkerComponent(Op.update.component_id))
	{
		return;
	}

	USpatialActorChannel* Channel = NetDriver->GetActorChannelByEntityId(Op.entity_id);
	if (Channel == nullptr)
	{
		// If there is no actor channel as a result of the actor being dormant, then assume the actor is about to become active.
		if (StaticComponentView->HasComponent(Op.entity_id, SpatialConstants::DORMANT_COMPONENT_ID))
		{
			if (AActor* Actor = Cast<AActor>(PackageMap->GetObjectFromEntityId(Op.entity_id)))
			{
				Channel = GetOrRecreateChannelForDomantActor(Actor, Op.entity_id);

				// As we haven't removed the dormant component just yet, this might be a single replication update where the actor
				// remains dormant. Add it back to pending dormancy so the local worker can clean up the channel. If we do process
				// a dormant component removal later in this frame, we'll clear the channel from pending dormancy channel then.
				NetDriver->AddPendingDormantChannel(Channel);
			}
			else
			{
				UE_LOG(LogSpatialReceiver, Warning,
					   TEXT("Worker: %s Dormant actor (entity: %lld) has been deleted on this worker but we have received a component "
							"update (id: %d) from the server."),
					   *NetDriver->Connection->GetWorkerId(), Op.entity_id, Op.update.component_id);
				return;
			}
		}
		else
		{
			UE_LOG(LogSpatialReceiver, Log,
				   TEXT("Worker: %s Entity: %lld Component: %d - No actor channel for update. This most likely occured due to the "
						"component updates that are sent when authority is lost during entity deletion."),
				   *NetDriver->Connection->GetWorkerId(), Op.entity_id, Op.update.component_id);
			return;
		}
	}

	uint32 Offset;
	bool bFoundOffset = ClassInfoManager->GetOffsetByComponentId(Op.update.component_id, Offset);
	if (!bFoundOffset)
	{
		UE_LOG(LogSpatialReceiver, Warning,
			   TEXT("Worker: %s EntityId %d ComponentId %d - Could not find offset for component id when receiving a component update."),
			   *NetDriver->Connection->GetWorkerId(), Op.entity_id, Op.update.component_id);
		return;
	}

	UObject* TargetObject = nullptr;

	if (Offset == 0)
	{
		TargetObject = Channel->GetActor();
	}
	else
	{
		TargetObject = PackageMap->GetObjectFromUnrealObjectRef(FUnrealObjectRef(Op.entity_id, Offset)).Get();
	}

	if (TargetObject == nullptr)
	{
		UE_LOG(LogSpatialReceiver, Warning, TEXT("Entity: %d Component: %d - Couldn't find target object for update"), Op.entity_id,
			   Op.update.component_id);
		return;
	}

	TOptional<Trace_SpanId> CauseSpanId = EventTracer->GetSpanId(EntityComponentId(Op.entity_id, Op.update.component_id));
	if (CauseSpanId.IsSet())
	{
		TOptional<Trace_SpanId> SpanId = EventTracer->CreateSpan(&CauseSpanId.GetValue(), 1);
		EventTracer->TraceEvent(
			FSpatialTraceEventBuilder::CreateComponentUpdate(Channel->Actor, TargetObject, Op.entity_id, Op.update.component_id), SpanId);
	}

	ESchemaComponentType Category = ClassInfoManager->GetCategoryByComponentId(Op.update.component_id);

	if (Category == ESchemaComponentType::SCHEMA_Data || Category == ESchemaComponentType::SCHEMA_OwnerOnly)
	{
		SCOPE_CYCLE_COUNTER(STAT_ReceiverApplyData);
		ApplyComponentUpdate(Op.update, *TargetObject, *Channel, /* bIsHandover */ false);
	}
	else if (Category == ESchemaComponentType::SCHEMA_Handover)
	{
		SCOPE_CYCLE_COUNTER(STAT_ReceiverApplyHandover);
		if (!NetDriver->IsServer())
		{
			UE_LOG(LogSpatialReceiver, Verbose, TEXT("Entity: %d Component: %d - Skipping Handover component because we're a client."),
				   Op.entity_id, Op.update.component_id);
			return;
		}

		ApplyComponentUpdate(Op.update, *TargetObject, *Channel, /* bIsHandover */ true);
	}
	else
	{
		UE_LOG(LogSpatialReceiver, Verbose,
			   TEXT("Entity: %d Component: %d - Skipping because it's an empty component update from an RPC component. (most likely as a "
					"result of gaining authority)"),
			   Op.entity_id, Op.update.component_id);
	}
}

void USpatialReceiver::OnCommandRequest(const Worker_Op& Op)
{
	SCOPE_CYCLE_COUNTER(STAT_ReceiverCommandRequest);

	const Worker_CommandRequestOp& CommandRequestOp = Op.op.command_request;
	const Worker_CommandRequest& Request = CommandRequestOp.request;
	const Worker_EntityId EntityId = CommandRequestOp.entity_id;
	const Worker_ComponentId ComponentId = Request.component_id;
	const Worker_RequestId RequestId = CommandRequestOp.request_id;
	const Schema_FieldId CommandIndex = Request.command_index;

	if (IsEntityWaitingForAsyncLoad(CommandRequestOp.entity_id))
	{
		UE_LOG(LogSpatialReceiver, Warning,
			   TEXT("USpatialReceiver::OnCommandRequest: Actor class async loading, cannot handle command. Entity %lld, Class %s"),
			   EntityId, *EntitiesWaitingForAsyncLoad[EntityId].ClassPath);
		Sender->SendCommandFailure(RequestId, TEXT("Target actor async loading."), Op.span_id);
		return;
	}

	if (ComponentId == SpatialConstants::PLAYER_SPAWNER_COMPONENT_ID
		&& CommandIndex == SpatialConstants::PLAYER_SPAWNER_SPAWN_PLAYER_COMMAND_ID)
	{
		NetDriver->PlayerSpawner->ReceivePlayerSpawnRequestOnServer(CommandRequestOp);

		TOptional<Trace_SpanId> SpanId = EventTracer->CreateSpan(&Op.span_id, 1);
		EventTracer->TraceEvent(FSpatialTraceEventBuilder::CreateReceiveCommandRequest(TEXT("SPAWN_PLAYER_COMMAND"), RequestId), SpanId);
		return;
	}
	else if (ComponentId == SpatialConstants::SERVER_WORKER_COMPONENT_ID
			 && CommandIndex == SpatialConstants::SERVER_WORKER_FORWARD_SPAWN_REQUEST_COMMAND_ID)
	{
		NetDriver->PlayerSpawner->ReceiveForwardedPlayerSpawnRequest(CommandRequestOp);

		TOptional<Trace_SpanId> SpanId = EventTracer->CreateSpan(&Op.span_id, 1);
		EventTracer->TraceEvent(
			FSpatialTraceEventBuilder::CreateReceiveCommandRequest(TEXT("SERVER_WORKER_FORWARD_SPAWN_REQUEST_COMMAND"), RequestId), SpanId);
		return;
	}
	else if (ComponentId == SpatialConstants::RPCS_ON_ENTITY_CREATION_ID && CommandIndex == SpatialConstants::CLEAR_RPCS_ON_ENTITY_CREATION)
	{
		Sender->ClearRPCsOnEntityCreation(EntityId);
		Sender->SendEmptyCommandResponse(ComponentId, CommandIndex, RequestId, Op.span_id);

		TOptional<Trace_SpanId> SpanId = EventTracer->CreateSpan(&Op.span_id, 1);
		EventTracer->TraceEvent(FSpatialTraceEventBuilder::CreateReceiveCommandRequest(TEXT("CLEAR_RPCS_ON_ENTITY_CREATION"), RequestId),
								SpanId);
		return;
	}
#if WITH_EDITOR
	else if (ComponentId == SpatialConstants::GSM_SHUTDOWN_COMPONENT_ID
			 && CommandIndex == SpatialConstants::SHUTDOWN_MULTI_PROCESS_REQUEST_ID)
	{
		NetDriver->GlobalStateManager->ReceiveShutdownMultiProcessRequest();

		TOptional<Trace_SpanId> SpanId = EventTracer->CreateSpan(&Op.span_id, 1);
		EventTracer->TraceEvent(FSpatialTraceEventBuilder::CreateReceiveCommandRequest(TEXT("SHUTDOWN_MULTI_PROCESS_REQUEST"), RequestId),
								SpanId);
		return;
	}
#endif // WITH_EDITOR
#if !UE_BUILD_SHIPPING
	else if (ComponentId == SpatialConstants::DEBUG_METRICS_COMPONENT_ID)
	{
		switch (CommandIndex)
		{
		case SpatialConstants::DEBUG_METRICS_START_RPC_METRICS_ID:
			NetDriver->SpatialMetrics->OnStartRPCMetricsCommand();
			break;
		case SpatialConstants::DEBUG_METRICS_STOP_RPC_METRICS_ID:
			NetDriver->SpatialMetrics->OnStopRPCMetricsCommand();
			break;
		case SpatialConstants::DEBUG_METRICS_MODIFY_SETTINGS_ID:
		{
			Schema_Object* Payload = Schema_GetCommandRequestObject(CommandRequestOp.request.schema_type);
			NetDriver->SpatialMetrics->OnModifySettingCommand(Payload);
			break;
		}
		default:
			UE_LOG(LogSpatialReceiver, Error, TEXT("Unknown command index for DebugMetrics component: %d, entity: %lld"), CommandIndex,
				   EntityId);
			break;
		}

		Sender->SendEmptyCommandResponse(ComponentId, CommandIndex, RequestId, Op.span_id);
		return;
	}
#endif // !UE_BUILD_SHIPPING

	Schema_Object* RequestObject = Schema_GetCommandRequestObject(Request.schema_type);

	RPCPayload Payload(RequestObject);
	const FUnrealObjectRef ObjectRef = FUnrealObjectRef(EntityId, Payload.Offset);
	UObject* TargetObject = PackageMap->GetObjectFromUnrealObjectRef(ObjectRef).Get();
	if (TargetObject == nullptr)
	{
		UE_LOG(LogSpatialReceiver, Warning, TEXT("No target object found for EntityId %d"), EntityId);
		Sender->SendEmptyCommandResponse(ComponentId, CommandIndex, RequestId, Op.span_id);
		return;
	}

	const FClassInfo& Info = ClassInfoManager->GetOrCreateClassInfoByObject(TargetObject);
	UFunction* Function = Info.RPCs[Payload.Index];

	UE_LOG(LogSpatialReceiver, Verbose, TEXT("Received command request (entity: %lld, component: %d, function: %s)"), EntityId, ComponentId,
		   *Function->GetName());

	RPCService->ProcessOrQueueIncomingRPC(ObjectRef, MoveTemp(Payload), /* RPCIdForLinearEventTrace */ TOptional<uint64>{});
	Sender->SendEmptyCommandResponse(ComponentId, CommandIndex, RequestId, Op.span_id);

	AActor* TargetActor = Cast<AActor>(PackageMap->GetObjectFromEntityId(EntityId));
#if TRACE_LIB_ACTIVE
	TraceKey TraceId = Payload.Trace;
#else
	TraceKey TraceId = InvalidTraceKey;
#endif

	UObject* TraceTargetObject = TargetActor != TargetObject ? TargetObject : nullptr;
	TOptional<Trace_SpanId> SpanId = EventTracer->CreateSpan(&Op.span_id, 1);
	EventTracer->TraceEvent(FSpatialTraceEventBuilder::CreateReceiveCommandRequest("RPC_COMMAND_REQUEST", TargetActor, TraceTargetObject,
																				   Function, TraceId, RequestId),
							SpanId);
}

void USpatialReceiver::OnCommandResponse(const Worker_Op& Op)
{
	const Worker_CommandResponseOp& CommandResponseOp = Op.op.command_response;
	const Worker_CommandResponse& Repsonse = CommandResponseOp.response;
	const Worker_ComponentId ComponentId = Repsonse.component_id;
	const Worker_RequestId RequestId = CommandResponseOp.request_id;

	SCOPE_CYCLE_COUNTER(STAT_ReceiverCommandResponse);
	if (ComponentId == SpatialConstants::PLAYER_SPAWNER_COMPONENT_ID)
	{
		NetDriver->PlayerSpawner->ReceivePlayerSpawnResponseOnClient(CommandResponseOp);
		EventTracer->TraceEvent(FSpatialTraceEventBuilder::CreateReceiveCommandResponse(TEXT("SPAWN_PLAYER_COMMAND"), RequestId),
								Op.span_id);
		return;
	}
	else if (ComponentId == SpatialConstants::SERVER_WORKER_COMPONENT_ID)
	{
		NetDriver->PlayerSpawner->ReceiveForwardPlayerSpawnResponse(CommandResponseOp);
		TOptional<Trace_SpanId> SpanId = EventTracer->CreateSpan(&Op.span_id, 1);
		EventTracer->TraceEvent(
			FSpatialTraceEventBuilder::CreateReceiveCommandResponse(TEXT("SERVER_WORKER_FORWARD_SPAWN_REQUEST_COMMAND"), RequestId),
			SpanId);
		return;
	}
	if (Op.op.command_response.response.component_id == SpatialConstants::WORKER_COMPONENT_ID
		&& Op.op.command_response.response.command_index == SpatialConstants::WORKER_CLAIM_PARTITION_COMMAND_ID)
	{
		ReceiveClaimPartitionResponse(Op.op.command_response);
		return;
	}

	ReceiveCommandResponse(Op);
}

void USpatialReceiver::ReceiveClaimPartitionResponse(const Worker_CommandResponseOp& Op)
{
	const Worker_PartitionId PartitionId = PendingPartitionAssignments.FindAndRemoveChecked(Op.request_id);

	if (Op.status_code == WORKER_STATUS_CODE_TIMEOUT)
	{
		UE_LOG(LogSpatialVirtualWorkerTranslationManager, Warning,
			   TEXT("ClaimPartition command timed out. "
					"Worker sytem entity: %lld. Retrying"),
			   Op.entity_id);

		// We don't worry about checking if we're resending this request twice, setting to the same value should be idempotent.
		Sender->SendClaimPartitionRequest(Op.entity_id, PartitionId);
		return;
	}

	if (Op.status_code != WORKER_STATUS_CODE_SUCCESS)
	{
		UE_LOG(LogSpatialVirtualWorkerTranslationManager, Error,
			   TEXT("ClaimPartition command failed for a reason other than timeout. "
					"This is fatal. Partition entity: %lld. Reason: %s"),
			   PartitionId, UTF8_TO_TCHAR(Op.message));
		return;
	}

	UE_LOG(LogSpatialVirtualWorkerTranslationManager, Log,
		   TEXT("ClaimPartition command succeeded. "
				"Worker sytem entity: %lld. Partition entity: %lld"),
		   Op.entity_id, PartitionId);
}

void USpatialReceiver::FlushRetryRPCs()
{
	Sender->FlushRetryRPCs();
}

void USpatialReceiver::ReceiveCommandResponse(const Worker_Op& Op)
{
	const Worker_CommandResponseOp& CommandResponseOp = Op.op.command_response;
	const Worker_CommandResponse& Repsonse = CommandResponseOp.response;
	const Worker_EntityId EntityId = CommandResponseOp.entity_id;
	const Worker_ComponentId ComponentId = Repsonse.component_id;
	const Worker_RequestId RequestId = CommandResponseOp.request_id;
	const uint8_t StatusCode = CommandResponseOp.status_code;

	AActor* TargetActor = Cast<AActor>(PackageMap->GetObjectFromEntityId(EntityId));
	TSharedRef<FReliableRPCForRetry>* ReliableRPCPtr = PendingReliableRPCs.Find(RequestId);
	if (ReliableRPCPtr == nullptr)
	{
		// We received a response for some other command, ignore.
		EventTracer->TraceEvent(FSpatialTraceEventBuilder::CreateReceiveCommandResponse(TargetActor, RequestId, false), Op.span_id);
		return;
	}

	TSharedRef<FReliableRPCForRetry> ReliableRPC = *ReliableRPCPtr;
	PendingReliableRPCs.Remove(RequestId);

	UObject* TargetObject = ReliableRPC->TargetObject.Get() != TargetActor ? ReliableRPC->TargetObject.Get() : nullptr;
	EventTracer->TraceEvent(FSpatialTraceEventBuilder::CreateReceiveCommandResponse(TargetActor, TargetObject, ReliableRPC->Function,
																					RequestId, WORKER_STATUS_CODE_SUCCESS),
							Op.span_id);

	if (StatusCode != WORKER_STATUS_CODE_SUCCESS)
	{
		bool bCanRetry = false;

		// Only attempt to retry if the error code indicates it makes sense too
		if ((StatusCode == WORKER_STATUS_CODE_TIMEOUT || StatusCode == WORKER_STATUS_CODE_NOT_FOUND)
			&& (ReliableRPC->Attempts < SpatialConstants::MAX_NUMBER_COMMAND_ATTEMPTS))
		{
			bCanRetry = true;
		}
		// Don't apply the retry limit on auth lost, as it should eventually succeed
		else if (StatusCode == WORKER_STATUS_CODE_AUTHORITY_LOST)
		{
			bCanRetry = true;
		}

		if (bCanRetry)
		{
			float WaitTime = SpatialConstants::GetCommandRetryWaitTimeSeconds(ReliableRPC->Attempts);
			UE_LOG(LogSpatialReceiver, Log, TEXT("%s: retrying in %f seconds. Error code: %d Message: %s"),
				   *ReliableRPC->Function->GetName(), WaitTime, (int)StatusCode, UTF8_TO_TCHAR(CommandResponseOp.message));

			if (!ReliableRPC->TargetObject.IsValid())
			{
				UE_LOG(LogSpatialReceiver, Warning, TEXT("%s: target object was destroyed before we could deliver the RPC."),
					   *ReliableRPC->Function->GetName());
				return;
			}

			// Queue retry
			FTimerHandle RetryTimer;
			TimerManager->SetTimer(
				RetryTimer,
				[WeakSender = TWeakObjectPtr<USpatialSender>(Sender), ReliableRPC]() {
					if (USpatialSender* SpatialSender = WeakSender.Get())
					{
						SpatialSender->EnqueueRetryRPC(ReliableRPC);
					}
				},
				WaitTime, false);
		}
		else
		{
			UE_LOG(LogSpatialReceiver, Error, TEXT("%s: failed too many times, giving up (%u attempts). Error code: %d Message: %s"),
				   *ReliableRPC->Function->GetName(), SpatialConstants::MAX_NUMBER_COMMAND_ATTEMPTS, (int)StatusCode,
				   UTF8_TO_TCHAR(CommandResponseOp.message));
		}
	}
}

void USpatialReceiver::ApplyComponentUpdate(const Worker_ComponentUpdate& ComponentUpdate, UObject& TargetObject,
											USpatialActorChannel& Channel, bool bIsHandover)
{
	RepStateUpdateHelper RepStateHelper(Channel, TargetObject);

	ComponentReader Reader(NetDriver, RepStateHelper.GetRefMap(), EventTracer);
	bool bOutReferencesChanged = false;
	Reader.ApplyComponentUpdate(ComponentUpdate, TargetObject, Channel, bIsHandover, bOutReferencesChanged);
	RepStateHelper.Update(*this, Channel, TargetObject, bOutReferencesChanged);

	// This is a temporary workaround, see UNR-841:
	// If the update includes tearoff, close the channel and clean up the entity.
	if (TargetObject.IsA<AActor>() && ClassInfoManager->GetCategoryByComponentId(ComponentUpdate.component_id) == SCHEMA_Data)
	{
		Schema_Object* ComponentObject = Schema_GetComponentUpdateFields(ComponentUpdate.schema_type);

		// Check if bTearOff has been set to true
		if (GetBoolFromSchema(ComponentObject, SpatialConstants::ACTOR_TEAROFF_ID))
		{
			Channel.ConditionalCleanUp(false, EChannelCloseReason::TearOff);
		}
	}
}

void USpatialReceiver::OnReserveEntityIdsResponse(const Worker_ReserveEntityIdsResponseOp& Op)
{
	SCOPE_CYCLE_COUNTER(STAT_ReceiverReserveEntityIds);
	if (Op.status_code != WORKER_STATUS_CODE_SUCCESS)
	{
		UE_LOG(LogSpatialReceiver, Warning, TEXT("ReserveEntityIds request failed: request id: %d, message: %s"), Op.request_id,
			   UTF8_TO_TCHAR(Op.message));
	}
	else
	{
		UE_LOG(LogSpatialReceiver, Verbose, TEXT("ReserveEntityIds request succeeded: request id: %d, message: %s"), Op.request_id,
			   UTF8_TO_TCHAR(Op.message));
	}

	if (ReserveEntityIDsDelegate* RequestDelegate = ReserveEntityIDsDelegates.Find(Op.request_id))
	{
		UE_LOG(LogSpatialReceiver, Log,
			   TEXT("Executing ReserveEntityIdsResponse with delegate, request id: %d, first entity id: %lld, message: %s"), Op.request_id,
			   Op.first_entity_id, UTF8_TO_TCHAR(Op.message));
		RequestDelegate->ExecuteIfBound(Op);
		ReserveEntityIDsDelegates.Remove(Op.request_id);
	}
	else
	{
		UE_LOG(LogSpatialReceiver, Warning,
			   TEXT("Received ReserveEntityIdsResponse but with no delegate set, request id: %d, first entity id: %lld, message: %s"),
			   Op.request_id, Op.first_entity_id, UTF8_TO_TCHAR(Op.message));
	}
}

void USpatialReceiver::OnCreateEntityResponse(const Worker_Op& Op)
{
	const Worker_CreateEntityResponseOp& CreateEntityResponseOp = Op.op.create_entity_response;
	const Worker_EntityId EntityId = CreateEntityResponseOp.entity_id;
	const Worker_RequestId RequestId = CreateEntityResponseOp.request_id;
	const uint8_t StatusCode = CreateEntityResponseOp.status_code;

	SCOPE_CYCLE_COUNTER(STAT_ReceiverCreateEntityResponse);
	switch (static_cast<Worker_StatusCode>(StatusCode))
	{
	case WORKER_STATUS_CODE_SUCCESS:
		UE_LOG(LogSpatialReceiver, Verbose,
			   TEXT("Create entity request succeeded. "
					"Request id: %d, entity id: %lld, message: %s"),
			   RequestId, EntityId, UTF8_TO_TCHAR(CreateEntityResponseOp.message));
		break;
	case WORKER_STATUS_CODE_TIMEOUT:
		UE_LOG(LogSpatialReceiver, Verbose,
			   TEXT("Create entity request timed out. "
					"Request id: %d, entity id: %lld, message: %s"),
			   RequestId, EntityId, UTF8_TO_TCHAR(CreateEntityResponseOp.message));
		break;
	case WORKER_STATUS_CODE_APPLICATION_ERROR:
		UE_LOG(LogSpatialReceiver, Verbose,
			   TEXT("Create entity request failed. "
					"Either the reservation expired, the entity already existed, or the entity was invalid. "
					"Request id: %d, entity id: %lld, message: %s"),
			   RequestId, EntityId, UTF8_TO_TCHAR(CreateEntityResponseOp.message));
		break;
	default:
		UE_LOG(LogSpatialReceiver, Error,
			   TEXT("Create entity request failed. This likely indicates a bug in the Unreal GDK and should be reported. "
					"Request id: %d, entity id: %lld, message: %s"),
			   RequestId, EntityId, UTF8_TO_TCHAR(CreateEntityResponseOp.message));
		break;
	}

	if (CreateEntityDelegate* Delegate = CreateEntityDelegates.Find(RequestId))
	{
		Delegate->ExecuteIfBound(CreateEntityResponseOp);
		CreateEntityDelegates.Remove(RequestId);
	}

	TWeakObjectPtr<USpatialActorChannel> Channel = PopPendingActorRequest(RequestId);

	// It's possible for the ActorChannel to have been closed by the time we receive a response. Actor validity is checked within the
	// channel.
	if (Channel.IsValid())
	{
		Channel->OnCreateEntityResponse(CreateEntityResponseOp);

		TOptional<Trace_SpanId> SpanId = EventTracer->CreateSpan(&Op.span_id, 1);
		EventTracer->TraceEvent(FSpatialTraceEventBuilder::CreateReceiveCreateEntitySuccess(Channel->Actor, EntityId), SpanId);
	}
	else if (Channel.IsStale())
	{
		UE_LOG(LogSpatialReceiver, Verbose,
			   TEXT("Received CreateEntityResponse for actor which no longer has an actor channel: "
					"request id: %d, entity id: %lld. This should only happen in the case where we attempt to delete the entity before we "
					"have authority. "
					"The entity will therefore be deleted once authority is gained."),
			   RequestId, EntityId);

		FString Message =
			FString::Printf(TEXT("Stale Actor Channel - tried to delete entity before gaining authority. Actor - %s EntityId - %d"),
							*Channel->Actor->GetName(), EntityId);

		TOptional<Trace_SpanId> SpanId = EventTracer->CreateSpan(&Op.span_id, 1);
		EventTracer->TraceEvent(FSpatialTraceEventBuilder::CreateGenericMessage(Message), SpanId);
	}
	else
	{
		TOptional<Trace_SpanId> SpanId = EventTracer->CreateSpan(&Op.span_id, 1);
		EventTracer->TraceEvent(FSpatialTraceEventBuilder::CreateGenericMessage(TEXT("Create entity response unknown error")), SpanId);
	}
}

void USpatialReceiver::OnEntityQueryResponse(const Worker_EntityQueryResponseOp& Op)
{
	SCOPE_CYCLE_COUNTER(STAT_ReceiverEntityQueryResponse);
	if (Op.status_code != WORKER_STATUS_CODE_SUCCESS)
	{
		UE_LOG(LogSpatialReceiver, Error, TEXT("EntityQuery failed: request id: %d, message: %s"), Op.request_id,
			   UTF8_TO_TCHAR(Op.message));
	}

	if (EntityQueryDelegate* RequestDelegate = EntityQueryDelegates.Find(Op.request_id))
	{
		UE_LOG(LogSpatialReceiver, Verbose,
			   TEXT("Executing EntityQueryResponse with delegate, request id: %d, number of entities: %d, message: %s"), Op.request_id,
			   Op.result_count, UTF8_TO_TCHAR(Op.message));
		RequestDelegate->ExecuteIfBound(Op);
	}
	else
	{
		UE_LOG(LogSpatialReceiver, Warning,
			   TEXT("Received EntityQueryResponse but with no delegate set, request id: %d, number of entities: %d, message: %s"),
			   Op.request_id, Op.result_count, UTF8_TO_TCHAR(Op.message));
	}
}

void USpatialReceiver::AddPendingActorRequest(Worker_RequestId RequestId, USpatialActorChannel* Channel)
{
	PendingActorRequests.Add(RequestId, Channel);
}

void USpatialReceiver::AddPendingReliableRPC(Worker_RequestId RequestId, TSharedRef<FReliableRPCForRetry> ReliableRPC)
{
	PendingReliableRPCs.Add(RequestId, ReliableRPC);
}

void USpatialReceiver::AddEntityQueryDelegate(Worker_RequestId RequestId, EntityQueryDelegate Delegate)
{
	EntityQueryDelegates.Add(RequestId, MoveTemp(Delegate));
}

void USpatialReceiver::AddReserveEntityIdsDelegate(Worker_RequestId RequestId, ReserveEntityIDsDelegate Delegate)
{
	ReserveEntityIDsDelegates.Add(RequestId, MoveTemp(Delegate));
}

void USpatialReceiver::AddCreateEntityDelegate(Worker_RequestId RequestId, CreateEntityDelegate Delegate)
{
	CreateEntityDelegates.Add(RequestId, MoveTemp(Delegate));
}

TWeakObjectPtr<USpatialActorChannel> USpatialReceiver::PopPendingActorRequest(Worker_RequestId RequestId)
{
	TWeakObjectPtr<USpatialActorChannel>* ChannelPtr = PendingActorRequests.Find(RequestId);
	if (ChannelPtr == nullptr)
	{
		return nullptr;
	}
	TWeakObjectPtr<USpatialActorChannel> Channel = *ChannelPtr;
	PendingActorRequests.Remove(RequestId);
	return Channel;
}

void USpatialReceiver::OnDisconnect(uint8 StatusCode, const FString& Reason)
{
	if (GEngine != nullptr)
	{
		GEngine->BroadcastNetworkFailure(NetDriver->GetWorld(), NetDriver, ENetworkFailure::FromDisconnectOpStatusCode(StatusCode), Reason);
	}
}

bool USpatialReceiver::IsPendingOpsOnChannel(USpatialActorChannel& Channel)
{
	SCOPE_CYCLE_COUNTER(STAT_SpatialPendingOpsOnChannel);

	// Don't allow Actors to go dormant if they have any pending operations waiting on their channel

	for (const auto& RefMap : Channel.ObjectReferenceMap)
	{
		if (RefMap.Value.HasUnresolved())
		{
			return true;
		}
	}

	for (const auto& ActorRequest : PendingActorRequests)
	{
		if (ActorRequest.Value == &Channel)
		{
			return true;
		}
	}

	return false;
}

void USpatialReceiver::ResolvePendingOperations(UObject* Object, const FUnrealObjectRef& ObjectRef)
{
	UE_LOG(LogSpatialReceiver, Verbose, TEXT("Resolving pending object refs and RPCs which depend on object: %s %s."), *Object->GetName(),
		   *ObjectRef.ToString());

	ResolveIncomingOperations(Object, ObjectRef);

	// When resolving an Actor that should uniquely exist in a deployment, e.g. GameMode, GameState, LevelScriptActors, we also
	// resolve using class path (in case any properties were set from a server that hasn't resolved the Actor yet).
	if (FUnrealObjectRef::ShouldLoadObjectFromClassPath(Object))
	{
		FUnrealObjectRef ClassObjectRef = FUnrealObjectRef::GetRefFromObjectClassPath(Object, PackageMap);
		if (ClassObjectRef.IsValid())
		{
			ResolveIncomingOperations(Object, ClassObjectRef);
		}
	}

	// TODO: UNR-1650 We're trying to resolve all queues, which introduces more overhead.
	RPCService->ProcessIncomingRPCs();
}

void USpatialReceiver::ResolveIncomingOperations(UObject* Object, const FUnrealObjectRef& ObjectRef)
{
	// TODO: queue up resolved objects since they were resolved during process ops
	// and then resolve all of them at the end of process ops - UNR:582

	TSet<FChannelObjectPair>* TargetObjectSet = ObjectRefToRepStateMap.Find(ObjectRef);
	if (!TargetObjectSet)
	{
		return;
	}

	UE_LOG(LogSpatialReceiver, Verbose, TEXT("Resolving incoming operations depending on object ref %s, resolved object: %s"),
		   *ObjectRef.ToString(), *Object->GetName());

	for (auto ChannelObjectIter = TargetObjectSet->CreateIterator(); ChannelObjectIter; ++ChannelObjectIter)
	{
		USpatialActorChannel* DependentChannel = ChannelObjectIter->Key.Get();
		if (!DependentChannel)
		{
			ChannelObjectIter.RemoveCurrent();
			continue;
		}

		UObject* ReplicatingObject = ChannelObjectIter->Value.Get();

		if (!ReplicatingObject)
		{
			if (DependentChannel->ObjectReferenceMap.Find(ChannelObjectIter->Value))
			{
				DependentChannel->ObjectReferenceMap.Remove(ChannelObjectIter->Value);
				ChannelObjectIter.RemoveCurrent();
			}
			continue;
		}

		FSpatialObjectRepState* RepState = DependentChannel->ObjectReferenceMap.Find(ChannelObjectIter->Value);
		if (!RepState || !RepState->UnresolvedRefs.Contains(ObjectRef))
		{
			continue;
		}

		// Check whether the resolved object has been torn off, or is on an actor that has been torn off.
		if (AActor* AsActor = Cast<AActor>(ReplicatingObject))
		{
			if (AsActor->GetTearOff())
			{
				UE_LOG(LogSpatialActorChannel, Log,
					   TEXT("Actor to be resolved was torn off, so ignoring incoming operations. Object ref: %s, resolved object: %s"),
					   *ObjectRef.ToString(), *Object->GetName());
				DependentChannel->ObjectReferenceMap.Remove(ChannelObjectIter->Value);
				continue;
			}
		}
		else if (AActor* OuterActor = ReplicatingObject->GetTypedOuter<AActor>())
		{
			if (OuterActor->GetTearOff())
			{
				UE_LOG(LogSpatialActorChannel, Log,
					   TEXT("Owning Actor of the object to be resolved was torn off, so ignoring incoming operations. Object ref: %s, "
							"resolved object: %s"),
					   *ObjectRef.ToString(), *Object->GetName());
				DependentChannel->ObjectReferenceMap.Remove(ChannelObjectIter->Value);
				continue;
			}
		}

		bool bSomeObjectsWereMapped = false;
		TArray<GDK_PROPERTY(Property)*> RepNotifies;

		FRepLayout& RepLayout = DependentChannel->GetObjectRepLayout(ReplicatingObject);
		FRepStateStaticBuffer& ShadowData = DependentChannel->GetObjectStaticBuffer(ReplicatingObject);
		if (ShadowData.Num() == 0)
		{
			DependentChannel->ResetShadowData(RepLayout, ShadowData, ReplicatingObject);
		}

		ResolveObjectReferences(RepLayout, ReplicatingObject, *RepState, RepState->ReferenceMap, ShadowData.GetData(),
								(uint8*)ReplicatingObject, ReplicatingObject->GetClass()->GetPropertiesSize(), RepNotifies,
								bSomeObjectsWereMapped);

		if (bSomeObjectsWereMapped)
		{
			DependentChannel->RemoveRepNotifiesWithUnresolvedObjs(RepNotifies, RepLayout, RepState->ReferenceMap, ReplicatingObject);

			UE_LOG(LogSpatialReceiver, Verbose, TEXT("Resolved for target object %s"), *ReplicatingObject->GetName());
			DependentChannel->PostReceiveSpatialUpdate(ReplicatingObject, RepNotifies, {});
		}

		RepState->UnresolvedRefs.Remove(ObjectRef);
	}
}

void USpatialReceiver::ResolveObjectReferences(FRepLayout& RepLayout, UObject* ReplicatedObject, FSpatialObjectRepState& RepState,
											   FObjectReferencesMap& ObjectReferencesMap, uint8* RESTRICT StoredData, uint8* RESTRICT Data,
											   int32 MaxAbsOffset, TArray<GDK_PROPERTY(Property) *>& RepNotifies,
											   bool& bOutSomeObjectsWereMapped)
{
	for (auto It = ObjectReferencesMap.CreateIterator(); It; ++It)
	{
		int32 AbsOffset = It.Key();

		if (AbsOffset >= MaxAbsOffset)
		{
			UE_LOG(LogSpatialReceiver, Error, TEXT("ResolveObjectReferences: Removed unresolved reference: AbsOffset >= MaxAbsOffset: %d"),
				   AbsOffset);
			It.RemoveCurrent();
			continue;
		}

		FObjectReferences& ObjectReferences = It.Value();

		GDK_PROPERTY(Property)* Property = ObjectReferences.Property;

		// ParentIndex is -1 for handover properties
		bool bIsHandover = ObjectReferences.ParentIndex == -1;
		FRepParentCmd* Parent = ObjectReferences.ParentIndex >= 0 ? &RepLayout.Parents[ObjectReferences.ParentIndex] : nullptr;

		int32 StoredDataOffset = ObjectReferences.ShadowOffset;

		if (ObjectReferences.Array)
		{
			GDK_PROPERTY(ArrayProperty)* ArrayProperty = GDK_CASTFIELD<GDK_PROPERTY(ArrayProperty)>(Property);
			check(ArrayProperty != nullptr);

			if (!bIsHandover)
			{
				Property->CopySingleValue(StoredData + StoredDataOffset, Data + AbsOffset);
			}

			FScriptArray* StoredArray = bIsHandover ? nullptr : (FScriptArray*)(StoredData + StoredDataOffset);
			FScriptArray* Array = (FScriptArray*)(Data + AbsOffset);

			int32 NewMaxOffset = Array->Num() * ArrayProperty->Inner->ElementSize;

			ResolveObjectReferences(RepLayout, ReplicatedObject, RepState, *ObjectReferences.Array,
									bIsHandover ? nullptr : (uint8*)StoredArray->GetData(), (uint8*)Array->GetData(), NewMaxOffset,
									RepNotifies, bOutSomeObjectsWereMapped);
			continue;
		}

		bool bResolvedSomeRefs = false;
		UObject* SinglePropObject = nullptr;
		FUnrealObjectRef SinglePropRef = FUnrealObjectRef::NULL_OBJECT_REF;

		for (auto UnresolvedIt = ObjectReferences.UnresolvedRefs.CreateIterator(); UnresolvedIt; ++UnresolvedIt)
		{
			FUnrealObjectRef& ObjectRef = *UnresolvedIt;

			bool bUnresolved = false;
			UObject* Object = FUnrealObjectRef::ToObjectPtr(ObjectRef, PackageMap, bUnresolved);
			if (!bUnresolved)
			{
				check(Object != nullptr);

				UE_LOG(LogSpatialReceiver, Verbose,
					   TEXT("ResolveObjectReferences: Resolved object ref: Offset: %d, Object ref: %s, PropName: %s, ObjName: %s"),
					   AbsOffset, *ObjectRef.ToString(), *Property->GetNameCPP(), *Object->GetName());

				if (ObjectReferences.bSingleProp)
				{
					SinglePropObject = Object;
					SinglePropRef = ObjectRef;
				}

				UnresolvedIt.RemoveCurrent();

				bResolvedSomeRefs = true;
			}
		}

		if (bResolvedSomeRefs)
		{
			if (!bOutSomeObjectsWereMapped)
			{
				ReplicatedObject->PreNetReceive();
				bOutSomeObjectsWereMapped = true;
			}

			if (Parent && Parent->Property->HasAnyPropertyFlags(CPF_RepNotify))
			{
				Property->CopySingleValue(StoredData + StoredDataOffset, Data + AbsOffset);
			}

			if (ObjectReferences.bSingleProp)
			{
				GDK_PROPERTY(ObjectPropertyBase)* ObjectProperty = GDK_CASTFIELD<GDK_PROPERTY(ObjectPropertyBase)>(Property);
				check(ObjectProperty);

				ObjectProperty->SetObjectPropertyValue(Data + AbsOffset, SinglePropObject);
				ObjectReferences.MappedRefs.Add(SinglePropRef);
			}
			else if (ObjectReferences.bFastArrayProp)
			{
				TSet<FUnrealObjectRef> NewMappedRefs;
				TSet<FUnrealObjectRef> NewUnresolvedRefs;
				FSpatialNetBitReader ValueDataReader(PackageMap, ObjectReferences.Buffer.GetData(), ObjectReferences.NumBufferBits,
													 NewMappedRefs, NewUnresolvedRefs);

				check(Property->IsA<GDK_PROPERTY(ArrayProperty)>());
				UScriptStruct* NetDeltaStruct = GetFastArraySerializerProperty(GDK_CASTFIELD<GDK_PROPERTY(ArrayProperty)>(Property));

				FSpatialNetDeltaSerializeInfo::DeltaSerializeRead(NetDriver, ValueDataReader, ReplicatedObject, Parent->ArrayIndex,
																  Parent->Property, NetDeltaStruct);

				ObjectReferences.MappedRefs.Append(NewMappedRefs);
			}
			else
			{
				TSet<FUnrealObjectRef> NewMappedRefs;
				TSet<FUnrealObjectRef> NewUnresolvedRefs;
				FSpatialNetBitReader BitReader(PackageMap, ObjectReferences.Buffer.GetData(), ObjectReferences.NumBufferBits, NewMappedRefs,
											   NewUnresolvedRefs);
				check(Property->IsA<GDK_PROPERTY(StructProperty)>());

				bool bHasUnresolved = false;
				ReadStructProperty(BitReader, GDK_CASTFIELD<GDK_PROPERTY(StructProperty)>(Property), NetDriver, Data + AbsOffset,
								   bHasUnresolved);

				ObjectReferences.MappedRefs.Append(NewMappedRefs);
			}

			if (Parent && Parent->Property->HasAnyPropertyFlags(CPF_RepNotify))
			{
				if (Parent->RepNotifyCondition == REPNOTIFY_Always || !Property->Identical(StoredData + StoredDataOffset, Data + AbsOffset))
				{
					RepNotifies.AddUnique(Parent->Property);
				}
			}
		}
	}
}

void USpatialReceiver::OnHeartbeatComponentUpdate(const Worker_ComponentUpdateOp& Op)
{
	if (!NetDriver->IsServer())
	{
		// Clients can ignore Heartbeat component updates.
		return;
	}

	TWeakObjectPtr<USpatialNetConnection>* ConnectionPtr = AuthorityPlayerControllerConnectionMap.Find(Op.entity_id);
	if (ConnectionPtr == nullptr)
	{
		// Heartbeat component update on a PlayerController that this server does not have authority over.
		// TODO: Disable component interest for Heartbeat components this server doesn't care about - UNR-986
		return;
	}

	if (!ConnectionPtr->IsValid())
	{
		UE_LOG(LogSpatialReceiver, Warning,
			   TEXT("Received heartbeat component update after NetConnection has been cleaned up. PlayerController entity: %lld"),
			   Op.entity_id);
		AuthorityPlayerControllerConnectionMap.Remove(Op.entity_id);
		return;
	}

	USpatialNetConnection* NetConnection = ConnectionPtr->Get();

	Schema_Object* EventsObject = Schema_GetComponentUpdateEvents(Op.update.schema_type);
	uint32 EventCount = Schema_GetObjectCount(EventsObject, SpatialConstants::HEARTBEAT_EVENT_ID);
	if (EventCount > 0)
	{
		if (EventCount > 1)
		{
			UE_LOG(LogSpatialReceiver, Verbose, TEXT("Received multiple heartbeat events in a single component update, entity %lld."),
				   Op.entity_id);
		}

		NetConnection->OnHeartbeat();
	}

	Schema_Object* FieldsObject = Schema_GetComponentUpdateFields(Op.update.schema_type);
	if (Schema_GetBoolCount(FieldsObject, SpatialConstants::HEARTBEAT_CLIENT_HAS_QUIT_ID) > 0
		&& GetBoolFromSchema(FieldsObject, SpatialConstants::HEARTBEAT_CLIENT_HAS_QUIT_ID))
	{
		// Client has disconnected, let's clean up their connection.
		CloseClientConnection(NetConnection, Op.entity_id);
	}
}

void USpatialReceiver::CloseClientConnection(USpatialNetConnection* ClientConnection, Worker_EntityId PlayerControllerEntityId)
{
	ClientConnection->CleanUp();
	AuthorityPlayerControllerConnectionMap.Remove(PlayerControllerEntityId);
}

bool USpatialReceiver::NeedToLoadClass(const FString& ClassPath)
{
	UObject* ClassObject = FindObject<UClass>(nullptr, *ClassPath, false);
	if (ClassObject == nullptr)
	{
		return true;
	}

	FString PackagePath = GetPackagePath(ClassPath);
	FName PackagePathName = *PackagePath;

	// UNR-3320 The following test checks if the package is currently being processed in the async loading thread.
	// Without it, we could be using an object loaded in memory, but not completely ready to be used.
	// Looking through PackageMapClient's code, which handles asset async loading in Native unreal, checking
	// UPackage::IsFullyLoaded, or UObject::HasAnyInternalFlag(EInternalObjectFlag::AsyncLoading) should tell us if it is the case.
	// In practice, these tests are not enough to prevent using objects too early (symptom is RF_NeedPostLoad being set, and crash when
	// using them later). GetAsyncLoadPercentage will actually look through the async loading thread's UAsyncPackage maps to see if there
	// are any entries.
	// TODO : UNR-3374 This looks like an expensive check, but it does the job. We should investigate further
	// what is the issue with the other flags and why they do not give us reliable information.

	float Percentage = GetAsyncLoadPercentage(PackagePathName);
	if (Percentage != -1.0f)
	{
		UE_LOG(LogSpatialReceiver, Warning, TEXT("Class %s package is registered in async loading thread."), *ClassPath)
		return true;
	}

	return false;
}

FString USpatialReceiver::GetPackagePath(const FString& ClassPath)
{
	return FSoftObjectPath(ClassPath).GetLongPackageName();
}

void USpatialReceiver::StartAsyncLoadingClass(const FString& ClassPath, Worker_EntityId EntityId)
{
	FString PackagePath = GetPackagePath(ClassPath);
	FName PackagePathName = *PackagePath;

	bool bAlreadyLoading = AsyncLoadingPackages.Contains(PackagePathName);

	if (IsEntityWaitingForAsyncLoad(EntityId))
	{
		// This shouldn't happen because even if the entity goes out and comes back into view,
		// we would've received a RemoveEntity op that would remove the entry from the map.
		UE_LOG(LogSpatialReceiver, Error,
			   TEXT("USpatialReceiver::ReceiveActor: Checked out entity but it's already waiting for async load! Entity: %lld"), EntityId);
	}

	EntityWaitingForAsyncLoad AsyncLoadEntity;
	AsyncLoadEntity.ClassPath = ClassPath;
	AsyncLoadEntity.InitialPendingAddComponents = ExtractAddComponents(EntityId);
	AsyncLoadEntity.PendingOps = ExtractAuthorityOps(EntityId);

	EntitiesWaitingForAsyncLoad.Emplace(EntityId, MoveTemp(AsyncLoadEntity));
	AsyncLoadingPackages.FindOrAdd(PackagePathName).Add(EntityId);

	UE_LOG(LogSpatialReceiver, Log, TEXT("Async loading package %s for entity %lld. Already loading: %s"), *PackagePath, EntityId,
		   bAlreadyLoading ? TEXT("true") : TEXT("false"));
	if (!bAlreadyLoading)
	{
		LoadPackageAsync(PackagePath, FLoadPackageAsyncDelegate::CreateUObject(this, &USpatialReceiver::OnAsyncPackageLoaded));
	}
}

void USpatialReceiver::OnAsyncPackageLoaded(const FName& PackageName, UPackage* Package, EAsyncLoadingResult::Type Result)
{
	TArray<Worker_EntityId> Entities;
	if (!AsyncLoadingPackages.RemoveAndCopyValue(PackageName, Entities))
	{
		UE_LOG(LogSpatialReceiver, Error,
			   TEXT("USpatialReceiver::OnAsyncPackageLoaded: Package loaded but no entry in AsyncLoadingPackages. Package: %s"),
			   *PackageName.ToString());
		return;
	}

	if (Result != EAsyncLoadingResult::Succeeded)
	{
		UE_LOG(LogSpatialReceiver, Error, TEXT("USpatialReceiver::OnAsyncPackageLoaded: Package was not loaded successfully. Package: %s"),
			   *PackageName.ToString());
		return;
	}

	for (Worker_EntityId Entity : Entities)
	{
		if (IsEntityWaitingForAsyncLoad(Entity))
		{
			UE_LOG(LogSpatialReceiver, Log, TEXT("Finished async loading package %s for entity %lld."), *PackageName.ToString(), Entity);

			// Save critical section if we're in one and restore upon leaving this scope.
			CriticalSectionSaveState CriticalSectionState(*this);

			EntityWaitingForAsyncLoad AsyncLoadEntity = EntitiesWaitingForAsyncLoad.FindAndRemoveChecked(Entity);
			PendingAddActors.Add(Entity);
			PendingAddComponents = MoveTemp(AsyncLoadEntity.InitialPendingAddComponents);
			LeaveCriticalSection();

			OpList Ops = MoveTemp(AsyncLoadEntity.PendingOps).CreateOpList();
			for (uint32 i = 0; i < Ops.Count; ++i)
			{
				HandleQueuedOpForAsyncLoad(Ops.Ops[i]);
			}
		}
	}
}

void USpatialReceiver::MoveMappedObjectToUnmapped(const FUnrealObjectRef& Ref)
{
	if (TSet<FChannelObjectPair>* RepStatesWithMappedRef = ObjectRefToRepStateMap.Find(Ref))
	{
		for (const FChannelObjectPair& ChannelObject : *RepStatesWithMappedRef)
		{
			if (USpatialActorChannel* Channel = ChannelObject.Key.Get())
			{
				if (FSpatialObjectRepState* RepState = Channel->ObjectReferenceMap.Find(ChannelObject.Value))
				{
					RepState->MoveMappedObjectToUnmapped(Ref);
				}
			}
		}
	}
}

void USpatialReceiver::RetireWhenAuthoritive(Worker_EntityId EntityId, Worker_ComponentId ActorClassId, bool bIsNetStartup,
											 bool bNeedsTearOff)
{
	DeferredRetire DeferredObj = { EntityId, ActorClassId, bIsNetStartup, bNeedsTearOff };
	EntitiesToRetireOnAuthorityGain.Add(DeferredObj);
}

bool USpatialReceiver::IsEntityWaitingForAsyncLoad(Worker_EntityId Entity)
{
	return EntitiesWaitingForAsyncLoad.Contains(Entity);
}

void USpatialReceiver::QueueAddComponentOpForAsyncLoad(const Worker_AddComponentOp& Op)
{
	EntityWaitingForAsyncLoad& AsyncLoadEntity = EntitiesWaitingForAsyncLoad.FindChecked(Op.entity_id);

	AsyncLoadEntity.PendingOps.AddComponent(Op.entity_id, ComponentData::CreateCopy(Op.data.schema_type, Op.data.component_id));
}

void USpatialReceiver::QueueRemoveComponentOpForAsyncLoad(const Worker_RemoveComponentOp& Op)
{
	EntityWaitingForAsyncLoad& AsyncLoadEntity = EntitiesWaitingForAsyncLoad.FindChecked(Op.entity_id);

	AsyncLoadEntity.PendingOps.RemoveComponent(Op.entity_id, Op.component_id);
}

void USpatialReceiver::QueueAuthorityOpForAsyncLoad(const Worker_ComponentSetAuthorityChangeOp& Op)
{
	EntityWaitingForAsyncLoad& AsyncLoadEntity = EntitiesWaitingForAsyncLoad.FindChecked(Op.entity_id);

	AsyncLoadEntity.PendingOps.SetAuthority(Op.entity_id, Op.component_set_id, static_cast<Worker_Authority>(Op.authority));
}

void USpatialReceiver::QueueComponentUpdateOpForAsyncLoad(const Worker_ComponentUpdateOp& Op)
{
	EntityWaitingForAsyncLoad& AsyncLoadEntity = EntitiesWaitingForAsyncLoad.FindChecked(Op.entity_id);

	AsyncLoadEntity.PendingOps.UpdateComponent(Op.entity_id, ComponentUpdate::CreateCopy(Op.update.schema_type, Op.update.component_id));
}

TArray<PendingAddComponentWrapper> USpatialReceiver::ExtractAddComponents(Worker_EntityId Entity)
{
	TArray<PendingAddComponentWrapper> ExtractedAddComponents;
	TArray<PendingAddComponentWrapper> RemainingAddComponents;

	for (PendingAddComponentWrapper& AddComponent : PendingAddComponents)
	{
		if (AddComponent.EntityId == Entity)
		{
			ExtractedAddComponents.Add(MoveTemp(AddComponent));
		}
		else
		{
			RemainingAddComponents.Add(MoveTemp(AddComponent));
		}
	}
	PendingAddComponents = MoveTemp(RemainingAddComponents);
	return ExtractedAddComponents;
}

EntityComponentOpListBuilder USpatialReceiver::ExtractAuthorityOps(Worker_EntityId Entity)
{
	EntityComponentOpListBuilder ExtractedOps;
	TArray<Worker_ComponentSetAuthorityChangeOp> RemainingOps;

	for (const Worker_ComponentSetAuthorityChangeOp& PendingAuthorityChange : PendingAuthorityChanges)
	{
		if (PendingAuthorityChange.entity_id == Entity)
		{
			ExtractedOps.SetAuthority(Entity, PendingAuthorityChange.component_set_id,
									  static_cast<Worker_Authority>(PendingAuthorityChange.authority));
		}
		else
		{
			RemainingOps.Add(PendingAuthorityChange);
		}
	}
	PendingAuthorityChanges = MoveTemp(RemainingOps);
	return ExtractedOps;
}

void USpatialReceiver::HandleQueuedOpForAsyncLoad(const Worker_Op& Op)
{
	switch (Op.op_type)
	{
	case WORKER_OP_TYPE_ADD_COMPONENT:
		OnAddComponent(Op.op.add_component);
		break;
	case WORKER_OP_TYPE_REMOVE_COMPONENT:
		ProcessRemoveComponent(Op.op.remove_component);
		break;
	case WORKER_OP_TYPE_COMPONENT_SET_AUTHORITY_CHANGE:
		HandleActorAuthority(Op.op.component_set_authority_change);
		break;
	case WORKER_OP_TYPE_COMPONENT_UPDATE:
		OnComponentUpdate(Op.op.component_update);
		break;
	default:
		checkNoEntry();
	}
}

USpatialReceiver::CriticalSectionSaveState::CriticalSectionSaveState(USpatialReceiver& InReceiver)
	: Receiver(InReceiver)
	, bInCriticalSection(InReceiver.bInCriticalSection)
{
	if (bInCriticalSection)
	{
		PendingAddActors = MoveTemp(Receiver.PendingAddActors);
		PendingAuthorityChanges = MoveTemp(Receiver.PendingAuthorityChanges);
		PendingAddComponents = MoveTemp(Receiver.PendingAddComponents);
		Receiver.PendingAddActors.Empty();
		Receiver.PendingAuthorityChanges.Empty();
		Receiver.PendingAddComponents.Empty();
	}
	Receiver.bInCriticalSection = true;
}

USpatialReceiver::CriticalSectionSaveState::~CriticalSectionSaveState()
{
	if (bInCriticalSection)
	{
		Receiver.PendingAddActors = MoveTemp(PendingAddActors);
		Receiver.PendingAuthorityChanges = MoveTemp(PendingAuthorityChanges);
		Receiver.PendingAddComponents = MoveTemp(PendingAddComponents);
	}
	Receiver.bInCriticalSection = bInCriticalSection;
}

namespace
{
FString GetObjectNameFromRepState(const FSpatialObjectRepState& RepState)
{
	if (UObject* Obj = RepState.GetChannelObjectPair().Value.Get())
	{
		return Obj->GetName();
	}
	return TEXT("<unknown>");
}
} // namespace

void USpatialReceiver::CleanupRepStateMap(FSpatialObjectRepState& RepState)
{
	for (const FUnrealObjectRef& Ref : RepState.ReferencedObj)
	{
		TSet<FChannelObjectPair>* RepStatesWithMappedRef = ObjectRefToRepStateMap.Find(Ref);
		if (ensureMsgf(RepStatesWithMappedRef,
					   TEXT("Ref to entity %lld on object %s is missing its referenced entry in the Ref/RepState map"), Ref.Entity,
					   *GetObjectNameFromRepState(RepState)))
		{
			checkf(RepStatesWithMappedRef->Contains(RepState.GetChannelObjectPair()),
				   TEXT("Ref to entity %lld on object %s is missing its referenced entry in the Ref/RepState map"), Ref.Entity,
				   *GetObjectNameFromRepState(RepState));
			RepStatesWithMappedRef->Remove(RepState.GetChannelObjectPair());
			if (RepStatesWithMappedRef->Num() == 0)
			{
				ObjectRefToRepStateMap.Remove(Ref);
			}
		}
	}
}

bool USpatialReceiver::HasEntityBeenRequestedForDelete(Worker_EntityId EntityId)
{
	return EntitiesToRetireOnAuthorityGain.ContainsByPredicate([EntityId](const DeferredRetire& Retire) {
		return EntityId == Retire.EntityId;
	});
}

void USpatialReceiver::HandleDeferredEntityDeletion(const DeferredRetire& Retire)
{
	if (Retire.bNeedsTearOff)
	{
		Sender->SendActorTornOffUpdate(Retire.EntityId, Retire.ActorClassId);
		NetDriver->DelayedRetireEntity(Retire.EntityId, 1.0f, Retire.bIsNetStartupActor);
	}
	else
	{
		Sender->RetireEntity(Retire.EntityId, Retire.bIsNetStartupActor);
	}
}

void USpatialReceiver::HandleEntityDeletedAuthority(Worker_EntityId EntityId)
{
	int32 Index = EntitiesToRetireOnAuthorityGain.IndexOfByPredicate([EntityId](const DeferredRetire& Retire) {
		return Retire.EntityId == EntityId;
	});
	if (Index != INDEX_NONE)
	{
		HandleDeferredEntityDeletion(EntitiesToRetireOnAuthorityGain[Index]);
	}
}

bool USpatialReceiver::IsDynamicSubObject(AActor* Actor, uint32 SubObjectOffset)
{
	const FClassInfo& ActorClassInfo = ClassInfoManager->GetOrCreateClassInfoByClass(Actor->GetClass());
	return !ActorClassInfo.SubobjectInfo.Contains(SubObjectOffset);
}
