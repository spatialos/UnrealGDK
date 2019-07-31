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
#include "Schema/ClientRPCEndpoint.h"
#include "Schema/DynamicComponent.h"
#include "Schema/RPCPayload.h"
#include "Schema/ServerRPCEndpoint.h"
#include "Schema/SpawnData.h"
#include "Schema/UnrealMetadata.h"
#include "SpatialConstants.h"
#include "Utils/ComponentReader.h"
#include "Utils/ErrorCodeRemapping.h"
#include "Utils/RepLayoutUtils.h"
#include "Utils/SpatialMetrics.h"

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

void USpatialReceiver::OnAddEntity(const Worker_AddEntityOp& Op)
{
	UE_LOG(LogSpatialReceiver, Verbose, TEXT("AddEntity: %lld"), Op.entity_id);

	check(bInCriticalSection);

	PendingAddEntities.Emplace(Op.entity_id);
}

void USpatialReceiver::OnAddComponent(const Worker_AddComponentOp& Op)
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
	case SpatialConstants::NETMULTICAST_RPCS_COMPONENT_ID:
	case SpatialConstants::RPCS_ON_ENTITY_CREATION_ID:
	case SpatialConstants::DEBUG_METRICS_COMPONENT_ID:
	case SpatialConstants::ALWAYS_RELEVANT_COMPONENT_ID:
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
	case SpatialConstants::CLIENT_RPC_ENDPOINT_COMPONENT_ID:
	case SpatialConstants::SERVER_RPC_ENDPOINT_COMPONENT_ID:
		Schema_Object* FieldsObject = Schema_GetComponentDataFields(Op.data.schema_type);
		RegisterListeningEntityIfReady(Op.entity_id, FieldsObject);
		return;
	}

	if (ClassInfoManager->IsSublevelComponent(Op.data.component_id))
	{
		return;
	}

	if (bInCriticalSection)
	{
		PendingAddComponents.Emplace(Op.entity_id, Op.data.component_id, MakeUnique<DynamicComponent>(Op.data));
	}
	else
	{
		HandleIndividualAddComponent(Op);
	}
}

void USpatialReceiver::OnRemoveEntity(const Worker_RemoveEntityOp& Op)
{
	RemoveActor(Op.entity_id);
}

void USpatialReceiver::OnRemoveComponent(const Worker_RemoveComponentOp& Op)
{
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
	for (const auto& Op : QueuedRemoveComponentOps)
	{
		ProcessRemoveComponent(Op);
	}

	QueuedRemoveComponentOps.Empty();
}

void USpatialReceiver::RemoveComponentOpsForEntity(Worker_EntityId EntityId)
{
	for (auto& RemoveComponentOp : QueuedRemoveComponentOps)
	{
		if (RemoveComponentOp.entity_id == EntityId)
		{
			// Zero component op to prevent array resize
			RemoveComponentOp = Worker_RemoveComponentOp{};
		}
	}
}

