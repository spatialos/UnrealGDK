// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "Interop/SpatialReceiver.h"

#include "Engine/Engine.h"
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "TimerManager.h"

#include "EngineClasses/SpatialActorChannel.h"
#include "EngineClasses/SpatialFastArrayNetSerialize.h"
#include "EngineClasses/SpatialGameInstance.h"
#include "EngineClasses/SpatialNetConnection.h"
#include "EngineClasses/SpatialPackageMapClient.h"
#include "Interop/Connection/SpatialWorkerConnection.h"
#include "Interop/GlobalStateManager.h"
#include "Interop/SpatialPlayerSpawner.h"
#include "Interop/SpatialSender.h"
#include "Schema/DynamicComponent.h"
#include "Schema/RPCPayload.h"
#include "Schema/SpawnData.h"
#include "Schema/UnrealMetadata.h"
#include "SpatialConstants.h"
#include "Utils/ComponentReader.h"
#include "Utils/ErrorCodeRemapping.h"
#include "Utils/RepLayoutUtils.h"

DEFINE_LOG_CATEGORY(LogSpatialReceiver);

using namespace SpatialGDK;

void USpatialReceiver::Init(USpatialNetDriver* InNetDriver, FTimerManager* InTimerManager)
{
	NetDriver = InNetDriver;
	StaticComponentView = InNetDriver->StaticComponentView;
	Sender = InNetDriver->Sender;
	PackageMap = InNetDriver->PackageMap;
	ClassInfoManager = InNetDriver->ClassInfoManager;
	GlobalStateManager = InNetDriver->GlobalStateManager;
	TimerManager = InTimerManager;
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
	UE_LOG(LogSpatialReceiver, Verbose, TEXT("Leaving critical section."));
	check(bInCriticalSection);

	for (Worker_EntityId& PendingAddEntity : PendingAddEntities)
	{
		ReceiveActor(PendingAddEntity);
	}

	for (Worker_AuthorityChangeOp& PendingAuthorityChange : PendingAuthorityChanges)
	{
		HandleActorAuthority(PendingAuthorityChange);
	}

	// Mark that we've left the critical section.
	bInCriticalSection = false;
	PendingAddEntities.Empty();
	PendingAddComponents.Empty();
	PendingAuthorityChanges.Empty();

	ProcessQueuedResolvedObjects();
}

void USpatialReceiver::OnAddEntity(Worker_AddEntityOp& Op)
{
	UE_LOG(LogSpatialReceiver, Verbose, TEXT("AddEntity: %lld"), Op.entity_id);

	check(bInCriticalSection);

	PendingAddEntities.Emplace(Op.entity_id);
}

void USpatialReceiver::OnAddComponent(Worker_AddComponentOp& Op)
{
	UE_LOG(LogSpatialReceiver, Verbose, TEXT("AddComponent component ID: %u entity ID: %lld"),
		Op.data.component_id, Op.entity_id);

	switch (Op.data.component_id)
	{
	case SpatialConstants::ENTITY_ACL_COMPONENT_ID:
	case SpatialConstants::METADATA_COMPONENT_ID:
	case SpatialConstants::POSITION_COMPONENT_ID:
	case SpatialConstants::PERSISTENCE_COMPONENT_ID:
	case SpatialConstants::SPAWN_DATA_COMPONENT_ID:
	case SpatialConstants::PLAYER_SPAWNER_COMPONENT_ID:
	case SpatialConstants::SINGLETON_COMPONENT_ID:
	case SpatialConstants::UNREAL_METADATA_COMPONENT_ID:
	case SpatialConstants::INTEREST_COMPONENT_ID:
	case SpatialConstants::NOT_STREAMED_COMPONENT_ID:
	case SpatialConstants::GSM_SHUTDOWN_COMPONENT_ID:
	case SpatialConstants::HEARTBEAT_COMPONENT_ID:
	case SpatialConstants::CLIENT_RPC_ENDPOINT_COMPONENT_ID:
	case SpatialConstants::SERVER_RPC_ENDPOINT_COMPONENT_ID:
	case SpatialConstants::NETMULTICAST_RPCS_COMPONENT_ID:
	case SpatialConstants::RPCS_ON_ENTITY_CREATION_ID:
		// Ignore static spatial components as they are managed by the SpatialStaticComponentView.
		return;
	case SpatialConstants::SINGLETON_MANAGER_COMPONENT_ID:
		GlobalStateManager->ApplySingletonManagerData(Op.data);
		GlobalStateManager->LinkAllExistingSingletonActors();
		return;
	case SpatialConstants::DEPLOYMENT_MAP_COMPONENT_ID:
		GlobalStateManager->ApplyDeploymentMapData(Op.data);
		return;
	case SpatialConstants::STARTUP_ACTOR_MANAGER_COMPONENT_ID:
		GlobalStateManager->ApplyStartupActorManagerData(Op.data);
		return;
	}

	if (ClassInfoManager->IsSublevelComponent(Op.data.component_id))
	{
		return;
	}

	// If a client gains ownership over something it had already checked out, it will
	// add component interest on the owner only data components, which will trigger an
	// AddComponentOp, but it is not guaranteed to be inside a critical section.
	if (!NetDriver->IsServer())
	{
		if (USpatialActorChannel* Channel = NetDriver->GetActorChannelByEntityId(Op.entity_id))
		{
			if (ClassInfoManager->GetCategoryByComponentId(Op.data.component_id) == SCHEMA_OwnerOnly)
			{
				// We received owner only data, and we have the entity checked out already,
				// so this happened as a result of adding component interest. Apply the data
				// immediately instead of queuing it up (since there will be no AddEntityOp).
				ApplyComponentData(Op.entity_id, Op.data, Channel);
				return;
			}
		}
	}

	if (!bInCriticalSection)
	{
		UE_LOG(LogSpatialReceiver, Warning, TEXT("Received a dynamically added component, these are currently unsupported - component ID: %u entity ID: %lld"),
			Op.data.component_id, Op.entity_id);
		return;
	}
	PendingAddComponents.Emplace(Op.entity_id, Op.data.component_id, MakeUnique<DynamicComponent>(Op.data));
}

void USpatialReceiver::OnRemoveEntity(Worker_RemoveEntityOp& Op)
{
	RemoveActor(Op.entity_id);
}

void USpatialReceiver::UpdateShadowData(Worker_EntityId EntityId)
{
	USpatialActorChannel* ActorChannel = NetDriver->GetActorChannelByEntityId(EntityId);
	ActorChannel->UpdateShadowData();
}

void USpatialReceiver::OnAuthorityChange(Worker_AuthorityChangeOp& Op)
{
	if (bInCriticalSection)
	{
		PendingAuthorityChanges.Add(Op);
		return;
	}

	HandleActorAuthority(Op);
}

void USpatialReceiver::HandlePlayerLifecycleAuthority(Worker_AuthorityChangeOp& Op, APlayerController* PlayerController)
{
	// Server initializes heartbeat logic based on its authority over the position component,
	// client does the same for heartbeat component
	if ((NetDriver->IsServer() && Op.component_id == SpatialConstants::POSITION_COMPONENT_ID) ||
		(!NetDriver->IsServer() && Op.component_id == SpatialConstants::HEARTBEAT_COMPONENT_ID))
	{
		if (Op.authority == WORKER_AUTHORITY_AUTHORITATIVE)
		{
			if (USpatialNetConnection* Connection = Cast<USpatialNetConnection>(PlayerController->GetNetConnection()))
			{
				Connection->InitHeartbeat(TimerManager, Op.entity_id);
			}
		}
		else if (Op.authority == WORKER_AUTHORITY_NOT_AUTHORITATIVE)
		{
			if (NetDriver->IsServer())
			{
				HeartbeatDelegates.Remove(Op.entity_id);
			}
			if (USpatialNetConnection* Connection = Cast<USpatialNetConnection>(PlayerController->GetNetConnection()))
			{
				Connection->DisableHeartbeat();
			}
		}
	}
}

