// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "Interop/GlobalStateManager.h"

#if WITH_EDITOR
#include "Settings/LevelEditorPlaySettings.h"
#include "Editor.h"
#endif

#include "Engine/Classes/AI/AISystemBase.h"
#include "Engine/World.h"
#include "EngineClasses/SpatialActorChannel.h"
#include "EngineClasses/SpatialNetConnection.h"
#include "EngineClasses/SpatialNetDriver.h"
#include "EngineClasses/SpatialPackageMapClient.h"
#include "EngineUtils.h"
#include "GameFramework/GameModeBase.h"
#include "Interop/Connection/SpatialWorkerConnection.h"
#include "Interop/SpatialReceiver.h"
#include "Interop/SpatialSender.h"
#include "LoadBalancing/AbstractLBStrategy.h"
#include "Kismet/GameplayStatics.h"
#include "Schema/ServerWorker.h"
#include "Schema/UnrealMetadata.h"
#include "SpatialConstants.h"
#include "UObject/UObjectGlobals.h"
#include "Utils/EntityPool.h"

DEFINE_LOG_CATEGORY(LogGlobalStateManager);

using namespace SpatialGDK;

void UGlobalStateManager::Init(USpatialNetDriver* InNetDriver)
{
	NetDriver = InNetDriver;
	StaticComponentView = InNetDriver->StaticComponentView;
	Sender = InNetDriver->Sender;
	Receiver = InNetDriver->Receiver;
	GlobalStateManagerEntityId = SpatialConstants::INITIAL_GLOBAL_STATE_MANAGER_ENTITY_ID;

#if WITH_EDITOR
	const ULevelEditorPlaySettings* const PlayInSettings = GetDefault<ULevelEditorPlaySettings>();

	// Only the client should ever send this request.
	if (PlayInSettings && NetDriver && NetDriver->GetNetMode() != NM_DedicatedServer)
	{
		bool bRunUnderOneProcess = true;
		PlayInSettings->GetRunUnderOneProcess(bRunUnderOneProcess);

		if (!bRunUnderOneProcess && !PrePIEEndedHandle.IsValid())
		{
			PrePIEEndedHandle = FEditorDelegates::PrePIEEnded.AddUObject(this, &UGlobalStateManager::OnPrePIEEnded);
		}
	}
#endif // WITH_EDITOR
  
	bAcceptingPlayers = false;
	bHasSentReadyForVirtualWorkerAssignment = false;
	bCanBeginPlay = false;
	bCanSpawnWithAuthority = false;
}

void UGlobalStateManager::ApplySingletonManagerData(const Worker_ComponentData& Data)
{
	Schema_Object* ComponentObject = Schema_GetComponentDataFields(Data.schema_type);
	SingletonNameToEntityId = GetStringToEntityMapFromSchema(ComponentObject, SpatialConstants::SINGLETON_MANAGER_SINGLETON_NAME_TO_ENTITY_ID);
}

void UGlobalStateManager::ApplyDeploymentMapData(const Worker_ComponentData& Data)
{
	Schema_Object* ComponentObject = Schema_GetComponentDataFields(Data.schema_type);
	
	SetDeploymentMapURL(GetStringFromSchema(ComponentObject, SpatialConstants::DEPLOYMENT_MAP_MAP_URL_ID));

	bAcceptingPlayers = GetBoolFromSchema(ComponentObject, SpatialConstants::DEPLOYMENT_MAP_ACCEPTING_PLAYERS_ID);

	DeploymentSessionId = Schema_GetInt32(ComponentObject, SpatialConstants::DEPLOYMENT_MAP_SESSION_ID);

	SchemaHash = Schema_GetUint32(ComponentObject, SpatialConstants::DEPLOYMENT_MAP_SCHEMA_HASH);
}

void UGlobalStateManager::ApplyStartupActorManagerData(const Worker_ComponentData& Data)
{
	Schema_Object* ComponentObject = Schema_GetComponentDataFields(Data.schema_type);

	bCanBeginPlay = GetBoolFromSchema(ComponentObject, SpatialConstants::STARTUP_ACTOR_MANAGER_CAN_BEGIN_PLAY_ID);

	TrySendWorkerReadyToBeginPlay();
}