void USpatialReceiver::ProcessRemoveComponent(const Worker_RemoveComponentOp& Op)
{
	if (!StaticComponentView->HasComponent(Op.entity_id, Op.component_id))
	{
		return;
	}

	if (AActor* Actor = Cast<AActor>(PackageMap->GetObjectFromEntityId(Op.entity_id).Get()))
	{
		if (UObject* Object = PackageMap->GetObjectFromUnrealObjectRef(FUnrealObjectRef(Op.entity_id, Op.component_id)).Get())
		{
			if (USpatialActorChannel* Channel = NetDriver->GetActorChannelByEntityId(Op.entity_id))
			{
				Channel->CreateSubObjects.Remove(Object);

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

void USpatialReceiver::OnAuthorityChange(const Worker_AuthorityChangeOp& Op)
{
	if (bInCriticalSection)
	{
		PendingAuthorityChanges.Add(Op);
		return;
	}

	HandleActorAuthority(Op);
}

void USpatialReceiver::HandlePlayerLifecycleAuthority(const Worker_AuthorityChangeOp& Op, APlayerController* PlayerController)
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

void USpatialReceiver::HandleActorAuthority(const Worker_AuthorityChangeOp& Op)
{
	StaticComponentView->OnAuthorityChange(Op);

	if (GlobalStateManager->HandlesComponent(Op.component_id))
	{
		GlobalStateManager->AuthorityChanged(Op);
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

		// Subobject Delegation
		TPair<Worker_EntityId_Key, Worker_ComponentId> EntityComponentPair = MakeTuple(static_cast<Worker_EntityId_Key>(Op.entity_id), Op.component_id);
		if (TSharedRef<FPendingSubobjectAttachment>* PendingSubobjectAttachmentPtr = PendingEntitySubobjectDelegations.Find(EntityComponentPair))
		{
			FPendingSubobjectAttachment& PendingSubobjectAttachment = PendingSubobjectAttachmentPtr->Get();

			PendingSubobjectAttachment.PendingAuthorityDelegations.Remove(Op.component_id);

			if (PendingSubobjectAttachment.PendingAuthorityDelegations.Num() == 0)
			{
				if (UObject* Object = PendingSubobjectAttachment.Subobject.Get())
				{
					Sender->SendAddComponent(PendingSubobjectAttachment.Channel, Object, *PendingSubobjectAttachment.Info);
				}
			}

			PendingEntitySubobjectDelegations.Remove(EntityComponentPair);
		}
	}
	else
	{
		if (Op.component_id == SpatialConstants::CLIENT_RPC_ENDPOINT_COMPONENT_ID)
		{
			if (USpatialActorChannel* ActorChannel = NetDriver->GetActorChannelByEntityId(Op.entity_id))
			{
				ActorChannel->ClientProcessOwnershipChange(Op.authority == WORKER_AUTHORITY_AUTHORITATIVE);
			}

			// If we are a Pawn or PlayerController, our local role should be ROLE_AutonomousProxy. Otherwise ROLE_SimulatedProxy
			if ((Actor->IsA<APawn>() || Actor->IsA<APlayerController>()) && Op.component_id == SpatialConstants::CLIENT_RPC_ENDPOINT_COMPONENT_ID)
			{
				Actor->Role = (Op.authority == WORKER_AUTHORITY_AUTHORITATIVE) ? ROLE_AutonomousProxy : ROLE_SimulatedProxy;
			}
		}
	}

	if (Op.authority == WORKER_AUTHORITY_AUTHORITATIVE)
	{
		if (Op.component_id == SpatialConstants::CLIENT_RPC_ENDPOINT_COMPONENT_ID)
		{
			Sender->SendClientEndpointReadyUpdate(Op.entity_id);
		}
		if (Op.component_id == SpatialConstants::SERVER_RPC_ENDPOINT_COMPONENT_ID)
		{
			Sender->SendServerEndpointReadyUpdate(Op.entity_id);
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

		Worker_ComponentData* ComponentData = PendingAddComponent.Data->ComponentData;
		Schema_Object* ComponentObject = Schema_GetComponentDataFields(ComponentData->schema_type);
		return Schema_GetBool(ComponentObject, SpatialConstants::ACTOR_TEAROFF_ID);
	}

	return false;
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
		UClass* Class = UnrealMetadataComp->GetNativeEntityClass();
		if (Class == nullptr)
		{
			UE_LOG(LogSpatialReceiver, Warning, TEXT("The received actor with entity id %lld couldn't be loaded. The actor (%s) will not be spawned."),
				EntityId, *UnrealMetadataComp->ClassPath);
			return;
		}

		// Make sure ClassInfo exists
		ClassInfoManager->GetOrCreateClassInfoByClass(Class);

		// If the received actor is torn off, don't bother spawning it.
		// (This is only needed due to the delay between tearoff and deleting the entity. See https://improbableio.atlassian.net/browse/UNR-841)
		if (IsReceivedEntityTornOff(EntityId))
		{
			UE_LOG(LogSpatialReceiver, Verbose, TEXT("The received actor with entity id %lld was already torn off. The actor will not be spawned."), EntityId);
			return;
		}

		EntityActor = TryGetOrCreateActor(UnrealMetadataComp, SpawnDataComp);

		if (EntityActor == nullptr)
		{
			// This could be nullptr if:
			// a stably named actor could not be found
			// the Actor is a singleton that has arrived over the wire before it has been created on this worker
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
				ApplyComponentDataOnActorCreation(EntityId, *PendingAddComponent.Data->ComponentData, Channel);
			}
		}

		if (!NetDriver->IsServer())
		{
			// Update interest on the entity's components after receiving initial component data (so Role and RemoteRole are properly set).
			Sender->SendComponentInterestForActor(Channel, EntityId, Channel->IsOwnedByWorker());

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

	UE_LOG(LogSpatialReceiver, Verbose, TEXT("Worker %s Remove Actor: %s %lld"), *NetDriver->Connection->GetWorkerId(), Actor && !Actor->IsPendingKill() ? *Actor->GetName() : TEXT("nullptr"), EntityId);

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
			UE_LOG(LogSpatialReceiver, Warning, TEXT("RemoveActor: actor for entity %lld was already deleted (likely on the authoritative worker) but still has an open actor channel."), EntityId);
#if ENGINE_MINOR_VERSION <= 20
			ActorChannel->ConditionalCleanUp();
#else
			ActorChannel->ConditionalCleanUp(false, EChannelCloseReason::Destroyed);
#endif
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
		}
		return;
	}

	// Actor is a startup actor that is a part of the level. We need to do an entity query to see
	// if the entity was actually deleted or only removed from our view
	if (Actor->IsFullNameStableForNetworking())
	{
		QueryForStartupActor(Actor, EntityId);

		// We can't call CleanupDeletedEntity here as we need the NetDriver to maintain the EntityId
		// to Actor Channel mapping for the DestoryActor to function correctly
		PackageMap->RemoveEntityActor(EntityId);
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
	StartupActorDelegate.BindLambda([this, WeakActor, EntityId](const Worker_EntityQueryResponseOp& Op)
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

	check(PackageMap->GetObjectFromEntityId(EntityId) == nullptr);
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

void USpatialReceiver::ApplyComponentDataOnActorCreation(Worker_EntityId EntityId, const Worker_ComponentData& Data, USpatialActorChannel* Channel)
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
		// If we can't find this subobject, it's a dynamically attached object. Create it now.
		TargetObject = NewObject<UObject>(Channel->GetActor(), ClassInfoManager->GetClassByComponentId(Data.component_id));

		Channel->GetActor()->OnSubobjectCreatedFromReplication(TargetObject.Get());

		PackageMap->ResolveSubobject(TargetObject.Get(), FUnrealObjectRef(EntityId, Offset));

		Channel->CreateSubObjects.Add(TargetObject.Get());
	}

	ApplyComponentData(TargetObject.Get(), Channel, Data);
}

void USpatialReceiver::HandleIndividualAddComponent(const Worker_AddComponentOp& Op)
{
	uint32 Offset = 0;
	bool bFoundOffset = ClassInfoManager->GetOffsetByComponentId(Op.data.component_id, Offset);
	if (!bFoundOffset)
	{
		UE_LOG(LogSpatialReceiver, Warning, TEXT("EntityId %lld, ComponentId %d - Could not find offset for component id "
			"when receiving dynamic AddComponent."), Op.entity_id, Op.data.component_id);
		return;
	}

	// Object already exists, we can apply data directly. 
	if (UObject* Object = PackageMap->GetObjectFromUnrealObjectRef(FUnrealObjectRef(Op.entity_id, Offset)).Get())
	{
		ApplyComponentData(Object, NetDriver->GetActorChannelByEntityId(Op.entity_id), Op.data);
		return;
	}

	// Otherwise this is a dynamically attached component. We need to make sure we have all related components before creation.
	PendingDynamicSubobjectComponents.Add(MakeTuple(static_cast<Worker_EntityId_Key>(Op.entity_id), Op.data.component_id),
		PendingAddComponentWrapper(Op.entity_id, Op.data.component_id, MakeUnique<DynamicComponent>(Op.data)));

	const FClassInfo& Info = ClassInfoManager->GetClassInfoByComponentId(Op.data.component_id);

	bool bReadyToCreate = true;
	ForAllSchemaComponentTypes([&](ESchemaComponentType Type)
	{
		Worker_ComponentId ComponentId = Info.SchemaComponents[Type];

		if (ComponentId == SpatialConstants::INVALID_COMPONENT_ID)
		{
			return;
		}

		if (!PendingDynamicSubobjectComponents.Contains(MakeTuple(static_cast<Worker_EntityId_Key>(Op.entity_id), ComponentId)))
		{
			bReadyToCreate = false;
		}
	});

	if (bReadyToCreate)
	{
		AttachDynamicSubobject(Op.entity_id, Info);
	}
}

void USpatialReceiver::AttachDynamicSubobject(Worker_EntityId EntityId, const FClassInfo& Info)
{
	AActor* Actor = Cast<AActor>(PackageMap->GetObjectFromEntityId(EntityId).Get());

	if (Actor == nullptr)
	{
		UE_LOG(LogSpatialReceiver, Verbose, TEXT("Tried to dynamically attach subobject of type %s to entity %lld but couldn't find Actor!"), *Info.Class->GetName(), EntityId);
		return;
	}

	USpatialActorChannel* Channel = NetDriver->GetActorChannelByEntityId(EntityId);
	if (Channel == nullptr)
	{
		UE_LOG(LogSpatialReceiver, Verbose, TEXT("Tried to dynamically attach subobject of type %s to entity %lld but couldn't find Channel!"), *Info.Class->GetName(), EntityId);
		return;
	}

	UObject* Subobject = NewObject<UObject>(Actor, Info.Class.Get());

	Actor->OnSubobjectCreatedFromReplication(Subobject);

	PackageMap->ResolveSubobject(Subobject, FUnrealObjectRef(EntityId, Info.SchemaComponents[SCHEMA_Data]));

	Channel->CreateSubObjects.Add(Subobject);

	ForAllSchemaComponentTypes([&](ESchemaComponentType Type)
	{
		Worker_ComponentId ComponentId = Info.SchemaComponents[Type];

		if (ComponentId == SpatialConstants::INVALID_COMPONENT_ID)
		{
			return;
		}

		TPair<Worker_EntityId_Key, Worker_ComponentId> EntityComponentPair = MakeTuple(static_cast<Worker_EntityId_Key>(EntityId), ComponentId);

		PendingAddComponentWrapper& AddComponent = PendingDynamicSubobjectComponents[EntityComponentPair];
		ApplyComponentData(Subobject, NetDriver->GetActorChannelByEntityId(EntityId), *AddComponent.Data->ComponentData);
		PendingDynamicSubobjectComponents.Remove(EntityComponentPair);
	});

	// If on a client, we need to set up the proper component interest for the new subobject.
	if (!NetDriver->IsServer())
	{
		Sender->SendComponentInterestForSubobject(Info, EntityId, Channel->IsOwnedByWorker());
	}
}

void USpatialReceiver::ApplyComponentData(UObject* TargetObject, USpatialActorChannel* Channel, const Worker_ComponentData& Data)
{
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
		Reader.ApplyComponentData(Data, TargetObject, Channel, /* bIsHandover */ false);

		QueueIncomingRepUpdates(ChannelObjectPair, ObjectReferencesMap, UnresolvedRefs);
	}
	else if (ComponentType == SCHEMA_Handover)
	{
		FObjectReferencesMap& ObjectReferencesMap = UnresolvedRefsMap.FindOrAdd(ChannelObjectPair);
		TSet<FUnrealObjectRef> UnresolvedRefs;

		ComponentReader Reader(NetDriver, ObjectReferencesMap, UnresolvedRefs);
		Reader.ApplyComponentData(Data, TargetObject, Channel, /* bIsHandover */ true);

		QueueIncomingRepUpdates(ChannelObjectPair, ObjectReferencesMap, UnresolvedRefs);
	}
	else
	{
		UE_LOG(LogSpatialReceiver, Verbose, TEXT("Entity: %d Component: %d - Skipping because RPC components don't have actual data."), Channel->GetEntityId(), Data.component_id);
	}
}

void USpatialReceiver::OnComponentUpdate(const Worker_ComponentUpdateOp& Op)
{
	if (Op.update.component_id == SpatialConstants::SERVER_RPC_ENDPOINT_COMPONENT_ID ||
		Op.update.component_id == SpatialConstants::CLIENT_RPC_ENDPOINT_COMPONENT_ID)
	{
		Schema_Object* FieldsObject = Schema_GetComponentUpdateFields(Op.update.schema_type);
		RegisterListeningEntityIfReady(Op.entity_id, FieldsObject);
	}

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
	case SpatialConstants::DEBUG_METRICS_COMPONENT_ID:
	case SpatialConstants::ALWAYS_RELEVANT_COMPONENT_ID:
		UE_LOG(LogSpatialReceiver, Verbose, TEXT("Entity: %d Component: %d - Skipping because this is hand-written Spatial component"), Op.entity_id, Op.update.component_id);
		return;
	case SpatialConstants::GSM_SHUTDOWN_COMPONENT_ID:
#if WITH_EDITOR
		GlobalStateManager->OnShutdownComponentUpdate(Op.update);
#endif // WITH_EDITOR
		return;
	case SpatialConstants::HEARTBEAT_COMPONENT_ID:
		OnHeartbeatComponentUpdate(Op);
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
		HandleRPC(Op);
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

void USpatialReceiver::HandleRPC(const Worker_ComponentUpdateOp& Op)
{
	Worker_EntityId EntityId = Op.entity_id;

	// If the update is to the client rpc endpoint, then the handler should have authority over the server rpc endpoint component and vice versa
	// Ideally these events are never delivered to workers which are not able to handle them with clever interest management
	const Worker_ComponentId RPCEndpointComponentId = Op.update.component_id == SpatialConstants::CLIENT_RPC_ENDPOINT_COMPONENT_ID
		? SpatialConstants::SERVER_RPC_ENDPOINT_COMPONENT_ID : SpatialConstants::CLIENT_RPC_ENDPOINT_COMPONENT_ID;

	// Multicast RPCs should be executed by whoever receives them.
	if (Op.update.component_id != SpatialConstants::NETMULTICAST_RPCS_COMPONENT_ID)
	{
		if (StaticComponentView->GetAuthority(Op.entity_id, RPCEndpointComponentId) != WORKER_AUTHORITY_AUTHORITATIVE)
		{
			return;
		}
	}

	// Always process unpacked RPCs since some cannot be packed.
	ProcessRPCEventField(EntityId, Op, RPCEndpointComponentId, /* bPacked */ false);

	if (GetDefault<USpatialGDKSettings>()->bPackRPCs)
	{
		// Only process packed RPCs if packing is enabled
		ProcessRPCEventField(EntityId, Op, RPCEndpointComponentId, /* bPacked */ true);
	}
}

void USpatialReceiver::ProcessRPCEventField(Worker_EntityId EntityId, const Worker_ComponentUpdateOp& Op, Worker_ComponentId RPCEndpointComponentId, bool bPacked)
{
	Schema_Object* EventsObject = Schema_GetComponentUpdateEvents(Op.update.schema_type);
	const Schema_FieldId EventId = bPacked ? SpatialConstants::UNREAL_RPC_ENDPOINT_PACKED_EVENT_ID : SpatialConstants::UNREAL_RPC_ENDPOINT_EVENT_ID;
	uint32 EventCount = Schema_GetObjectCount(EventsObject, EventId);

	for (uint32 i = 0; i < EventCount; i++)
	{
		Schema_Object* EventData = Schema_IndexObject(EventsObject, EventId, i);

		RPCPayload Payload(EventData);

		FUnrealObjectRef ObjectRef(EntityId, Payload.Offset);

		if (bPacked)
		{
			// When packing unreliable RPCs into one update, they also always go through the PlayerController.
			// This means we need to retrieve the actual target Entity ID from the payload.
			if (Op.update.component_id == SpatialConstants::CLIENT_RPC_ENDPOINT_COMPONENT_ID ||
				Op.update.component_id == SpatialConstants::SERVER_RPC_ENDPOINT_COMPONENT_ID)
			{
				ObjectRef.Entity = Schema_GetEntityId(EventData, SpatialConstants::UNREAL_PACKED_RPC_PAYLOAD_ENTITY_ID);


				// In a zoned multiworker scenario we might not have gained authority over the current entity in this bundle in time
				// before processing so don't ApplyRPCs to an entity that we don't have authority over.
				if (StaticComponentView->GetAuthority(ObjectRef.Entity, RPCEndpointComponentId) != WORKER_AUTHORITY_AUTHORITATIVE)
				{
					continue;
				}
			}
		}

		FPendingRPCParamsPtr Params = MakeUnique<FPendingRPCParams>(ObjectRef, MoveTemp(Payload));
		if (UObject* TargetObject = PackageMap->GetObjectFromUnrealObjectRef(ObjectRef).Get())
		{
			const FClassInfo& ClassInfo = ClassInfoManager->GetOrCreateClassInfoByObject(TargetObject);
			UFunction* Function = ClassInfo.RPCs[Payload.Index];
			const FRPCInfo& RPCInfo = ClassInfoManager->GetRPCInfo(TargetObject, Function);

			if (!IncomingRPCs.ObjectHasRPCsQueuedOfType(ObjectRef.Entity, RPCInfo.Type))
			{
				// Apply if possible, queue otherwise
				if (ApplyRPC(*Params))
				{
					continue;
				}
			}
		}

		QueueIncomingRPC(MoveTemp(Params));
	}
}

void USpatialReceiver::OnCommandRequest(const Worker_CommandRequestOp& Op)
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
		Sender->SendEmptyCommandResponse(Op.request.component_id, CommandIndex, Op.request_id);
		return;
	}
#if WITH_EDITOR
	else if (Op.request.component_id == SpatialConstants::GSM_SHUTDOWN_COMPONENT_ID && CommandIndex == SpatialConstants::SHUTDOWN_MULTI_PROCESS_REQUEST_ID)
	{
		NetDriver->GlobalStateManager->ReceiveShutdownMultiProcessRequest();
		return;
	}
#endif // WITH_EDITOR
#if !UE_BUILD_SHIPPING
	else if (Op.request.component_id == SpatialConstants::DEBUG_METRICS_COMPONENT_ID)
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
			Schema_Object* Payload = Schema_GetCommandRequestObject(Op.request.schema_type);
			NetDriver->SpatialMetrics->OnModifySettingCommand(Payload);
			break;
		}
		default:
			UE_LOG(LogSpatialReceiver, Error, TEXT("Unknown command index for DebugMetrics component: %d, entity: %lld"), CommandIndex, Op.entity_id);
			break;
		}

		Sender->SendEmptyCommandResponse(Op.request.component_id, CommandIndex, Op.request_id);
		return;
	}