void USpatialReceiver::HandleActorAuthority(Worker_AuthorityChangeOp& Op)
{
	if (Op.component_id == SpatialConstants::DEPLOYMENT_MAP_COMPONENT_ID)
	{
		GlobalStateManager->AuthorityChanged(Op.authority == WORKER_AUTHORITY_AUTHORITATIVE, Op.entity_id);
		return;
	}

	if (Op.component_id == SpatialConstants::SINGLETON_MANAGER_COMPONENT_ID
		&& Op.authority == WORKER_AUTHORITY_AUTHORITATIVE)
	{
		GlobalStateManager->ExecuteInitialSingletonActorReplication();
		return;
	}

	AActor* Actor = Cast<AActor>(NetDriver->PackageMap->GetObjectFromEntityId(Op.entity_id));
	if (Actor == nullptr)
	{
		return;
	}

	if (Op.component_id == SpatialConstants::CLIENT_RPC_ENDPOINT_COMPONENT_ID
		&& Op.authority == WORKER_AUTHORITY_AUTHORITATIVE)
	{
		check(!NetDriver->IsServer());
		if (RPCsOnEntityCreation* QueuedRPCs = StaticComponentView->GetComponentData<RPCsOnEntityCreation>(Op.entity_id))
		{
			if (QueuedRPCs->HasRPCPayloadData())
			{
				ProcessQueuedActorRPCsOnEntityCreation(Actor, *QueuedRPCs);
			}

			Sender->SendRequestToClearRPCsOnEntityCreation(Op.entity_id);
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
			Sender->ProcessUpdatesQueuedUntilAuthority(Op.entity_id);
		}

		// If we became authoritative over the position component. set our role to be ROLE_Authority
		// and set our RemoteRole to be ROLE_AutonomousProxy if the actor has an owning connection.
		if (Op.component_id == SpatialConstants::POSITION_COMPONENT_ID)
		{
			if (Op.authority == WORKER_AUTHORITY_AUTHORITATIVE)
			{
				if (IsValid(NetDriver->GetActorChannelByEntityId(Op.entity_id)))
				{
					Actor->Role = ROLE_Authority;
					Actor->RemoteRole = ROLE_SimulatedProxy;

					if (Actor->IsA<APlayerController>())
					{
						Actor->RemoteRole = ROLE_AutonomousProxy;
					}
					else if (APawn* Pawn = Cast<APawn>(Actor))
					{
						if (Pawn->IsPlayerControlled())
						{
							Pawn->RemoteRole = ROLE_AutonomousProxy;
						}
					}

					UpdateShadowData(Op.entity_id);

					Actor->OnAuthorityGained();
				}
				else
				{
					UE_LOG(LogSpatialReceiver, Verbose, TEXT("Received authority over actor %s, with entity id %lld, which has no channel. This means it attempted to delete it earlier, when it had no authority. Retrying to delete now."), *Actor->GetName(), Op.entity_id);
					Sender->SendDeleteEntityRequest(Op.entity_id);
				}
			}
			else if (Op.authority == WORKER_AUTHORITY_AUTHORITY_LOSS_IMMINENT)
			{
				Actor->OnAuthorityLossImminent();
			}
			else if (Op.authority == WORKER_AUTHORITY_NOT_AUTHORITATIVE)
			{
				if (USpatialActorChannel* ActorChannel = NetDriver->GetActorChannelByEntityId(Op.entity_id))
				{
					ActorChannel->bCreatedEntity = false;
				}

				Actor->Role = ROLE_SimulatedProxy;
				Actor->RemoteRole = ROLE_Authority;

				Actor->OnAuthorityLost();
			}
		}
	}
	else
	{
		// Check to see if we became authoritative over the UnrealClientRPCEndpoint component over this entity
		// If we did, our local role should be ROLE_AutonomousProxy. Otherwise ROLE_SimulatedProxy
		if ((Actor->IsA<APawn>() || Actor->IsA<APlayerController>()) && Op.component_id == SpatialConstants::CLIENT_RPC_ENDPOINT_COMPONENT_ID)
		{
			Actor->Role = (Op.authority == WORKER_AUTHORITY_AUTHORITATIVE) ? ROLE_AutonomousProxy : ROLE_SimulatedProxy;
		}
	}

	if (GetDefault<USpatialGDKSettings>()->bCheckRPCOrder && Op.authority == WORKER_AUTHORITY_AUTHORITATIVE)
	{
		ESchemaComponentType ComponentType = ClassInfoManager->GetCategoryByComponentId(Op.component_id);
		if (Op.component_id == SpatialConstants::CLIENT_RPC_ENDPOINT_COMPONENT_ID ||
			Op.component_id == SpatialConstants::SERVER_RPC_ENDPOINT_COMPONENT_ID ||
			Op.component_id == SpatialConstants::NETMULTICAST_RPCS_COMPONENT_ID)
		{
			// This will be called multiple times on each RPC component.
			NetDriver->OnRPCAuthorityGained(Actor, ComponentType);
		}
	}
}