void UGlobalStateManager::TrySendWorkerReadyToBeginPlay()
{
	// Once a worker has received the StartupActorManager AddComponent op, we say that a
	// worker is ready to begin play. This means if the GSM-authoritative worker then sets
	// canBeginPlay=true it will be received as a ComponentUpdate and so we can differentiate
	// from when canBeginPlay=true was loaded from the snapshot and was received as an
	// AddComponent. This is important for handling startup Actors correctly in a zoned
	// environment.
	const bool bHasReceivedStartupActorData = StaticComponentView->HasComponent(GlobalStateManagerEntityId, SpatialConstants::STARTUP_ACTOR_MANAGER_COMPONENT_ID);
	const bool bWorkerEntityCreated = NetDriver->WorkerEntityId != SpatialConstants::INVALID_ENTITY_ID;
	if (bHasSentReadyForVirtualWorkerAssignment || !bHasReceivedStartupActorData || !bWorkerEntityCreated)
	{
		return;
	}

	FWorkerComponentUpdate Update = {};
	Update.component_id = SpatialConstants::SERVER_WORKER_COMPONENT_ID;
	Update.schema_type = Schema_CreateComponentUpdate();
	Schema_Object* UpdateObject = Schema_GetComponentUpdateFields(Update.schema_type);
	Schema_AddBool(UpdateObject, SpatialConstants::SERVER_WORKER_READY_TO_BEGIN_PLAY_ID, true);

	bHasSentReadyForVirtualWorkerAssignment = true;
	NetDriver->Connection->SendComponentUpdate(NetDriver->WorkerEntityId, &Update);
}

void UGlobalStateManager::ApplySingletonManagerUpdate(const Worker_ComponentUpdate& Update)
{
	Schema_Object* ComponentObject = Schema_GetComponentUpdateFields(Update.schema_type);

	if (Schema_GetObjectCount(ComponentObject, SpatialConstants::SINGLETON_MANAGER_SINGLETON_NAME_TO_ENTITY_ID) > 0)
	{
		SingletonNameToEntityId = GetStringToEntityMapFromSchema(ComponentObject, SpatialConstants::SINGLETON_MANAGER_SINGLETON_NAME_TO_ENTITY_ID);
	}
}

void UGlobalStateManager::ApplyDeploymentMapUpdate(const Worker_ComponentUpdate& Update)
{
	Schema_Object* ComponentObject = Schema_GetComponentUpdateFields(Update.schema_type);

	if (Schema_GetObjectCount(ComponentObject, SpatialConstants::DEPLOYMENT_MAP_MAP_URL_ID) == 1)
	{
		SetDeploymentMapURL(GetStringFromSchema(ComponentObject, SpatialConstants::DEPLOYMENT_MAP_MAP_URL_ID));
	}

	if (Schema_GetBoolCount(ComponentObject, SpatialConstants::DEPLOYMENT_MAP_ACCEPTING_PLAYERS_ID) == 1)
	{
		bAcceptingPlayers = GetBoolFromSchema(ComponentObject, SpatialConstants::DEPLOYMENT_MAP_ACCEPTING_PLAYERS_ID);
	}

	if (Schema_GetObjectCount(ComponentObject, SpatialConstants::DEPLOYMENT_MAP_SESSION_ID) == 1)
	{
		DeploymentSessionId = Schema_GetInt32(ComponentObject, SpatialConstants::DEPLOYMENT_MAP_SESSION_ID);
	}

	if (Schema_GetObjectCount(ComponentObject, SpatialConstants::DEPLOYMENT_MAP_SCHEMA_HASH) == 1)
	{
		SchemaHash = Schema_GetUint32(ComponentObject, SpatialConstants::DEPLOYMENT_MAP_SCHEMA_HASH);
	}
}

#if WITH_EDITOR
void UGlobalStateManager::OnPrePIEEnded(bool bValue)
{
	SendShutdownMultiProcessRequest();
	FEditorDelegates::PrePIEEnded.Remove(PrePIEEndedHandle);
}

void UGlobalStateManager::SendShutdownMultiProcessRequest()
{
	/** When running with Use Single Process unticked, send a shutdown command to the servers to allow SpatialOS to shutdown.
	  * Standard UnrealEngine behavior is to call TerminateProc on external processes and there is no method to send any messaging
	  * to those external process.
	  * The GDK requires shutdown code to be ran for workers to disconnect cleanly so instead of abruptly shutting down the server worker,
	  * just send a command to the worker to begin it's shutdown phase. 
	  */
	Worker_CommandRequest CommandRequest = {};
	CommandRequest.component_id = SpatialConstants::GSM_SHUTDOWN_COMPONENT_ID;
	CommandRequest.command_index = SpatialConstants::SHUTDOWN_MULTI_PROCESS_REQUEST_ID;
	CommandRequest.schema_type = Schema_CreateCommandRequest();

	NetDriver->Connection->SendCommandRequest(GlobalStateManagerEntityId, &CommandRequest, SpatialConstants::SHUTDOWN_MULTI_PROCESS_REQUEST_ID);
}

void UGlobalStateManager::ReceiveShutdownMultiProcessRequest()
{
	if (NetDriver && NetDriver->GetNetMode() == NM_DedicatedServer)
	{
		UE_LOG(LogGlobalStateManager, Log, TEXT("Received shutdown multi-process request."));
		
		// Since the server works are shutting down, set reset the accepting_players flag to false to prevent race conditions  where the client connects quicker than the server. 
		SetAcceptingPlayers(false);
		DeploymentSessionId = 0;
		SendSessionIdUpdate();

		// If we have multiple servers, they need to be informed of PIE session ending.
		SendShutdownAdditionalServersEvent();

		// Allow this worker to begin shutting down.
		FGenericPlatformMisc::RequestExit(false);
	}
}