#endif // !UE_BUILD_SHIPPING

	Schema_Object* RequestObject = Schema_GetCommandRequestObject(Op.request.schema_type);

	RPCPayload Payload(RequestObject);
	FUnrealObjectRef ObjectRef = FUnrealObjectRef(Op.entity_id, Payload.Offset);
	UObject* TargetObject = PackageMap->GetObjectFromUnrealObjectRef(ObjectRef).Get();
	if (TargetObject == nullptr)
	{
		UE_LOG(LogSpatialReceiver, Warning, TEXT("No target object found for EntityId %d"), Op.entity_id);
		Sender->SendEmptyCommandResponse(Op.request.component_id, CommandIndex, Op.request_id);
		return;
	}

	const FClassInfo& Info = ClassInfoManager->GetOrCreateClassInfoByObject(TargetObject);
	UFunction* Function = Info.RPCs[Payload.Index];
	const FRPCInfo& RPCInfo = ClassInfoManager->GetRPCInfo(TargetObject, Function);

	UE_LOG(LogSpatialReceiver, Verbose, TEXT("Received command request (entity: %lld, component: %d, function: %s)"),
		Op.entity_id, Op.request.component_id, *Function->GetName());

	bool bAppliedRPC = false;
	if (!IncomingRPCs.ObjectHasRPCsQueuedOfType(ObjectRef.Entity, RPCInfo.Type))
	{
		if (ApplyRPC(TargetObject, Function, Payload, FString()))
		{
			bAppliedRPC = true;
		}
	}

	if (!bAppliedRPC)
	{
		QueueIncomingRPC(MakeUnique<FPendingRPCParams>(ObjectRef, MoveTemp(Payload)));
	}

	Sender->SendEmptyCommandResponse(Op.request.component_id, CommandIndex, Op.request_id);
}