void USpatialReceiver::ReceiveActor(Worker_EntityId EntityId)
{
	checkf(NetDriver, TEXT("We should have a NetDriver whilst processing ops."));
	checkf(NetDriver->GetWorld(), TEXT("We should have a World whilst processing ops."));

	SpawnData* SpawnDataComp = StaticComponentView->GetComponentData<SpawnData>(EntityId);
	UnrealMetadata* UnrealMetadataComp = StaticComponentView->GetComponentData<UnrealMetadata>(EntityId);

	if (UnrealMetadataComp == nullptr)
	{
		// Not an Unreal entity
		return;
	}

	// If the received actor is torn off, don't bother receiving it.
	// (This is only needed due to the delay between tearoff and deleting the entity. See https://improbableio.atlassian.net/browse/UNR-841)
	// Check the pending add components, to find the root component for the received entity.
	for (PendingAddComponentWrapper& PendingAddComponent : PendingAddComponents)
	{
		if (PendingAddComponent.EntityId != EntityId
			|| ClassInfoManager->GetCategoryByComponentId(PendingAddComponent.ComponentId) != SCHEMA_Data)
		{
			continue;
		}
		uint32 Offset = 0;
		if (!ClassInfoManager->GetOffsetByComponentId(PendingAddComponent.ComponentId, Offset) || Offset != 0)
		{
			continue;
		}

		Worker_ComponentData* ComponentData = PendingAddComponent.Data->ComponentData;
		Schema_Object* ComponentObject = Schema_GetComponentDataFields(ComponentData->schema_type);
		if (Schema_GetBool(ComponentObject, SpatialConstants::ACTOR_TEAROFF_ID))
		{
			UE_LOG(LogSpatialReceiver, Verbose, TEXT("The received actor with entity id %lld was already torn off. The actor will not be spawned."), EntityId);
			return;
		}
	}

	if (AActor* EntityActor = Cast<AActor>(PackageMap->GetObjectFromEntityId(EntityId)))
	{
		UE_LOG(LogSpatialReceiver, Log, TEXT("Entity for actor %s has been checked out on the worker which spawned it or is a singleton linked on this worker"), \
			*EntityActor->GetName());

		// Assume SimulatedProxy until we've been delegated Authority
		bool bAuthority = StaticComponentView->GetAuthority(EntityId, Position::ComponentId) == WORKER_AUTHORITY_AUTHORITATIVE;
		EntityActor->Role = bAuthority ? ROLE_Authority : ROLE_SimulatedProxy;
		EntityActor->RemoteRole = bAuthority ? ROLE_SimulatedProxy : ROLE_Authority;
		if (bAuthority)
		{
			if (EntityActor->GetNetConnection() != nullptr || EntityActor->IsA<APawn>())
			{
				EntityActor->RemoteRole = ROLE_AutonomousProxy;
			}
		}

		// If we're a singleton, apply the data, regardless of authority - JIRA: 736

		UE_LOG(LogSpatialReceiver, Log, TEXT("Received create entity response op for %lld"), EntityId);
	}
	else
	{
		EntityActor = TryGetOrCreateActor(UnrealMetadataComp, SpawnDataComp);

		if (EntityActor == nullptr)
		{
			// This could be nullptr if:
			// a stably named actor could not be found
			// the Actor is a singleton
			// the class couldn't be loaded
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
			UE_LOG(LogSpatialReceiver, Error, TEXT("Unable to find SpatialOSNetConnection! Has this worker been disconnected from SpatialOS due to a timeout?"));
			return;
		}

		// Set up actor channel.
#if ENGINE_MINOR_VERSION <= 20
		USpatialActorChannel* Channel = Cast<USpatialActorChannel>(Connection->CreateChannel(CHTYPE_Actor, NetDriver->IsServer()));
#else
		USpatialActorChannel* Channel = Cast<USpatialActorChannel>(Connection->CreateChannelByName(NAME_Actor, NetDriver->IsServer() ? EChannelCreateFlags::OpenedLocally : EChannelCreateFlags::None));
#endif

		if (!Channel)
		{
			UE_LOG(LogSpatialReceiver, Warning, TEXT("Failed to create an actor channel when receiving entity %lld. The actor will not be spawned."), EntityId);
			EntityActor->Destroy(true);
			return;
		}

		PackageMap->ResolveEntityActor(EntityActor, EntityId);

		Channel->SetChannelActor(EntityActor);

		// Apply initial replicated properties.
		// This was moved to after FinishingSpawning because components existing only in blueprints aren't added until spawning is complete
		// Potentially we could split out the initial actor state and the initial component state
		for (PendingAddComponentWrapper& PendingAddComponent : PendingAddComponents)
		{
			if (ClassInfoManager->IsSublevelComponent(PendingAddComponent.ComponentId))
			{
				continue;
			}

			if (PendingAddComponent.EntityId == EntityId)
			{
				ApplyComponentData(EntityId, *PendingAddComponent.Data->ComponentData, Channel);
			}
		}

		if (!NetDriver->IsServer())
		{
			// Update interest on the entity's components after receiving initial component data (so Role and RemoteRole are properly set).
			Sender->SendComponentInterest(EntityActor, EntityId, Channel->IsOwnedByWorker());

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

		// Taken from PostNetInit
		if (NetDriver->GetWorld()->HasBegunPlay() && !EntityActor->HasActorBegunPlay())
		{
			EntityActor->DispatchBeginPlay();
		}

		if (EntityActor->GetClass()->HasAnySpatialClassFlags(SPATIALCLASS_Singleton))
		{
			GlobalStateManager->RegisterSingletonChannel(EntityActor, Channel);
		}

		EntityActor->UpdateOverlaps();
	}
}

void USpatialReceiver::RemoveActor(Worker_EntityId EntityId)
{
	TWeakObjectPtr<UObject> WeakActor = PackageMap->GetObjectFromEntityId(EntityId);

	// Actor has been destroyed already. Clean up surrounding bookkeeping.
	if (!WeakActor.IsValid())
	{
		DestroyActor(nullptr, EntityId);
		return;
	}

	AActor* Actor = Cast<AActor>(WeakActor.Get());

	UE_LOG(LogSpatialReceiver, Log, TEXT("Worker %s Remove Actor: %s %lld"), *NetDriver->Connection->GetWorkerId(), Actor && !Actor->IsPendingKill() ? *Actor->GetName() : TEXT("nullptr"), EntityId);

	// Actor already deleted (this worker was most likely authoritative over it and deleted it earlier).
	if (Actor == nullptr || Actor->IsPendingKill())
	{
		if (USpatialActorChannel* ActorChannel = NetDriver->GetActorChannelByEntityId(EntityId))
		{
			UE_LOG(LogSpatialReceiver, Warning, TEXT("RemoveActor: actor for entity %lld was already deleted (likely on the authoritative worker) but still has an open actor channel."), EntityId);
#if ENGINE_MINOR_VERSION <= 20
			ActorChannel->ConditionalCleanUp();
#else
			ActorChannel->ConditionalCleanUp(false, EChannelCloseReason::Destroyed);
#endif
			CleanupDeletedEntity(EntityId);
		}
		return;
	}

	// If the entity is to be deleted after having been torn off, ignore the request (but clean up the channel if it has not been cleaned up already).
	if (Actor->GetTearOff())
	{
		if (USpatialActorChannel* ActorChannel = NetDriver->GetActorChannelByEntityId(EntityId))
		{
#if ENGINE_MINOR_VERSION <= 20
			ActorChannel->ConditionalCleanUp();
#else
			ActorChannel->ConditionalCleanUp(false, EChannelCloseReason::TearOff);
#endif
			CleanupDeletedEntity(EntityId);
		}
		return;
	}

	// Actor is a startup actor that is a part of the level. We need to do an entity query to see
	// if the entity was actually deleted or only removed from our view
	if (Actor->IsFullNameStableForNetworking())
	{
		QueryForStartupActor(Actor, EntityId);
		return;
	}

	if (APlayerController* PC = Cast<APlayerController>(Actor))
	{
		// Force APlayerController::DestroyNetworkActorHandled to return false
		PC->Player = nullptr;
	}

	// Workaround for camera loss on handover: prevent UnPossess() (non-authoritative destruction of pawn, while being authoritative over the controller)
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

	if (Actor->GetClass()->HasAnySpatialClassFlags(SPATIALCLASS_Singleton))
	{
		return;
	}

	DestroyActor(Actor, EntityId);
}

void USpatialReceiver::QueryForStartupActor(AActor* Actor, Worker_EntityId EntityId)
{
	Worker_EntityIdConstraint StartupActorConstraintEntityId;
	StartupActorConstraintEntityId.entity_id = EntityId;

	Worker_Constraint StartupActorConstraint{};
	StartupActorConstraint.constraint_type = WORKER_CONSTRAINT_TYPE_ENTITY_ID;
	StartupActorConstraint.entity_id_constraint = StartupActorConstraintEntityId;

	Worker_EntityQuery StartupActorQuery{};
	StartupActorQuery.constraint = StartupActorConstraint;
	StartupActorQuery.result_type = WORKER_RESULT_TYPE_COUNT;

	Worker_RequestId RequestID;
	RequestID = NetDriver->Connection->SendEntityQueryRequest(&StartupActorQuery);

	EntityQueryDelegate StartupActorDelegate;
	TWeakObjectPtr<AActor> WeakActor(Actor);
	StartupActorDelegate.BindLambda([this, WeakActor, EntityId](Worker_EntityQueryResponseOp& Op)
	{
		if (Op.status_code != WORKER_STATUS_CODE_SUCCESS)
		{
			UE_LOG(LogSpatialReceiver, Warning, TEXT("Entity Query Failed! %s"), Op.message);
			return;
		}

		if (Op.result_count == 0 && WeakActor.IsValid())
		{
			DestroyActor(WeakActor.Get(), EntityId);
		}
	});

	AddEntityQueryDelegate(RequestID, StartupActorDelegate);
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

#if ENGINE_MINOR_VERSION <= 20
		ActorChannel->ConditionalCleanUp();
#else
		ActorChannel->ConditionalCleanUp(false, EChannelCloseReason::Destroyed);
#endif
	}
	else
	{
		if (Actor == nullptr)
		{
			UE_LOG(LogSpatialReceiver, Verbose, TEXT("Removing actor as a result of a remove entity op, which has a missing actor channel. EntityId: %lld"), EntityId);
		}
		else
		{
			UE_LOG(LogSpatialReceiver, Warning, TEXT("Removing actor as a result of a remove entity op, which has a missing actor channel. Actor: %s EntityId: %lld"), *Actor->GetName(), EntityId);
		}
	}

	// It is safe to call AActor::Destroy even if the destruction has already started.
	if (Actor != nullptr && !Actor->Destroy(true))
	{
		UE_LOG(LogSpatialReceiver, Error, TEXT("Failed to destroy actor in RemoveActor %s %lld"), *Actor->GetName(), EntityId);
	}
	NetDriver->StopIgnoringAuthoritativeDestruction();

	CleanupDeletedEntity(EntityId);

	StaticComponentView->OnRemoveEntity(EntityId);
}

void USpatialReceiver::CleanupDeletedEntity(Worker_EntityId EntityId)
{
	PackageMap->RemoveEntityActor(EntityId);
	NetDriver->RemoveActorChannel(EntityId);
}

AActor* USpatialReceiver::TryGetOrCreateActor(UnrealMetadata* UnrealMetadataComp, SpawnData* SpawnDataComp)
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

	return CreateActor(UnrealMetadataComp, SpawnDataComp);
}