void UGlobalStateManager::OnShutdownComponentUpdate(const Worker_ComponentUpdate& Update)
{
	Schema_Object* EventsObject = Schema_GetComponentUpdateEvents(Update.schema_type);
	if (Schema_GetObjectCount(EventsObject, SpatialConstants::SHUTDOWN_ADDITIONAL_SERVERS_EVENT_ID) > 0)
	{
		ReceiveShutdownAdditionalServersEvent();
	}
}

void UGlobalStateManager::ReceiveShutdownAdditionalServersEvent()
{
	if (NetDriver && NetDriver->GetNetMode() == NM_DedicatedServer)
	{
		UE_LOG(LogGlobalStateManager, Log, TEXT("Received shutdown additional servers event."));

		FGenericPlatformMisc::RequestExit(false);
	}
}

void UGlobalStateManager::SendShutdownAdditionalServersEvent()
{
	if (!NetDriver->StaticComponentView->HasAuthority(GlobalStateManagerEntityId, SpatialConstants::GSM_SHUTDOWN_COMPONENT_ID))
	{
		UE_LOG(LogGlobalStateManager, Warning, TEXT("Tried to send shutdown_additional_servers event on the GSM but this worker does not have authority."));
		return;
	}

	FWorkerComponentUpdate ComponentUpdate = {};

	ComponentUpdate.component_id = SpatialConstants::GSM_SHUTDOWN_COMPONENT_ID;
	ComponentUpdate.schema_type = Schema_CreateComponentUpdate();
	Schema_Object* EventsObject = Schema_GetComponentUpdateEvents(ComponentUpdate.schema_type);
	Schema_AddObject(EventsObject, SpatialConstants::SHUTDOWN_ADDITIONAL_SERVERS_EVENT_ID);

	NetDriver->Connection->SendComponentUpdate(GlobalStateManagerEntityId, &ComponentUpdate);
}
#endif // WITH_EDITOR

void UGlobalStateManager::ApplyStartupActorManagerUpdate(const Worker_ComponentUpdate& Update)
{
	Schema_Object* ComponentObject = Schema_GetComponentUpdateFields(Update.schema_type);

	bCanBeginPlay = GetBoolFromSchema(ComponentObject, SpatialConstants::STARTUP_ACTOR_MANAGER_CAN_BEGIN_PLAY_ID);
	bCanSpawnWithAuthority = true;
}

void UGlobalStateManager::LinkExistingSingletonActor(const UClass* SingletonActorClass)
{
	const Worker_EntityId* SingletonEntityIdPtr = SingletonNameToEntityId.Find(SingletonActorClass->GetPathName());
	if (SingletonEntityIdPtr == nullptr)
	{
		// No entry in SingletonNameToEntityId for this singleton class type
		UE_LOG(LogGlobalStateManager, Verbose, TEXT("LinkExistingSingletonActor %s failed to find entry"), *SingletonActorClass->GetName());
		return;
	}

	const Worker_EntityId SingletonEntityId = *SingletonEntityIdPtr;
	if (SingletonEntityId == SpatialConstants::INVALID_ENTITY_ID)
	{
		// Singleton Entity hasn't been created yet
		UE_LOG(LogGlobalStateManager, Log, TEXT("LinkExistingSingletonActor %s entity id is invalid"), *SingletonActorClass->GetName());
		return;
	}

	TPair<AActor*, USpatialActorChannel*>* ActorChannelPair = SingletonClassPathToActorChannels.Find(SingletonActorClass->GetPathName());
	if (ActorChannelPair == nullptr)
	{
		// Dynamically spawn singleton actor if we have queued up data - ala USpatialReceiver::ReceiveActor - JIRA: 735

		// No local actor has registered itself as replicatible on this worker
		UE_LOG(LogGlobalStateManager, Log, TEXT("LinkExistingSingletonActor no actor registered for class %s"), *SingletonActorClass->GetName());
		return;
	}

	AActor* SingletonActor = ActorChannelPair->Key;
	USpatialActorChannel*& Channel = ActorChannelPair->Value;

	if (Channel != nullptr)
	{
		// Channel has already been setup
		UE_LOG(LogGlobalStateManager, Verbose, TEXT("UGlobalStateManager::LinkExistingSingletonActor channel already setup for %s"), *SingletonActorClass->GetName());
		return;
	}

	// If we have previously queued up data for this entity, apply it - UNR-734

	// We're now ready to start replicating this actor, create a channel
	USpatialNetConnection* Connection = Cast<USpatialNetConnection>(NetDriver->ClientConnections[0]);

	Channel = Cast<USpatialActorChannel>(Connection->CreateChannelByName(NAME_Actor, EChannelCreateFlags::OpenedLocally));

	if (StaticComponentView->HasAuthority(SingletonEntityId, SpatialConstants::POSITION_COMPONENT_ID))
	{
		SingletonActor->Role = ROLE_Authority;
		SingletonActor->RemoteRole = ROLE_SimulatedProxy;
	}
	else
	{
		SingletonActor->Role = ROLE_SimulatedProxy;
		SingletonActor->RemoteRole = ROLE_Authority;
	}

	// Since the entity already exists, we have to handle setting up the PackageMap properly for this Actor
	NetDriver->PackageMap->ResolveEntityActor(SingletonActor, SingletonEntityId);

#if ENGINE_MINOR_VERSION <= 22
	Channel->SetChannelActor(SingletonActor);
#else
	Channel->SetChannelActor(SingletonActor, ESetChannelActorFlags::None);
#endif

	UE_LOG(LogGlobalStateManager, Log, TEXT("Linked Singleton Actor %s with id %d"), *SingletonActor->GetClass()->GetName(), SingletonEntityId);
}