void USpatialReceiver::OnCommandResponse(const Worker_CommandResponseOp& Op)
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

void USpatialReceiver::ReceiveCommandResponse(const Worker_CommandResponseOp& Op)
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
			TimerManager->SetTimer(RetryTimer, [WeakSender = TWeakObjectPtr<USpatialSender>(Sender), ReliableRPC]()
			{
				if (USpatialSender* SpatialSender = WeakSender.Get())
				{
					SpatialSender->EnqueueRetryRPC(ReliableRPC);
				}
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

void USpatialReceiver::RegisterListeningEntityIfReady(Worker_EntityId EntityId, Schema_Object* Object)
{
	if (Schema_GetBoolCount(Object, SpatialConstants::UNREAL_RPC_ENDPOINT_READY_ID) > 0)
	{
		bool bReady = GetBoolFromSchema(Object, SpatialConstants::UNREAL_RPC_ENDPOINT_READY_ID);
		if (bReady)
		{
			if (USpatialActorChannel* Channel = NetDriver->GetActorChannelByEntityId(EntityId))
			{
				Channel->StartListening();
				if (UObject* TargetObject = Channel->GetActor())
				{
					Sender->SendOutgoingRPCs();
				}
			}
		}
	}
}

bool USpatialReceiver::ApplyRPC(UObject* TargetObject, UFunction* Function, const RPCPayload& Payload, const FString& SenderWorkerId)
{
	bool bApplied = false;

	uint8* Parms = (uint8*)FMemory_Alloca(Function->ParmsSize);
	FMemory::Memzero(Parms, Function->ParmsSize);

	TSet<FUnrealObjectRef> UnresolvedRefs;

	RPCPayload PayloadCopy = Payload;
	FSpatialNetBitReader PayloadReader(PackageMap, PayloadCopy.PayloadData.GetData(), PayloadCopy.CountDataBits(), UnresolvedRefs);

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
		bApplied = true;
	}

	// Destroy the parameters.
	// warning: highly dependent on UObject::ProcessEvent freeing of parms!
	for (TFieldIterator<UProperty> It(Function); It && It->HasAnyPropertyFlags(CPF_Parm); ++It)
	{
		It->DestroyValue_InContainer(Parms);
	}
	return bApplied;
}

bool USpatialReceiver::ApplyRPC(const FPendingRPCParams& Params)
{
	TWeakObjectPtr<UObject> TargetObjectWeakPtr = PackageMap->GetObjectFromUnrealObjectRef(Params.ObjectRef);
	if (!TargetObjectWeakPtr.IsValid())
	{
		return false;
	}

	const FClassInfo& ClassInfo = ClassInfoManager->GetOrCreateClassInfoByObject(TargetObjectWeakPtr.Get());
	UFunction* Function = ClassInfo.RPCs[Params.Payload.Index];
	if (Function == nullptr)
	{
		return false;
	}

	return ApplyRPC(TargetObjectWeakPtr.Get(), Function, Params.Payload, FString{});
}

void USpatialReceiver::OnReserveEntityIdsResponse(const Worker_ReserveEntityIdsResponseOp& Op)
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

void USpatialReceiver::OnCreateEntityResponse(const Worker_CreateEntityResponseOp& Op)
{
	if (Op.status_code != WORKER_STATUS_CODE_SUCCESS)
	{
		UE_LOG(LogSpatialReceiver, Error, TEXT("Create entity request failed: request id: %d, entity id: %lld, message: %s"), Op.request_id, Op.entity_id, UTF8_TO_TCHAR(Op.message));
	}
	else
	{
		UE_LOG(LogSpatialReceiver, Verbose, TEXT("Create entity request succeeded: request id: %d, entity id: %lld, message: %s"), Op.request_id, Op.entity_id, UTF8_TO_TCHAR(Op.message));
	}

	if (CreateEntityDelegate* Delegate = CreateEntityDelegates.Find(Op.request_id))
	{
		Delegate->ExecuteIfBound(Op);
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

void USpatialReceiver::OnEntityQueryResponse(const Worker_EntityQueryResponseOp& Op)
{
	if (Op.status_code != WORKER_STATUS_CODE_SUCCESS)
	{
		UE_LOG(LogSpatialReceiver, Error, TEXT("EntityQuery failed: request id: %d, message: %s"), Op.request_id, UTF8_TO_TCHAR(Op.message));
	}

	if (EntityQueryDelegate* RequestDelegate = EntityQueryDelegates.Find(Op.request_id))
	{
		UE_LOG(LogSpatialReceiver, Verbose, TEXT("Executing EntityQueryResponse with delegate, request id: %d, number of entities: %d, message: %s"), Op.request_id, Op.result_count, UTF8_TO_TCHAR(Op.message));
		RequestDelegate->ExecuteIfBound(Op);
	}
	else
	{
		UE_LOG(LogSpatialReceiver, Warning, TEXT("Recieved EntityQueryResponse but with no delegate set, request id: %d, number of entities: %d, message: %s"), Op.request_id, Op.result_count, UTF8_TO_TCHAR(Op.message));
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

void USpatialReceiver::AddCreateEntityDelegate(Worker_RequestId RequestId, const CreateEntityDelegate& Delegate)
{
	CreateEntityDelegates.Add(RequestId, Delegate);
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
		const FRPCInfo& RPCInfo = ClassInfoManager->GetRPCInfo(Actor, Function);
		const FUnrealObjectRef ObjectRef = PackageMap->GetUnrealObjectRefFromObject(Actor);
		check(ObjectRef != FUnrealObjectRef::UNRESOLVED_OBJECT_REF);

		if (!IncomingRPCs.ObjectHasRPCsQueuedOfType(ObjectRef.Entity, RPCInfo.Type))
		{
			if (ApplyRPC(Actor, Function, RPC, FString()))
			{
				continue;
			}
		}

		QueueIncomingRPC(MakeUnique<FPendingRPCParams>(ObjectRef, MoveTemp(RPC)));
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

void USpatialReceiver::QueueIncomingRPC(FPendingRPCParamsPtr Params)
{
	TWeakObjectPtr<UObject> TargetObjectWeakPtr = PackageMap->GetObjectFromUnrealObjectRef(Params->ObjectRef);
	if (!TargetObjectWeakPtr.IsValid())
	{
		UE_LOG(LogSpatialReceiver, Verbose, TEXT("The object has been deleted, dropping the RPC"));
		return;
	}

	UObject* TargetObject = TargetObjectWeakPtr.Get();
	const FClassInfo& ClassInfo = ClassInfoManager->GetOrCreateClassInfoByObject(TargetObject);
	UFunction* Function = ClassInfo.RPCs[Params->Payload.Index];
	const FRPCInfo& RPCInfo = ClassInfoManager->GetRPCInfo(TargetObject, Function);
	ESchemaComponentType Type = RPCInfo.Type;

	IncomingRPCs.QueueRPC(MoveTemp(Params), Type);
}

void USpatialReceiver::ResolvePendingOperations_Internal(UObject* Object, const FUnrealObjectRef& ObjectRef)
{
	UE_LOG(LogSpatialReceiver, Verbose, TEXT("Resolving pending object refs and RPCs which depend on object: %s %s."), *Object->GetName(), *ObjectRef.ToString());

	Sender->ResolveOutgoingOperations(Object, /* bIsHandover */ false);
	Sender->ResolveOutgoingOperations(Object, /* bIsHandover */ true);
	ResolveIncomingOperations(Object, ObjectRef);
	// TODO: UNR-1650 We're trying to resolve all queues, which introduces more overhead.
	ResolveIncomingRPCs();
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

			UE_LOG(LogSpatialReceiver, Verbose, TEXT("Resolved for target object %s"), *ReplicatingObject->GetName());
			DependentChannel->PostReceiveSpatialUpdate(ReplicatingObject, RepNotifies);
		}

		if (!bStillHasUnresolved)
		{
			UnresolvedRefsMap.Remove(ChannelObjectPair);
		}
	}

	IncomingRefsMap.Remove(ObjectRef);
}

void USpatialReceiver::ResolveIncomingRPCs()
{
	FProcessRPCDelegate Delegate;
	Delegate.BindUObject(this, &USpatialReceiver::ApplyRPC);
	IncomingRPCs.ProcessRPCs(Delegate);
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

				UE_LOG(LogSpatialReceiver, Verbose, TEXT("ResolveObjectReferences: Resolved object ref: Offset: %d, Object ref: %s, PropName: %s, ObjName: %s"), AbsOffset, *ObjectRef.ToString(), *Property->GetNameCPP(), *Object->GetName());

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
		UE_LOG(LogSpatialReceiver, Warning, TEXT("Received heartbeat component update after NetConnection has been cleaned up. PlayerController entity: %lld"), Op.entity_id);
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
			UE_LOG(LogSpatialReceiver, Verbose, TEXT("Received multiple heartbeat events in a single component update, entity %lld."), Op.entity_id);
		}

		NetConnection->OnHeartbeat();
	}

	Schema_Object* FieldsObject = Schema_GetComponentUpdateFields(Op.update.schema_type);
	if (Schema_GetBoolCount(FieldsObject, SpatialConstants::HEARTBEAT_CLIENT_HAS_QUIT_ID) > 0 &&
		GetBoolFromSchema(FieldsObject, SpatialConstants::HEARTBEAT_CLIENT_HAS_QUIT_ID))
	{
		// Client has disconnected, let's clean up their connection.
		NetConnection->CleanUp();
		AuthorityPlayerControllerConnectionMap.Remove(Op.entity_id);
	}
} 