// This function is only called for client and server workers who did not spawn the Actor
AActor* USpatialReceiver::CreateActor(UnrealMetadata* UnrealMetadataComp, SpawnData* SpawnDataComp)
{
	UClass* ActorClass = UnrealMetadataComp->GetNativeEntityClass();

	if (ActorClass == nullptr)
	{
		UE_LOG(LogSpatialReceiver, Error, TEXT("Could not load class %s when spawning entity!"), *UnrealMetadataComp->ClassPath);
		return nullptr;
	}

	// Initial Singleton Actor replication is handled with GlobalStateManager::LinkExistingSingletonActors
	if (NetDriver->IsServer() && ActorClass->HasAnySpatialClassFlags(SPATIALCLASS_Singleton))
	{
		return FindSingletonActor(ActorClass);
	}

	// If we're checking out a player controller, spawn it via "USpatialNetDriver::AcceptNewPlayer"
	if (NetDriver->IsServer() && ActorClass->IsChildOf(APlayerController::StaticClass()))
	{
		checkf(!UnrealMetadataComp->OwnerWorkerAttribute.IsEmpty(), TEXT("A player controller entity must have an owner worker attribute."));

		FString URLString = FURL().ToString();
		URLString += TEXT("?workerAttribute=") + UnrealMetadataComp->OwnerWorkerAttribute;

		// TODO: Once we can checkout PlayerController and PlayerState atomically, we can grab the UniqueId and online subsystem type from PlayerState. UNR-933
		UNetConnection* Connection = NetDriver->AcceptNewPlayer(FURL(nullptr, *URLString, TRAVEL_Absolute), FUniqueNetIdRepl(), FName(), true);
		check(Connection);

		return Connection->PlayerController;
	}

	UE_LOG(LogSpatialReceiver, Verbose, TEXT("Spawning a %s whilst checking out an entity."), *ActorClass->GetFullName());

	FActorSpawnParameters SpawnInfo;
	SpawnInfo.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	SpawnInfo.bRemoteOwned = true;
	SpawnInfo.bNoFail = true;

	FVector SpawnLocation = FRepMovement::RebaseOntoLocalOrigin(SpawnDataComp->Location, NetDriver->GetWorld()->OriginLocation);

	AActor* NewActor = NetDriver->GetWorld()->SpawnActorAbsolute(ActorClass, FTransform(SpawnDataComp->Rotation, SpawnLocation), SpawnInfo);
	check(NewActor);

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

void USpatialReceiver::ApplyComponentData(Worker_EntityId EntityId, Worker_ComponentData& Data, USpatialActorChannel* Channel)
{
	uint32 Offset = 0;
	bool bFoundOffset = ClassInfoManager->GetOffsetByComponentId(Data.component_id, Offset);
	if (!bFoundOffset)
	{
		UE_LOG(LogSpatialReceiver, Warning, TEXT("EntityId %lld, ComponentId %d - Could not find offset for component id when applying component data to Actor %s!"), EntityId, Data.component_id, *Channel->GetActor()->GetName());
		return;
	}

	TWeakObjectPtr<UObject> TargetObject = PackageMap->GetObjectFromUnrealObjectRef(FUnrealObjectRef(EntityId, Offset));
	if (!TargetObject.IsValid())
	{
		UE_LOG(LogSpatialReceiver, Warning, TEXT("EntityId %lld, ComponentId %d, Offset %d - Could not find target object with given offset for Actor %s!"), EntityId, Data.component_id, Offset, *Channel->GetActor()->GetName());
		return;
	}

	UClass* Class = ClassInfoManager->GetClassByComponentId(Data.component_id);
	checkf(Class, TEXT("Component %d isn't hand-written and not present in ComponentToClassMap."), Data.component_id);

	FChannelObjectPair ChannelObjectPair(Channel, TargetObject);

	ESchemaComponentType ComponentType = ClassInfoManager->GetCategoryByComponentId(Data.component_id);

	if (ComponentType == SCHEMA_Data || ComponentType == SCHEMA_OwnerOnly)
	{
		if (ComponentType == SCHEMA_Data && TargetObject->IsA<UActorComponent>())
		{
			Schema_Object* ComponentObject = Schema_GetComponentDataFields(Data.schema_type);
			bool bReplicates = !!Schema_IndexBool(ComponentObject, SpatialConstants::ACTOR_COMPONENT_REPLICATES_ID, 0);
			if (!bReplicates)
			{
				return;
			}
		}

		FObjectReferencesMap& ObjectReferencesMap = UnresolvedRefsMap.FindOrAdd(ChannelObjectPair);
		TSet<FUnrealObjectRef> UnresolvedRefs;

		ComponentReader Reader(NetDriver, ObjectReferencesMap, UnresolvedRefs);
		Reader.ApplyComponentData(Data, TargetObject.Get(), Channel, /* bIsHandover */ false);

		QueueIncomingRepUpdates(ChannelObjectPair, ObjectReferencesMap, UnresolvedRefs);
	}
	else if (ComponentType == SCHEMA_Handover)
	{
		FObjectReferencesMap& ObjectReferencesMap = UnresolvedRefsMap.FindOrAdd(ChannelObjectPair);
		TSet<FUnrealObjectRef> UnresolvedRefs;

		ComponentReader Reader(NetDriver, ObjectReferencesMap, UnresolvedRefs);
		Reader.ApplyComponentData(Data, TargetObject.Get(), Channel, /* bIsHandover */ true);

		QueueIncomingRepUpdates(ChannelObjectPair, ObjectReferencesMap, UnresolvedRefs);
	}
	else
	{
		UE_LOG(LogSpatialReceiver, Verbose, TEXT("Entity: %d Component: %d - Skipping because RPC components don't have actual data."), EntityId, Data.component_id);
	}
}

void USpatialReceiver::OnComponentUpdate(Worker_ComponentUpdateOp& Op)
{
	if (StaticComponentView->GetAuthority(Op.entity_id, Op.update.component_id) == WORKER_AUTHORITY_AUTHORITATIVE)
	{
		UE_LOG(LogSpatialReceiver, Verbose, TEXT("Entity: %d Component: %d - Skipping update because this was short circuited"), Op.entity_id, Op.update.component_id);
		return;
	}

	switch (Op.update.component_id)
	{
	case SpatialConstants::ENTITY_ACL_COMPONENT_ID:
	case SpatialConstants::METADATA_COMPONENT_ID:
	case SpatialConstants::POSITION_COMPONENT_ID:
	case SpatialConstants::PERSISTENCE_COMPONENT_ID:
	case SpatialConstants::INTEREST_COMPONENT_ID:
	case SpatialConstants::SPAWN_DATA_COMPONENT_ID:
	case SpatialConstants::PLAYER_SPAWNER_COMPONENT_ID:
	case SpatialConstants::SINGLETON_COMPONENT_ID:
	case SpatialConstants::UNREAL_METADATA_COMPONENT_ID:
	case SpatialConstants::NOT_STREAMED_COMPONENT_ID:
	case SpatialConstants::RPCS_ON_ENTITY_CREATION_ID:
		UE_LOG(LogSpatialReceiver, Verbose, TEXT("Entity: %d Component: %d - Skipping because this is hand-written Spatial component"), Op.entity_id, Op.update.component_id);
		return;
	case SpatialConstants::GSM_SHUTDOWN_COMPONENT_ID:
#if WITH_EDITOR
		GlobalStateManager->OnShutdownComponentUpdate(Op.update);
#endif // WITH_EDITOR
		return;
	case SpatialConstants::HEARTBEAT_COMPONENT_ID:
		if (HeartbeatDelegate* UpdateDelegate = HeartbeatDelegates.Find(Op.entity_id))
		{
			UpdateDelegate->ExecuteIfBound(Op);
		}
		return;
	case SpatialConstants::SINGLETON_MANAGER_COMPONENT_ID:
		GlobalStateManager->ApplySingletonManagerUpdate(Op.update);
		GlobalStateManager->LinkAllExistingSingletonActors();
		return;
	case SpatialConstants::DEPLOYMENT_MAP_COMPONENT_ID:
		NetDriver->GlobalStateManager->ApplyDeploymentMapUpdate(Op.update);
		return;
	case SpatialConstants::STARTUP_ACTOR_MANAGER_COMPONENT_ID:
		NetDriver->GlobalStateManager->ApplyStartupActorManagerUpdate(Op.update);
		return;
	case SpatialConstants::CLIENT_RPC_ENDPOINT_COMPONENT_ID:
	case SpatialConstants::SERVER_RPC_ENDPOINT_COMPONENT_ID:
	case SpatialConstants::NETMULTICAST_RPCS_COMPONENT_ID:
		HandleUnreliableRPC(Op);
		return;
	}

	if (ClassInfoManager->IsSublevelComponent(Op.update.component_id))
	{
		return;
	}

	USpatialActorChannel* Channel = NetDriver->GetActorChannelByEntityId(Op.entity_id);
	if (Channel == nullptr)
	{
		UE_LOG(LogSpatialReceiver, Verbose, TEXT("Worker: %s Entity: %d Component: %d - No actor channel for update. This most likely occured due to the component updates that are sent when authority is lost during entity deletion."), *NetDriver->Connection->GetWorkerId(), Op.entity_id, Op.update.component_id);
		return;
	}

	const FClassInfo& Info = ClassInfoManager->GetClassInfoByComponentId(Op.update.component_id);

	uint32 Offset;
	bool bFoundOffset = ClassInfoManager->GetOffsetByComponentId(Op.update.component_id, Offset);
	if (!bFoundOffset)
	{
		UE_LOG(LogSpatialReceiver, Warning, TEXT("Entity: %d Component: %d - Couldn't find Offset for component id"), Op.entity_id, Op.update.component_id);
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
		UE_LOG(LogSpatialReceiver, Warning, TEXT("Entity: %d Component: %d - Couldn't find target object for update"), Op.entity_id, Op.update.component_id);
		return;
	}

	ESchemaComponentType Category = ClassInfoManager->GetCategoryByComponentId(Op.update.component_id);

	if (Category == ESchemaComponentType::SCHEMA_Data || Category == ESchemaComponentType::SCHEMA_OwnerOnly)
	{
		ApplyComponentUpdate(Op.update, TargetObject, Channel, /* bIsHandover */ false);
	}
	else if (Category == ESchemaComponentType::SCHEMA_Handover)
	{
		if (!NetDriver->IsServer())
		{
			UE_LOG(LogSpatialReceiver, Verbose, TEXT("Entity: %d Component: %d - Skipping Handover component because we're a client."), Op.entity_id, Op.update.component_id);
			return;
		}

		ApplyComponentUpdate(Op.update, TargetObject, Channel, /* bIsHandover */ true);
	}
	else
	{
		UE_LOG(LogSpatialReceiver, Verbose, TEXT("Entity: %d Component: %d - Skipping because it's an empty component update from an RPC component. (most likely as a result of gaining authority)"), Op.entity_id, Op.update.component_id);
	}
}

void USpatialReceiver::HandleUnreliableRPC(Worker_ComponentUpdateOp& Op)
{
	Worker_EntityId EntityId = Op.entity_id;

	// Multicast RPCs should be executed by whoever receives them.
	if (Op.update.component_id != SpatialConstants::NETMULTICAST_RPCS_COMPONENT_ID)
	{
		// If the update is to the client rpc endpoint, then the handler should have authority over the server rpc endpoint component and vice versa
		// Ideally these events are never delivered to workers which are not able to handle them with clever interest management
		const Worker_ComponentId RPCEndpointComponentId = Op.update.component_id == SpatialConstants::CLIENT_RPC_ENDPOINT_COMPONENT_ID
			? SpatialConstants::SERVER_RPC_ENDPOINT_COMPONENT_ID : SpatialConstants::CLIENT_RPC_ENDPOINT_COMPONENT_ID;

		if (StaticComponentView->GetAuthority(Op.entity_id, RPCEndpointComponentId) != WORKER_AUTHORITY_AUTHORITATIVE)
		{
			return;
		}
	}

	Schema_Object* EventsObject = Schema_GetComponentUpdateEvents(Op.update.schema_type);

	uint32 EventCount = Schema_GetObjectCount(EventsObject, SpatialConstants::UNREAL_RPC_ENDPOINT_EVENT_ID);

	for (uint32 i = 0; i < EventCount; i++)
	{
		Schema_Object* EventData = Schema_IndexObject(EventsObject, SpatialConstants::UNREAL_RPC_ENDPOINT_EVENT_ID, i);

		RPCPayload Payload(EventData);

		FUnrealObjectRef ObjectRef(EntityId, Payload.Offset);

		UObject* TargetObject = PackageMap->GetObjectFromUnrealObjectRef(ObjectRef).Get();

		if (!TargetObject)
		{
			UE_LOG(LogSpatialReceiver, Warning, TEXT("HandleUnreliableRPC: Could not find target object: %s, skipping rpc at index: %d"), *ObjectRef.ToString(), Payload.Index);
			continue;
		}

		const FClassInfo& ClassInfo = ClassInfoManager->GetOrCreateClassInfoByObject(TargetObject);

		UFunction* Function = ClassInfo.RPCs[Payload.Index];
		ApplyRPC(TargetObject, Function, Payload, FString());
	}
}

void USpatialReceiver::OnCommandRequest(Worker_CommandRequestOp& Op)
{
	Schema_FieldId CommandIndex = Schema_GetCommandRequestCommandIndex(Op.request.schema_type);

	if (Op.request.component_id == SpatialConstants::PLAYER_SPAWNER_COMPONENT_ID && CommandIndex == SpatialConstants::PLAYER_SPAWNER_SPAWN_PLAYER_COMMAND_ID)
	{
		Schema_Object* Payload = Schema_GetCommandRequestObject(Op.request.schema_type);

		// Op.caller_attribute_set has two attributes.
		// 1. The attribute of the worker type
		// 2. The attribute of the specific worker that sent the request
		// We want to give authority to the specific worker, so we grab the second element from the attribute set.
		NetDriver->PlayerSpawner->ReceivePlayerSpawnRequest(Payload, Op.caller_attribute_set.attributes[1], Op.request_id);
		return;
	}
	else if (Op.request.component_id == SpatialConstants::RPCS_ON_ENTITY_CREATION_ID && CommandIndex == SpatialConstants::CLEAR_RPCS_ON_ENTITY_CREATION)
	{
		Sender->ClearRPCsOnEntityCreation(Op.entity_id);
		return;
	}
#if WITH_EDITOR
	else if (Op.request.component_id == SpatialConstants::GSM_SHUTDOWN_COMPONENT_ID && CommandIndex == SpatialConstants::SHUTDOWN_MULTI_PROCESS_REQUEST_ID)
	{
		NetDriver->GlobalStateManager->ReceiveShutdownMultiProcessRequest();
		return;
	}
#endif // WITH_EDITOR

	Worker_CommandResponse Response = {};
	Response.component_id = Op.request.component_id;
	Response.schema_type = Schema_CreateCommandResponse(Op.request.component_id, CommandIndex);

	Schema_Object* RequestObject = Schema_GetCommandRequestObject(Op.request.schema_type);

	RPCPayload Payload(RequestObject);

	UObject* TargetObject = PackageMap->GetObjectFromUnrealObjectRef(FUnrealObjectRef(Op.entity_id, Payload.Offset)).Get();
	if (TargetObject == nullptr)
	{
		UE_LOG(LogSpatialReceiver, Warning, TEXT("No target object found for EntityId %d"), Op.entity_id);
		Sender->SendCommandResponse(Op.request_id, Response);
		return;
	}

	const FClassInfo& Info = ClassInfoManager->GetOrCreateClassInfoByObject(TargetObject);

	UFunction* Function = Info.RPCs[Payload.Index];

	UE_LOG(LogSpatialReceiver, Verbose, TEXT("Received command request (entity: %lld, component: %d, function: %s)"),
		Op.entity_id, Op.request.component_id, *Function->GetName());

	ApplyRPC(TargetObject, Function, Payload, UTF8_TO_TCHAR(Op.caller_worker_id));

	Sender->SendCommandResponse(Op.request_id, Response);
}

void USpatialReceiver::OnCommandResponse(Worker_CommandResponseOp& Op)
{
	if (Op.response.component_id == SpatialConstants::PLAYER_SPAWNER_COMPONENT_ID)
	{
		NetDriver->PlayerSpawner->ReceivePlayerSpawnResponse(Op);
		return;
	}

	ReceiveCommandResponse(Op);
}

void USpatialReceiver::FlushRetryRPCs()
{
	Sender->FlushRetryRPCs();
}

void USpatialReceiver::ReceiveCommandResponse(Worker_CommandResponseOp& Op)
{
	TSharedRef<FReliableRPCForRetry>* ReliableRPCPtr = PendingReliableRPCs.Find(Op.request_id);
	if (ReliableRPCPtr == nullptr)
	{
		// We received a response for some other command, ignore.
		return;
	}

	TSharedRef<FReliableRPCForRetry> ReliableRPC = *ReliableRPCPtr;
	PendingReliableRPCs.Remove(Op.request_id);
	if (Op.status_code != WORKER_STATUS_CODE_SUCCESS)
	{
		bool bCanRetry = false;

		// Only attempt to retry if the error code indicates it makes sense too
		if ((Op.status_code == WORKER_STATUS_CODE_TIMEOUT || Op.status_code == WORKER_STATUS_CODE_NOT_FOUND)
			&& (ReliableRPC->Attempts < SpatialConstants::MAX_NUMBER_COMMAND_ATTEMPTS))
		{
			bCanRetry = true;
		}
		// Don't apply the retry limit on auth lost, as it should eventually succeed
		else if (Op.status_code == WORKER_STATUS_CODE_AUTHORITY_LOST)
		{
			bCanRetry = true;
		}

		if (bCanRetry)
		{
			float WaitTime = SpatialConstants::GetCommandRetryWaitTimeSeconds(ReliableRPC->Attempts);
			UE_LOG(LogSpatialReceiver, Log, TEXT("%s: retrying in %f seconds. Error code: %d Message: %s"),
				*ReliableRPC->Function->GetName(), WaitTime, (int)Op.status_code, UTF8_TO_TCHAR(Op.message));

			if (!ReliableRPC->TargetObject.IsValid())
			{
				UE_LOG(LogSpatialReceiver, Warning, TEXT("%s: target object was destroyed before we could deliver the RPC."),
					*ReliableRPC->Function->GetName());
				return;
			}

			// Queue retry
			FTimerHandle RetryTimer;
			TimerManager->SetTimer(RetryTimer, [this, ReliableRPC]()
			{
				Sender->EnqueueRetryRPC(ReliableRPC);
			}, WaitTime, false);
		}
		else
		{
			UE_LOG(LogSpatialReceiver, Error, TEXT("%s: failed too many times, giving up (%u attempts). Error code: %d Message: %s"),
				*ReliableRPC->Function->GetName(), SpatialConstants::MAX_NUMBER_COMMAND_ATTEMPTS, (int)Op.status_code, UTF8_TO_TCHAR(Op.message));
		}
	}
}

void USpatialReceiver::ApplyComponentUpdate(const Worker_ComponentUpdate& ComponentUpdate, UObject* TargetObject, USpatialActorChannel* Channel, bool bIsHandover)
{
	FChannelObjectPair ChannelObjectPair(Channel, TargetObject);

	FObjectReferencesMap& ObjectReferencesMap = UnresolvedRefsMap.FindOrAdd(ChannelObjectPair);
	TSet<FUnrealObjectRef> UnresolvedRefs;
	ComponentReader Reader(NetDriver, ObjectReferencesMap, UnresolvedRefs);
	Reader.ApplyComponentUpdate(ComponentUpdate, TargetObject, Channel, bIsHandover);

	// This is a temporary workaround, see UNR-841:
	// If the update includes tearoff, close the channel and clean up the entity.
	if (TargetObject->IsA<AActor>() && ClassInfoManager->GetCategoryByComponentId(ComponentUpdate.component_id) == SCHEMA_Data)
	{
		Schema_Object* ComponentObject = Schema_GetComponentUpdateFields(ComponentUpdate.schema_type);

		// Check if bTearOff has been set to true
		if (Schema_GetBool(ComponentObject, SpatialConstants::ACTOR_TEAROFF_ID))
		{
#if ENGINE_MINOR_VERSION <= 20
			Channel->ConditionalCleanUp();
#else
			Channel->ConditionalCleanUp(false, EChannelCloseReason::TearOff);
#endif
			CleanupDeletedEntity(Channel->GetEntityId());
		}
	}

	QueueIncomingRepUpdates(ChannelObjectPair, ObjectReferencesMap, UnresolvedRefs);
}

void USpatialReceiver::ApplyRPC(UObject* TargetObject, UFunction* Function, RPCPayload& Payload, const FString& SenderWorkerId)
{
	uint8* Parms = (uint8*)FMemory_Alloca(Function->ParmsSize);
	FMemory::Memzero(Parms, Function->ParmsSize);

	TSet<FUnrealObjectRef> UnresolvedRefs;

	FSpatialNetBitReader PayloadReader(PackageMap, Payload.PayloadData.GetData(), Payload.CountDataBits(), UnresolvedRefs);

	int ReliableRPCId = 0;
	if (GetDefault<USpatialGDKSettings>()->bCheckRPCOrder)
	{
		if (Function->HasAnyFunctionFlags(FUNC_NetReliable) && !Function->HasAnyFunctionFlags(FUNC_NetMulticast))
		{
			PayloadReader << ReliableRPCId;
		}
	}

	TSharedPtr<FRepLayout> RepLayout = NetDriver->GetFunctionRepLayout(Function);
	RepLayout_ReceivePropertiesForRPC(*RepLayout, PayloadReader, Parms);

	if (UnresolvedRefs.Num() == 0)
	{
		if (GetDefault<USpatialGDKSettings>()->bCheckRPCOrder)
		{
			if (Function->HasAnyFunctionFlags(FUNC_NetReliable) && !Function->HasAnyFunctionFlags(FUNC_NetMulticast))
			{
				AActor* Actor = Cast<AActor>(TargetObject);
				if (Actor == nullptr)
				{
					Actor = Cast<AActor>(TargetObject->GetOuter());
					check(Actor);
				}
				NetDriver->OnReceivedReliableRPC(Actor, FunctionFlagsToRPCSchemaType(Function->FunctionFlags), SenderWorkerId, ReliableRPCId, TargetObject, Function);
			}
		}

		TargetObject->ProcessEvent(Function, Parms);
	}
	else
	{
		QueueIncomingRPC(UnresolvedRefs, TargetObject, Function, Payload, SenderWorkerId);
	}

	// Destroy the parameters.
	// warning: highly dependent on UObject::ProcessEvent freeing of parms!
	for (TFieldIterator<UProperty> It(Function); It && It->HasAnyPropertyFlags(CPF_Parm); ++It)
	{
		It->DestroyValue_InContainer(Parms);
	}
}

void USpatialReceiver::OnReserveEntityIdsResponse(Worker_ReserveEntityIdsResponseOp& Op)
{
	if (Op.status_code == WORKER_STATUS_CODE_SUCCESS)
	{
		if (ReserveEntityIDsDelegate* RequestDelegate = ReserveEntityIDsDelegates.Find(Op.request_id))
		{
			UE_LOG(LogSpatialReceiver, Log, TEXT("Executing ReserveEntityIdsResponse with delegate, request id: %d, first entity id: %lld, message: %s"), Op.request_id, Op.first_entity_id, UTF8_TO_TCHAR(Op.message));
			RequestDelegate->ExecuteIfBound(Op);
			ReserveEntityIDsDelegates.Remove(Op.request_id);
		}
		else
		{
			UE_LOG(LogSpatialReceiver, Warning, TEXT("Recieved ReserveEntityIdsResponse but with no delegate set, request id: %d, first entity id: %lld, message: %s"), Op.request_id, Op.first_entity_id, UTF8_TO_TCHAR(Op.message));
		}
	}
	else
	{
		UE_LOG(LogSpatialReceiver, Error, TEXT("Failed ReserveEntityIds: request id: %d, message: %s"), Op.request_id, UTF8_TO_TCHAR(Op.message));
	}
}

void USpatialReceiver::OnCreateEntityResponse(Worker_CreateEntityResponseOp& Op)
{
	if (Op.status_code != WORKER_STATUS_CODE_SUCCESS)
	{
		UE_LOG(LogSpatialReceiver, Error, TEXT("Create entity request failed: request id: %d, entity id: %lld, message: %s"), Op.request_id, Op.entity_id, UTF8_TO_TCHAR(Op.message));
	}
	else
	{
		UE_LOG(LogSpatialReceiver, Verbose, TEXT("Create entity request succeeded: request id: %d, entity id: %lld, message: %s"), Op.request_id, Op.entity_id, UTF8_TO_TCHAR(Op.message));
	}

	TWeakObjectPtr<USpatialActorChannel> Channel = PopPendingActorRequest(Op.request_id);

	// It's possible for the ActorChannel to have been closed by the time we receive a response. Actor validity is checked within the channel.
	if (Channel.IsValid())
	{
		Channel->OnCreateEntityResponse(Op);
	}
	else
	{
		UE_LOG(LogSpatialReceiver, Verbose, TEXT("Received CreateEntityResponse for actor which no longer has an actor channel: request id: %d, entity id: %lld. This should only happen in the case where we attempt to delete the entity before we have authority. The entity will therefore be deleted once authority is gained."), Op.request_id, Op.entity_id);
	}
}

void USpatialReceiver::OnEntityQueryResponse(Worker_EntityQueryResponseOp& Op)
{
	if (Op.status_code == WORKER_STATUS_CODE_SUCCESS)
	{
		auto RequestDelegate = EntityQueryDelegates.Find(Op.request_id);
		if (RequestDelegate)
		{
			UE_LOG(LogSpatialReceiver, Log, TEXT("Executing EntityQueryResponse with delegate, request id: %d, number of entities: %d, message: %s"), Op.request_id, Op.result_count, UTF8_TO_TCHAR(Op.message));
			RequestDelegate->ExecuteIfBound(Op);
		}
		else
		{
			UE_LOG(LogSpatialReceiver, Warning, TEXT("Recieved EntityQueryResponse but with no delegate set, request id: %d, number of entities: %d, message: %s"), Op.request_id, Op.result_count, UTF8_TO_TCHAR(Op.message));
		}
	}
	else
	{
		UE_LOG(LogSpatialReceiver, Error, TEXT("EntityQuery failed: request id: %d, message: %s"), Op.request_id, UTF8_TO_TCHAR(Op.message));
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
	EntityQueryDelegates.Add(RequestId, Delegate);
}

void USpatialReceiver::AddReserveEntityIdsDelegate(Worker_RequestId RequestId, ReserveEntityIDsDelegate Delegate)
{
	ReserveEntityIDsDelegates.Add(RequestId, Delegate);
}

void USpatialReceiver::AddHeartbeatDelegate(Worker_EntityId EntityId, HeartbeatDelegate Delegate)
{
	HeartbeatDelegates.Add(EntityId, Delegate);
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

AActor* USpatialReceiver::FindSingletonActor(UClass* SingletonClass)
{
	TArray<AActor*> FoundActors;
	UGameplayStatics::GetAllActorsOfClass(NetDriver->World, SingletonClass, FoundActors);

	// There should be only one singleton actor per class
	if (FoundActors.Num() == 1)
	{
		return FoundActors[0];
	}
	else
	{
		UE_LOG(LogSpatialReceiver, Error, TEXT("Found incorrect number (%d) of singleton actors (%s)"),
			FoundActors.Num(), *SingletonClass->GetName());
	}

	return nullptr;
}

void USpatialReceiver::ProcessQueuedResolvedObjects()
{
	for (TPair<UObject*, FUnrealObjectRef>& It : ResolvedObjectQueue)
	{
		ResolvePendingOperations_Internal(It.Key, It.Value);
	}
	ResolvedObjectQueue.Empty();
}

void USpatialReceiver::ProcessQueuedActorRPCsOnEntityCreation(AActor* Actor, RPCsOnEntityCreation& QueuedRPCs)
{
	const FClassInfo& Info = ClassInfoManager->GetOrCreateClassInfoByClass(Actor->GetClass());

	for (auto& RPC : QueuedRPCs.RPCs)
	{
		UFunction* Function = Info.RPCs[RPC.Index];
		ApplyRPC(Actor, Function, RPC, FString());
	}
}

void USpatialReceiver::ResolvePendingOperations(UObject* Object, const FUnrealObjectRef& ObjectRef)
{
	if (bInCriticalSection)
	{
		ResolvedObjectQueue.Add(TPair<UObject*, FUnrealObjectRef>{ Object, ObjectRef });
	}
	else
	{
		ResolvePendingOperations_Internal(Object, ObjectRef);
	}
}

void USpatialReceiver::OnDisconnect(Worker_DisconnectOp& Op)
{
	if (GEngine != nullptr)
	{
		GEngine->BroadcastNetworkFailure(NetDriver->GetWorld(), NetDriver, ENetworkFailure::FromDisconnectOpStatusCode(Op.connection_status_code), UTF8_TO_TCHAR(Op.reason));
	}
}

void USpatialReceiver::QueueIncomingRepUpdates(FChannelObjectPair ChannelObjectPair, const FObjectReferencesMap& ObjectReferencesMap, const TSet<FUnrealObjectRef>& UnresolvedRefs)
{
	for (const FUnrealObjectRef& UnresolvedRef : UnresolvedRefs)
	{
		UE_LOG(LogSpatialReceiver, Log, TEXT("Added pending incoming property for object ref: %s, target object: %s"), *UnresolvedRef.ToString(), *ChannelObjectPair.Value->GetName());
		IncomingRefsMap.FindOrAdd(UnresolvedRef).Add(ChannelObjectPair);
	}

	if (ObjectReferencesMap.Num() == 0)
	{
		UnresolvedRefsMap.Remove(ChannelObjectPair);
	}
}

void USpatialReceiver::QueueIncomingRPC(const TSet<FUnrealObjectRef>& UnresolvedRefs, UObject* TargetObject, UFunction* Function, RPCPayload& Payload, const FString& SenderWorkerId)
{
	TSharedPtr<FPendingIncomingRPC> IncomingRPC = MakeShared<FPendingIncomingRPC>(UnresolvedRefs, TargetObject, Function, Payload);
	IncomingRPC->SenderWorkerId = SenderWorkerId;

	for (const FUnrealObjectRef& UnresolvedRef : UnresolvedRefs)
	{
		FIncomingRPCArray& IncomingRPCArray = IncomingRPCMap.FindOrAdd(UnresolvedRef);
		IncomingRPCArray.Add(IncomingRPC);
	}
}

void USpatialReceiver::ResolvePendingOperations_Internal(UObject* Object, const FUnrealObjectRef& ObjectRef)
{
	UE_LOG(LogSpatialReceiver, Verbose, TEXT("Resolving pending object refs and RPCs which depend on object: %s %s."), *Object->GetName(), *ObjectRef.ToString());

	Sender->ResolveOutgoingOperations(Object, /* bIsHandover */ false);
	Sender->ResolveOutgoingOperations(Object, /* bIsHandover */ true);
	ResolveIncomingOperations(Object, ObjectRef);
	Sender->ResolveOutgoingRPCs(Object);
	ResolveIncomingRPCs(Object, ObjectRef);
}

void USpatialReceiver::ResolveIncomingOperations(UObject* Object, const FUnrealObjectRef& ObjectRef)
{
	// TODO: queue up resolved objects since they were resolved during process ops
	// and then resolve all of them at the end of process ops - UNR:582

	TSet<FChannelObjectPair>* TargetObjectSet = IncomingRefsMap.Find(ObjectRef);
	if (!TargetObjectSet)
	{
		return;
	}

	UE_LOG(LogSpatialReceiver, Verbose, TEXT("Resolving incoming operations depending on object ref %s, resolved object: %s"), *ObjectRef.ToString(), *Object->GetName());

	for (FChannelObjectPair& ChannelObjectPair : *TargetObjectSet)
	{
		FObjectReferencesMap* UnresolvedRefs = UnresolvedRefsMap.Find(ChannelObjectPair);
		if (!UnresolvedRefs)
		{
			continue;
		}

		if (!ChannelObjectPair.Key.IsValid() || !ChannelObjectPair.Value.IsValid())
		{
			UnresolvedRefsMap.Remove(ChannelObjectPair);
			continue;
		}

		USpatialActorChannel* DependentChannel = ChannelObjectPair.Key.Get();
		UObject* ReplicatingObject = ChannelObjectPair.Value.Get();

		bool bStillHasUnresolved = false;
		bool bSomeObjectsWereMapped = false;
		TArray<UProperty*> RepNotifies;

		FRepLayout& RepLayout = DependentChannel->GetObjectRepLayout(ReplicatingObject);
		FRepStateStaticBuffer& ShadowData = DependentChannel->GetObjectStaticBuffer(ReplicatingObject);

		ResolveObjectReferences(RepLayout, ReplicatingObject, *UnresolvedRefs, ShadowData.GetData(), (uint8*)ReplicatingObject, ReplicatingObject->GetClass()->GetPropertiesSize(), RepNotifies, bSomeObjectsWereMapped, bStillHasUnresolved);

		if (bSomeObjectsWereMapped)
		{
			DependentChannel->RemoveRepNotifiesWithUnresolvedObjs(RepNotifies, RepLayout, *UnresolvedRefs, ReplicatingObject);

			UE_LOG(LogSpatialReceiver, Log, TEXT("Resolved for target object %s"), *ReplicatingObject->GetName());
			DependentChannel->PostReceiveSpatialUpdate(ReplicatingObject, RepNotifies);
		}

		if (!bStillHasUnresolved)
		{
			UnresolvedRefsMap.Remove(ChannelObjectPair);
		}
	}

	IncomingRefsMap.Remove(ObjectRef);
}

void USpatialReceiver::ResolveIncomingRPCs(UObject* Object, const FUnrealObjectRef& ObjectRef)
{
	FIncomingRPCArray* IncomingRPCArray = IncomingRPCMap.Find(ObjectRef);
	if (!IncomingRPCArray)
	{
		return;
	}

	UE_LOG(LogSpatialReceiver, Verbose, TEXT("Resolving incoming RPCs depending on object ref %s, resolved object: %s"), *ObjectRef.ToString(), *Object->GetName());

	for (const TSharedPtr<FPendingIncomingRPC>& IncomingRPC : *IncomingRPCArray)
	{
		if (!IncomingRPC->TargetObject.IsValid())
		{
			// The target object has been destroyed before this RPC was resolved
			continue;
		}

		IncomingRPC->UnresolvedRefs.Remove(ObjectRef);
		if (IncomingRPC->UnresolvedRefs.Num() == 0)
		{
			FString SenderWorkerId = IncomingRPC->SenderWorkerId;
			ApplyRPC(IncomingRPC->TargetObject.Get(), IncomingRPC->Function, IncomingRPC->Payload, SenderWorkerId);
		}
	}

	IncomingRPCMap.Remove(ObjectRef);
}

void USpatialReceiver::ResolveObjectReferences(FRepLayout& RepLayout, UObject* ReplicatedObject, FObjectReferencesMap& ObjectReferencesMap, uint8* RESTRICT StoredData, uint8* RESTRICT Data, int32 MaxAbsOffset, TArray<UProperty*>& RepNotifies, bool& bOutSomeObjectsWereMapped, bool& bOutStillHasUnresolved)
{
	for (auto It = ObjectReferencesMap.CreateIterator(); It; ++It)
	{
		int32 AbsOffset = It.Key();

		if (AbsOffset >= MaxAbsOffset)
		{
			UE_LOG(LogSpatialReceiver, Log, TEXT("ResolveObjectReferences: Removed unresolved reference: AbsOffset >= MaxAbsOffset: %d"), AbsOffset);
			It.RemoveCurrent();
			continue;
		}

		FObjectReferences& ObjectReferences = It.Value();

		UProperty* Property = ObjectReferences.Property;

		// ParentIndex is -1 for handover properties
		bool bIsHandover = ObjectReferences.ParentIndex == -1;
		FRepParentCmd* Parent = ObjectReferences.ParentIndex >= 0 ? &RepLayout.Parents[ObjectReferences.ParentIndex] : nullptr;

#if ENGINE_MINOR_VERSION <= 20
		int32 StoredDataOffset = AbsOffset;
#else
		int32 StoredDataOffset = ObjectReferences.ShadowOffset;
#endif

		if (ObjectReferences.Array)
		{
			check(Property->IsA<UArrayProperty>());

			if (!bIsHandover)
			{
				Property->CopySingleValue(StoredData + StoredDataOffset, Data + AbsOffset);
			}

			FScriptArray* StoredArray = bIsHandover ? nullptr : (FScriptArray*)(StoredData + StoredDataOffset);
			FScriptArray* Array = (FScriptArray*)(Data + AbsOffset);

			int32 NewMaxOffset = Array->Num() * Property->ElementSize;

			bool bArrayHasUnresolved = false;
			ResolveObjectReferences(RepLayout, ReplicatedObject, *ObjectReferences.Array, bIsHandover ? nullptr : (uint8*)StoredArray->GetData(), (uint8*)Array->GetData(), NewMaxOffset, RepNotifies, bOutSomeObjectsWereMapped, bArrayHasUnresolved);
			if (!bArrayHasUnresolved)
			{
				It.RemoveCurrent();
			}
			else
			{
				bOutStillHasUnresolved = true;
			}
			continue;
		}

		bool bResolvedSomeRefs = false;
		UObject* SinglePropObject = nullptr;

		for (auto UnresolvedIt = ObjectReferences.UnresolvedRefs.CreateIterator(); UnresolvedIt; ++UnresolvedIt)
		{
			FUnrealObjectRef& ObjectRef = *UnresolvedIt;

			FNetworkGUID NetGUID = PackageMap->GetNetGUIDFromUnrealObjectRef(ObjectRef);
			if (NetGUID.IsValid())
			{
				UObject* Object = PackageMap->GetObjectFromNetGUID(NetGUID, true);
				check(Object);

				UE_LOG(LogSpatialReceiver, Log, TEXT("ResolveObjectReferences: Resolved object ref: Offset: %d, Object ref: %s, PropName: %s, ObjName: %s"), AbsOffset, *ObjectRef.ToString(), *Property->GetNameCPP(), *Object->GetName());

				UnresolvedIt.RemoveCurrent();
				bResolvedSomeRefs = true;

				if (ObjectReferences.bSingleProp)
				{
					SinglePropObject = Object;
				}
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
				UObjectPropertyBase* ObjectProperty = Cast<UObjectPropertyBase>(Property);
				check(ObjectProperty);

				ObjectProperty->SetObjectPropertyValue(Data + AbsOffset, SinglePropObject);
			}
			else if (ObjectReferences.bFastArrayProp)
			{
				TSet<FUnrealObjectRef> NewUnresolvedRefs;
				FSpatialNetBitReader ValueDataReader(PackageMap, ObjectReferences.Buffer.GetData(), ObjectReferences.NumBufferBits, NewUnresolvedRefs);

				check(Property->IsA<UArrayProperty>());
				UScriptStruct* NetDeltaStruct = GetFastArraySerializerProperty(Cast<UArrayProperty>(Property));

				FSpatialNetDeltaSerializeInfo::DeltaSerializeRead(NetDriver, ValueDataReader, ReplicatedObject, Parent->ArrayIndex, Parent->Property, NetDeltaStruct);

				if (NewUnresolvedRefs.Num() > 0)
				{
					bOutStillHasUnresolved = true;
				}
			}
			else
			{
				TSet<FUnrealObjectRef> NewUnresolvedRefs;
				FSpatialNetBitReader BitReader(PackageMap, ObjectReferences.Buffer.GetData(), ObjectReferences.NumBufferBits, NewUnresolvedRefs);
				check(Property->IsA<UStructProperty>());
				ReadStructProperty(BitReader, Cast<UStructProperty>(Property), NetDriver, Data + AbsOffset, bOutStillHasUnresolved);
			}

			if (Parent && Parent->Property->HasAnyPropertyFlags(CPF_RepNotify))
			{
				if (Parent->RepNotifyCondition == REPNOTIFY_Always || !Property->Identical(StoredData + StoredDataOffset, Data + AbsOffset))
				{
					RepNotifies.AddUnique(Parent->Property);
				}
			}
		}

		if (ObjectReferences.UnresolvedRefs.Num() > 0)
		{
			bOutStillHasUnresolved = true;
		}
		else
		{
			It.RemoveCurrent();
		}
	}
}