void UGlobalStateManager::LinkAllExistingSingletonActors()
{
	// Early out for clients as they receive Singleton Actors via the normal Unreal replicated actor flow
	if (!NetDriver->IsServer())
	{
		return;
	}

	for (const auto& Pair : SingletonNameToEntityId)
	{
		UClass* SingletonActorClass = LoadObject<UClass>(nullptr, *Pair.Key);
		if (SingletonActorClass == nullptr)
		{
			UE_LOG(LogGlobalStateManager, Error, TEXT("Failed to find Singleton Actor Class: %s"), *Pair.Key);
			continue;
		}

		LinkExistingSingletonActor(SingletonActorClass);
	}
}

USpatialActorChannel* UGlobalStateManager::AddSingleton(AActor* SingletonActor)
{
	check(SingletonActor->GetIsReplicated());

	UClass* SingletonActorClass = SingletonActor->GetClass();

	TPair<AActor*, USpatialActorChannel*>& ActorChannelPair = SingletonClassPathToActorChannels.FindOrAdd(SingletonActorClass->GetPathName());
	USpatialActorChannel*& Channel = ActorChannelPair.Value;
	check(ActorChannelPair.Key == nullptr || ActorChannelPair.Key == SingletonActor);
	ActorChannelPair.Key = SingletonActor;

	// Just return the channel if it's already been setup
	if (Channel != nullptr)
	{
		UE_LOG(LogGlobalStateManager, Log, TEXT("AddSingleton called when channel already setup: %s"), *SingletonActor->GetName());
		return Channel;
	}

	bool bHasGSMAuthority = NetDriver->StaticComponentView->HasAuthority(GlobalStateManagerEntityId, SpatialConstants::SINGLETON_MANAGER_COMPONENT_ID);
	if (bHasGSMAuthority)
	{
		// We have control over the GSM, so can safely setup a new channel and let it allocate an entity id
		USpatialNetConnection* Connection = Cast<USpatialNetConnection>(NetDriver->ClientConnections[0]);
		Channel = Cast<USpatialActorChannel>(Connection->CreateChannelByName(NAME_Actor, EChannelCreateFlags::OpenedLocally));

		// If entity id already exists for this singleton, set the actor to it
		// Otherwise SetChannelActor will issue a new entity id request
		if (const Worker_EntityId* SingletonEntityId = SingletonNameToEntityId.Find(SingletonActorClass->GetPathName()))
		{
			check(NetDriver->PackageMap->GetObjectFromEntityId(*SingletonEntityId) == nullptr);
			NetDriver->PackageMap->ResolveEntityActor(SingletonActor, *SingletonEntityId);
			if (!StaticComponentView->HasAuthority(*SingletonEntityId, SpatialConstants::POSITION_COMPONENT_ID))
			{
				SingletonActor->Role = ROLE_SimulatedProxy;
				SingletonActor->RemoteRole = ROLE_Authority;
			}
		}

#if ENGINE_MINOR_VERSION <= 22
		Channel->SetChannelActor(SingletonActor);
#else
		Channel->SetChannelActor(SingletonActor, ESetChannelActorFlags::None);
#endif
		UE_LOG(LogGlobalStateManager, Log, TEXT("Started replication of Singleton Actor %s"), *SingletonActorClass->GetName());
	}
	else
	{
		// We don't have control over the GSM, but we may have received the entity id for this singleton already
		LinkExistingSingletonActor(SingletonActorClass);
	}

	return Channel;
}

void UGlobalStateManager::RemoveSingletonInstance(const AActor* SingletonActor)
{
	check(SingletonActor->GetClass()->HasAnySpatialClassFlags(SPATIALCLASS_Singleton));

	SingletonClassPathToActorChannels.Remove(SingletonActor->GetClass()->GetPathName());
}

void UGlobalStateManager::RegisterSingletonChannel(AActor* SingletonActor, USpatialActorChannel* SingletonChannel)
{
	TPair<AActor*, USpatialActorChannel*>& ActorChannelPair = SingletonClassPathToActorChannels.FindOrAdd(SingletonActor->GetClass()->GetPathName());

	check(ActorChannelPair.Key == nullptr || ActorChannelPair.Key == SingletonActor);
	check(ActorChannelPair.Value == nullptr || ActorChannelPair.Value == SingletonChannel);

	ActorChannelPair.Key = SingletonActor;
	ActorChannelPair.Value = SingletonChannel;
}

void UGlobalStateManager::ExecuteInitialSingletonActorReplication()
{
	for (auto& ClassToActorChannel : SingletonClassPathToActorChannels)
	{
		auto& ActorChannelPair = ClassToActorChannel.Value;
		AddSingleton(ActorChannelPair.Key);
	}
}

void UGlobalStateManager::UpdateSingletonEntityId(const FString& ClassName, const Worker_EntityId SingletonEntityId)
{
	Worker_EntityId& EntityId = SingletonNameToEntityId.FindOrAdd(ClassName);
	EntityId = SingletonEntityId;

	if (!NetDriver->StaticComponentView->HasAuthority(GlobalStateManagerEntityId, SpatialConstants::SINGLETON_MANAGER_COMPONENT_ID))
	{
		UE_LOG(LogGlobalStateManager, Warning, TEXT("UpdateSingletonEntityId: no authority over the GSM! Update will not be sent. Singleton class: %s, entity: %lld"), *ClassName, SingletonEntityId);
		return;
	}

	FWorkerComponentUpdate Update = {};
	Update.component_id = SpatialConstants::SINGLETON_MANAGER_COMPONENT_ID;
	Update.schema_type = Schema_CreateComponentUpdate();
	Schema_Object* UpdateObject = Schema_GetComponentUpdateFields(Update.schema_type);

	AddStringToEntityMapToSchema(UpdateObject, 1, SingletonNameToEntityId);

	NetDriver->Connection->SendComponentUpdate(GlobalStateManagerEntityId, &Update);
}

bool UGlobalStateManager::IsSingletonEntity(Worker_EntityId EntityId) const
{
	for (const auto& Pair : SingletonNameToEntityId)
	{
		if (Pair.Value == EntityId)
		{
			return true;
		}
	}
	return false;
}

void UGlobalStateManager::SetDeploymentState()
{
	check(NetDriver->StaticComponentView->HasAuthority(GlobalStateManagerEntityId, SpatialConstants::DEPLOYMENT_MAP_COMPONENT_ID));

	// Send the component update that we can now accept players.
	UE_LOG(LogGlobalStateManager, Log, TEXT("Setting deployment URL to '%s'"), *NetDriver->GetWorld()->URL.Map);
	UE_LOG(LogGlobalStateManager, Log, TEXT("Setting schema hash to '%u'"), NetDriver->ClassInfoManager->SchemaDatabase->SchemaDescriptorHash);

	FWorkerComponentUpdate Update = {};
	Update.component_id = SpatialConstants::DEPLOYMENT_MAP_COMPONENT_ID;
	Update.schema_type = Schema_CreateComponentUpdate();
	Schema_Object* UpdateObject = Schema_GetComponentUpdateFields(Update.schema_type);

	// Set the map URL on the GSM.
	AddStringToSchema(UpdateObject, SpatialConstants::DEPLOYMENT_MAP_MAP_URL_ID, NetDriver->GetWorld()->URL.Map);

	// Set the schema hash for connecting workers to check against
	Schema_AddUint32(UpdateObject, SpatialConstants::DEPLOYMENT_MAP_SCHEMA_HASH, NetDriver->ClassInfoManager->SchemaDatabase->SchemaDescriptorHash);

	// Component updates are short circuited so we set the updated state here and then send the component update.
	NetDriver->Connection->SendComponentUpdate(GlobalStateManagerEntityId, &Update);
}

void UGlobalStateManager::SetAcceptingPlayers(bool bInAcceptingPlayers)
{
	check(NetDriver->StaticComponentView->HasAuthority(GlobalStateManagerEntityId, SpatialConstants::DEPLOYMENT_MAP_COMPONENT_ID));

	// Send the component update that we can now accept players.
	UE_LOG(LogGlobalStateManager, Log, TEXT("Setting accepting players to '%s'"), bInAcceptingPlayers ? TEXT("true") : TEXT("false"));
	FWorkerComponentUpdate Update = {};
	Update.component_id = SpatialConstants::DEPLOYMENT_MAP_COMPONENT_ID;
	Update.schema_type = Schema_CreateComponentUpdate();
	Schema_Object* UpdateObject = Schema_GetComponentUpdateFields(Update.schema_type);

	// Set the AcceptingPlayers state on the GSM
	Schema_AddBool(UpdateObject, SpatialConstants::DEPLOYMENT_MAP_ACCEPTING_PLAYERS_ID, static_cast<uint8_t>(bInAcceptingPlayers));

	// Component updates are short circuited so we set the updated state here and then send the component update.
	bAcceptingPlayers = bInAcceptingPlayers;
	NetDriver->Connection->SendComponentUpdate(GlobalStateManagerEntityId, &Update);
}

void UGlobalStateManager::AuthorityChanged(const Worker_AuthorityChangeOp& AuthOp)
{
	UE_LOG(LogGlobalStateManager, Verbose, TEXT("Authority over the GSM component %d has changed. This worker %s authority."), AuthOp.component_id,
		AuthOp.authority == WORKER_AUTHORITY_AUTHORITATIVE ? TEXT("now has") : TEXT ("does not have"));

	if (AuthOp.authority != WORKER_AUTHORITY_AUTHORITATIVE)
	{
		return;
	}

	switch (AuthOp.component_id)
	{
		case SpatialConstants::DEPLOYMENT_MAP_COMPONENT_ID:
		{
			GlobalStateManagerEntityId = AuthOp.entity_id;
			SetDeploymentState();
			if (!bAcceptingPlayers && IsReady())
			{
				SetAcceptingPlayers(true);
			}
			break;
		}
		case SpatialConstants::SINGLETON_MANAGER_COMPONENT_ID:
		{
			ExecuteInitialSingletonActorReplication();
			break;
		}
		case SpatialConstants::STARTUP_ACTOR_MANAGER_COMPONENT_ID:
		{
			// The bCanSpawnWithAuthority member determines whether a server-side worker
			// should consider calling BeginPlay on startup Actors if the load-balancing
			// strategy dictates that the worker should have authority over the Actor
			// (providing Unreal load balancing is enabled). This should only happen for
			// workers launching for fresh deployments, since for restarted workers and
			// when deployments are launched from a snapshot, the entities representing
			// startup Actors should already exist. If bCanBeginPlay is set to false, this
			// means it's a fresh deployment, so bCanSpawnWithAuthority should be true.
			// Conversely, if bCanBeginPlay is set to true, this worker is either a restarted
			// crashed worker or in a deployment loaded from snapshot, so bCanSpawnWithAuthority
			// should be false.
			bCanSpawnWithAuthority = !bCanBeginPlay;
			break;
		}
		default:
		{
			break;
		}
	}
}

bool UGlobalStateManager::HandlesComponent(const Worker_ComponentId ComponentId) const
{
	switch (ComponentId)
	{
		case SpatialConstants::SINGLETON_MANAGER_COMPONENT_ID:
		case SpatialConstants::DEPLOYMENT_MAP_COMPONENT_ID:
		case SpatialConstants::STARTUP_ACTOR_MANAGER_COMPONENT_ID:
		case SpatialConstants::GSM_SHUTDOWN_COMPONENT_ID:
			return true;
		default:
			return false;
	}
}

void UGlobalStateManager::ResetGSM()
{
	UE_LOG(LogGlobalStateManager, Display, TEXT("GlobalStateManager singletons are being reset. Session restarting."));

	SingletonNameToEntityId.Empty();
	SetAcceptingPlayers(false);

	// Reset the BeginPlay flag so Startup Actors are properly managed.
	SendCanBeginPlayUpdate(false);

	// Reset the Singleton map so Singletons are recreated.
	FWorkerComponentUpdate Update = {};
	Update.component_id = SpatialConstants::SINGLETON_MANAGER_COMPONENT_ID;
	Update.schema_type = Schema_CreateComponentUpdate();
	Schema_AddComponentUpdateClearedField(Update.schema_type, SpatialConstants::SINGLETON_MANAGER_SINGLETON_NAME_TO_ENTITY_ID);

	NetDriver->Connection->SendComponentUpdate(GlobalStateManagerEntityId, &Update);
}

void UGlobalStateManager::BeginDestroy()
{
	Super::BeginDestroy();

#if WITH_EDITOR
	if (NetDriver != nullptr && NetDriver->StaticComponentView->HasAuthority(GlobalStateManagerEntityId, SpatialConstants::STARTUP_ACTOR_MANAGER_COMPONENT_ID))
	{
		// If we are deleting dynamically spawned entities, we need to
		if (GetDefault<ULevelEditorPlaySettings>()->GetDeleteDynamicEntities())
		{
			// Reset the BeginPlay flag so Startup Actors are properly managed.
			SendCanBeginPlayUpdate(false);

			// Reset the Singleton map so Singletons are recreated.
			FWorkerComponentUpdate Update = {};
			Update.component_id = SpatialConstants::SINGLETON_MANAGER_COMPONENT_ID;
			Update.schema_type = Schema_CreateComponentUpdate();
			Schema_AddComponentUpdateClearedField(Update.schema_type, SpatialConstants::SINGLETON_MANAGER_SINGLETON_NAME_TO_ENTITY_ID);

			NetDriver->Connection->SendComponentUpdate(GlobalStateManagerEntityId, &Update);
		}
	}
#endif
}

void UGlobalStateManager::BecomeAuthoritativeOverAllActors()
{
	for (TActorIterator<AActor> It(NetDriver->World); It; ++It)
	{
		AActor* Actor = *It;
		if (Actor != nullptr && !Actor->IsPendingKill())
		{
			if (Actor->GetIsReplicated())
			{
				Actor->Role = ROLE_Authority;
				Actor->RemoteRole = ROLE_SimulatedProxy;
			}
		}
	}
}

void UGlobalStateManager::BecomeAuthoritativeOverActorsBasedOnLBStrategy()
{
	for (TActorIterator<AActor> It(NetDriver->World); It; ++It)
	{
		AActor* Actor = *It;
		if (Actor != nullptr && !Actor->IsPendingKill())
		{
			if (Actor->GetIsReplicated() && NetDriver->LoadBalanceStrategy->ShouldHaveAuthority(*Actor))
			{
				Actor->Role = ROLE_Authority;
				Actor->RemoteRole = ROLE_SimulatedProxy;
			}
		}
	}
}

void UGlobalStateManager::TriggerBeginPlay()
{
	const bool bHasStartupActorAuthority = NetDriver->StaticComponentView->HasAuthority(GlobalStateManagerEntityId, SpatialConstants::STARTUP_ACTOR_MANAGER_COMPONENT_ID);
	if (bHasStartupActorAuthority)
	{
		SendCanBeginPlayUpdate(true);
	}

	const bool bHasDeploymentMapAuth = NetDriver->StaticComponentView->HasAuthority(GlobalStateManagerEntityId, SpatialConstants::DEPLOYMENT_MAP_COMPONENT_ID);
	if (bHasDeploymentMapAuth && !bAcceptingPlayers)
	{
		SetAcceptingPlayers(true);
	}

	// If we're loading from a snapshot, we shouldn't try and call BeginPlay with authority
	if (bCanSpawnWithAuthority)
	{
		if (GetDefault<USpatialGDKSettings>()->bEnableUnrealLoadBalancer)
		{
			BecomeAuthoritativeOverActorsBasedOnLBStrategy();
		}
		else
		{
			BecomeAuthoritativeOverAllActors();
		}
	}

	NetDriver->World->GetWorldSettings()->SetGSMReadyForPlay();
	NetDriver->World->GetWorldSettings()->NotifyBeginPlay();
}

bool UGlobalStateManager::GetCanBeginPlay() const
{
	return bCanBeginPlay;
}

bool UGlobalStateManager::IsReady() const
{
	return GetCanBeginPlay() || NetDriver->StaticComponentView->HasAuthority(GlobalStateManagerEntityId, SpatialConstants::STARTUP_ACTOR_MANAGER_COMPONENT_ID);
}

void UGlobalStateManager::SendCanBeginPlayUpdate(const bool bInCanBeginPlay)
{
	check(NetDriver->StaticComponentView->HasAuthority(GlobalStateManagerEntityId, SpatialConstants::STARTUP_ACTOR_MANAGER_COMPONENT_ID));

	bCanBeginPlay = bInCanBeginPlay;

	FWorkerComponentUpdate Update = {};
	Update.component_id = SpatialConstants::STARTUP_ACTOR_MANAGER_COMPONENT_ID;
	Update.schema_type = Schema_CreateComponentUpdate();
	Schema_Object* UpdateObject = Schema_GetComponentUpdateFields(Update.schema_type);

	Schema_AddBool(UpdateObject, SpatialConstants::STARTUP_ACTOR_MANAGER_CAN_BEGIN_PLAY_ID, static_cast<uint8_t>(bCanBeginPlay));

	NetDriver->Connection->SendComponentUpdate(GlobalStateManagerEntityId, &Update);
}

// Queries for the GlobalStateManager in the deployment.
// bRetryUntilRecievedExpectedValues will continue querying until the state of AcceptingPlayers and SessionId are the same as the given arguments
// This is so clients know when to connect to the deployment.
void UGlobalStateManager::QueryGSM(const QueryDelegate& Callback)
{
	Worker_ComponentConstraint GSMComponentConstraint{};
	GSMComponentConstraint.component_id = SpatialConstants::DEPLOYMENT_MAP_COMPONENT_ID;

	Worker_Constraint GSMConstraint{};
	GSMConstraint.constraint_type = WORKER_CONSTRAINT_TYPE_COMPONENT;
	GSMConstraint.constraint.component_constraint = GSMComponentConstraint;

	Worker_EntityQuery GSMQuery{};
	GSMQuery.constraint = GSMConstraint;
	GSMQuery.result_type = WORKER_RESULT_TYPE_SNAPSHOT;

	Worker_RequestId RequestID;
	RequestID = NetDriver->Connection->SendEntityQueryRequest(&GSMQuery);

	EntityQueryDelegate GSMQueryDelegate;
	GSMQueryDelegate.BindLambda([this, Callback](const Worker_EntityQueryResponseOp& Op)
	{
		if (Op.status_code != WORKER_STATUS_CODE_SUCCESS)
		{
			UE_LOG(LogGlobalStateManager, Warning, TEXT("Could not find GSM via entity query: %s"), UTF8_TO_TCHAR(Op.message));
		}
		else if (Op.result_count == 0)
		{
			UE_LOG(LogGlobalStateManager, Log, TEXT("GSM entity query shows the GSM does not yet exist in the world."));
		}
		else
		{
			if (NetDriver->VirtualWorkerTranslator.IsValid())
			{
				ApplyVirtualWorkerMappingFromQueryResponse(Op);
			}
			ApplyDeploymentMapDataFromQueryResponse(Op);
			Callback.ExecuteIfBound(Op);
		}
	});

	Receiver->AddEntityQueryDelegate(RequestID, GSMQueryDelegate);
}

void UGlobalStateManager::ApplyVirtualWorkerMappingFromQueryResponse(const Worker_EntityQueryResponseOp& Op)
{
	check(NetDriver->VirtualWorkerTranslator.IsValid());
	for (uint32_t i = 0; i < Op.results[0].component_count; i++)
	{
		Worker_ComponentData Data = Op.results[0].components[i];
		if (Data.component_id == SpatialConstants::VIRTUAL_WORKER_TRANSLATION_COMPONENT_ID)
		{
			Schema_Object* ComponentObject = Schema_GetComponentDataFields(Data.schema_type);
			NetDriver->VirtualWorkerTranslator->ApplyVirtualWorkerManagerData(ComponentObject);
		}
	}
}

void UGlobalStateManager::ApplyDeploymentMapDataFromQueryResponse(const Worker_EntityQueryResponseOp& Op)
{
	for (uint32_t i = 0; i < Op.results[0].component_count; i++)
	{
		Worker_ComponentData Data = Op.results[0].components[i];
		if (Data.component_id == SpatialConstants::DEPLOYMENT_MAP_COMPONENT_ID)
		{
			ApplyDeploymentMapData(Data);
		}
	}
}

bool UGlobalStateManager::GetAcceptingPlayersAndSessionIdFromQueryResponse(const Worker_EntityQueryResponseOp& Op, bool& OutAcceptingPlayers, int32& OutSessionId)
{
	checkf(Op.result_count == 1, TEXT("There should never be more than one GSM"));

	bool AcceptingPlayersFound = false;
	bool SessionIdFound = false;

	// Iterate over each component on the GSM until we get the DeploymentMap component.
	for (uint32_t i = 0; i < Op.results[0].component_count; i++)
	{
		Worker_ComponentData Data = Op.results[0].components[i];
		if (Data.component_id == SpatialConstants::DEPLOYMENT_MAP_COMPONENT_ID)
		{
			Schema_Object* ComponentObject = Schema_GetComponentDataFields(Data.schema_type);

			if (Schema_GetBoolCount(ComponentObject, SpatialConstants::DEPLOYMENT_MAP_ACCEPTING_PLAYERS_ID) == 1)
			{
				OutAcceptingPlayers = GetBoolFromSchema(ComponentObject, SpatialConstants::DEPLOYMENT_MAP_ACCEPTING_PLAYERS_ID);
				AcceptingPlayersFound = true;
			}

			if (Schema_GetUint32Count(ComponentObject, SpatialConstants::DEPLOYMENT_MAP_SESSION_ID) == 1)
			{
				OutSessionId = Schema_GetInt32(ComponentObject, SpatialConstants::DEPLOYMENT_MAP_SESSION_ID);
				SessionIdFound = true;
			}

			if (AcceptingPlayersFound && SessionIdFound)
			{
				return true;
			}
		}
	}

	UE_LOG(LogGlobalStateManager, Warning, TEXT("Entity query response for the GSM did not contain both AcceptingPlayers and SessionId states."));

	return false;
}

void UGlobalStateManager::SetDeploymentMapURL(const FString& MapURL)
{
	UE_LOG(LogGlobalStateManager, Log, TEXT("Setting DeploymentMapURL: %s"), *MapURL);
	DeploymentMapURL = MapURL;
}

void UGlobalStateManager::IncrementSessionID()
{
	DeploymentSessionId++;
	SendSessionIdUpdate();
}

void UGlobalStateManager::SendSessionIdUpdate()
{
	FWorkerComponentUpdate Update = {};
	Update.component_id = SpatialConstants::DEPLOYMENT_MAP_COMPONENT_ID;
	Update.schema_type = Schema_CreateComponentUpdate();
	Schema_Object* ComponentObject = Schema_GetComponentUpdateFields(Update.schema_type);

	Schema_AddInt32(ComponentObject, SpatialConstants::DEPLOYMENT_MAP_SESSION_ID, DeploymentSessionId);

	NetDriver->Connection->SendComponentUpdate(GlobalStateManagerEntityId, &Update);
}
