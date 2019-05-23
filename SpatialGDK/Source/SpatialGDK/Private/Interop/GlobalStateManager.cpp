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
#include "Kismet/GameplayStatics.h"
#include "Runtime/Engine/Public/TimerManager.h"
#include "Schema/UnrealMetadata.h"
#include "SpatialConstants.h"
#include "UObject/UObjectGlobals.h"

DEFINE_LOG_CATEGORY(LogGlobalStateManager);

using namespace SpatialGDK;

void UGlobalStateManager::Init(USpatialNetDriver* InNetDriver, FTimerManager* InTimerManager)
{
	NetDriver = InNetDriver;
	StaticComponentView = InNetDriver->StaticComponentView;
	Sender = InNetDriver->Sender;
	Receiver = InNetDriver->Receiver;
	TimerManager = InTimerManager;
	GlobalStateManagerEntityId = SpatialConstants::INITIAL_GLOBAL_STATE_MANAGER_ENTITY_ID;

#if WITH_EDITOR
	const ULevelEditorPlaySettings* const PlayInSettings = GetDefault<ULevelEditorPlaySettings>();

	// Only the client should ever send this request.
	if (PlayInSettings && NetDriver && NetDriver->GetNetMode() != NM_DedicatedServer)
	{
		bool bRunUnderOneProcess = true;
		PlayInSettings->GetRunUnderOneProcess(bRunUnderOneProcess);

		if (!bRunUnderOneProcess)
		{
			FEditorDelegates::PrePIEEnded.AddUObject(this, &UGlobalStateManager::OnPrePIEEnded);
		}
	}
#endif // WITH_EDITOR
  
	bAcceptingPlayers = false;
	bCanBeginPlay = false;
	bTriggeredBeginPlay = false;
}

void UGlobalStateManager::ApplySingletonManagerData(const Worker_ComponentData& Data)
{
	Schema_Object* ComponentObject = Schema_GetComponentDataFields(Data.schema_type);
	SingletonNameToEntityId = GetStringToEntityMapFromSchema(ComponentObject, SpatialConstants::SINGLETON_MANAGER_SINGLETON_NAME_TO_ENTITY_ID);
}

void UGlobalStateManager::ApplyDeploymentMapData(const Worker_ComponentData& Data)
{
	Schema_Object* ComponentObject = Schema_GetComponentDataFields(Data.schema_type);
	
	// Set the Deployment Map URL.
	SetDeploymentMapURL(GetStringFromSchema(ComponentObject, SpatialConstants::DEPLOYMENT_MAP_MAP_URL_ID));

	// Set the AcceptingPlayers state.
	bool bDataAcceptingPlayers = GetBoolFromSchema(ComponentObject, SpatialConstants::DEPLOYMENT_MAP_ACCEPTING_PLAYERS_ID);
	ApplyAcceptingPlayersUpdate(bDataAcceptingPlayers);
}

void UGlobalStateManager::ApplyStartupActorManagerData(const Worker_ComponentData& Data)
{
	Schema_Object* ComponentObject = Schema_GetComponentDataFields(Data.schema_type);

	bool bCanBeginPlayData = GetBoolFromSchema(ComponentObject, SpatialConstants::STARTUP_ACTOR_MANAGER_CAN_BEGIN_PLAY_ID);
	ApplyCanBeginPlayUpdate(bCanBeginPlayData);
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
		bool bUpdateAcceptingPlayers = GetBoolFromSchema(ComponentObject, SpatialConstants::DEPLOYMENT_MAP_ACCEPTING_PLAYERS_ID);
		ApplyAcceptingPlayersUpdate(bUpdateAcceptingPlayers);
	}
}

void UGlobalStateManager::ApplyAcceptingPlayersUpdate(bool bAcceptingPlayersUpdate)
{
	if (bAcceptingPlayersUpdate != bAcceptingPlayers)
	{
		UE_LOG(LogGlobalStateManager, Log, TEXT("GlobalStateManager Update - AcceptingPlayers: %s"), bAcceptingPlayersUpdate ? TEXT("true") : TEXT("false"));
		bAcceptingPlayers = bAcceptingPlayersUpdate;

		// Tell the SpatialNetDriver that AcceptingPlayers has changed.
		NetDriver->OnAcceptingPlayersChanged(bAcceptingPlayersUpdate);
	}
}

#if WITH_EDITOR
void UGlobalStateManager::OnPrePIEEnded(bool bValue)
{
	SendShutdownMultiProcessRequest();
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
	CommandRequest.schema_type = Schema_CreateCommandRequest(SpatialConstants::GSM_SHUTDOWN_COMPONENT_ID, SpatialConstants::SHUTDOWN_MULTI_PROCESS_REQUEST_ID);

	NetDriver->Connection->SendCommandRequest(GlobalStateManagerEntityId, &CommandRequest, SpatialConstants::SHUTDOWN_MULTI_PROCESS_REQUEST_ID);
}

void UGlobalStateManager::ReceiveShutdownMultiProcessRequest()
{
	if (NetDriver && NetDriver->GetNetMode() == NM_DedicatedServer)
	{
		UE_LOG(LogGlobalStateManager, Log, TEXT("Received shutdown multi-process request."));
		
		// Since the server works are shutting down, set reset the accepting_players flag to false to prevent race conditions  where the client connects quicker than the server. 
		SetAcceptingPlayers(false);

		// If we have multiple servers, they need to be informed of PIE session ending.
		SendShutdownAdditionalServersEvent();

		// Allow this worker to begin shutting down.
		FGenericPlatformMisc::RequestExit(false);
	}
}

void UGlobalStateManager::OnShutdownComponentUpdate(Worker_ComponentUpdate& Update)
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

	Worker_ComponentUpdate ComponentUpdate = {};

	ComponentUpdate.component_id = SpatialConstants::GSM_SHUTDOWN_COMPONENT_ID;
	ComponentUpdate.schema_type = Schema_CreateComponentUpdate(SpatialConstants::GSM_SHUTDOWN_COMPONENT_ID);
	Schema_Object* EventsObject = Schema_GetComponentUpdateEvents(ComponentUpdate.schema_type);
	Schema_AddObject(EventsObject, SpatialConstants::SHUTDOWN_ADDITIONAL_SERVERS_EVENT_ID);

	NetDriver->Connection->SendComponentUpdate(GlobalStateManagerEntityId, &ComponentUpdate);
}
#endif // WITH_EDITOR

void UGlobalStateManager::ApplyStartupActorManagerUpdate(const Worker_ComponentUpdate& Update)
{
	Schema_Object* ComponentObject = Schema_GetComponentUpdateFields(Update.schema_type);

	if (Schema_GetBoolCount(ComponentObject, SpatialConstants::STARTUP_ACTOR_MANAGER_CAN_BEGIN_PLAY_ID) == 1)
	{
		bool bCanBeginPlayUpdate = GetBoolFromSchema(ComponentObject, SpatialConstants::STARTUP_ACTOR_MANAGER_CAN_BEGIN_PLAY_ID);
		ApplyCanBeginPlayUpdate(bCanBeginPlayUpdate);
	}
}

void UGlobalStateManager::ApplyCanBeginPlayUpdate(bool bCanBeginPlayUpdate)
{
	bCanBeginPlay = bCanBeginPlayUpdate;

	// For now, this will only be called on non-authoritative workers.
	if (bCanBeginPlay)
	{
		TriggerBeginPlay();
	}
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

	TPair<AActor*, USpatialActorChannel*>* ActorChannelPair = NetDriver->SingletonActorChannels.Find(SingletonActorClass);
	if (ActorChannelPair == nullptr)
	{
		// Dynamically spawn singleton actor if we have queued up data - ala USpatialReceiver::ReceiveActor - JIRA: 735

		// No local actor has registered itself as replicatible on this worker
		UE_LOG(LogGlobalStateManager, Log, TEXT("LinkExistingSingletonActor no actor registered"), *SingletonActorClass->GetName());
		return;
	}

	AActor* SingletonActor = ActorChannelPair->Key;
	USpatialActorChannel*& Channel = ActorChannelPair->Value;

	if (Channel != nullptr)
	{
		// Channel has already been setup
		UE_LOG(LogGlobalStateManager, Verbose, TEXT("UGlobalStateManager::LinkExistingSingletonActor channel already setup"), *SingletonActorClass->GetName());
		return;
	}

	// If we have previously queued up data for this entity, apply it - UNR-734

	// We're now ready to start replicating this actor, create a channel
	USpatialNetConnection* Connection = Cast<USpatialNetConnection>(NetDriver->ClientConnections[0]);

#if ENGINE_MINOR_VERSION <= 20
	Channel = Cast<USpatialActorChannel>(Connection->CreateChannel(CHTYPE_Actor, 1));
#else
	Channel = Cast<USpatialActorChannel>(Connection->CreateChannelByName(NAME_Actor, EChannelCreateFlags::OpenedLocally));
#endif

	if (StaticComponentView->GetAuthority(SingletonEntityId, SpatialConstants::POSITION_COMPONENT_ID) == WORKER_AUTHORITY_AUTHORITATIVE)
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

	Channel->SetChannelActor(SingletonActor);

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

	TPair<AActor*, USpatialActorChannel*>& ActorChannelPair = NetDriver->SingletonActorChannels.FindOrAdd(SingletonActorClass);
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
#if ENGINE_MINOR_VERSION <= 20
		Channel = Cast<USpatialActorChannel>(Connection->CreateChannel(CHTYPE_Actor, 1));
#else
		Channel = Cast<USpatialActorChannel>(Connection->CreateChannelByName(NAME_Actor, EChannelCreateFlags::OpenedLocally));
#endif

		// If entity id already exists for this singleton, set the actor to it
		// Otherwise SetChannelActor will issue a new entity id request
		if (const Worker_EntityId* SingletonEntityId = SingletonNameToEntityId.Find(SingletonActorClass->GetPathName()))
		{
			check(NetDriver->PackageMap->GetObjectFromEntityId(*SingletonEntityId) == nullptr);
			NetDriver->PackageMap->ResolveEntityActor(SingletonActor, *SingletonEntityId);
			if (StaticComponentView->GetAuthority(*SingletonEntityId, SpatialConstants::POSITION_COMPONENT_ID) != WORKER_AUTHORITY_AUTHORITATIVE)
			{
				SingletonActor->Role = ROLE_SimulatedProxy;
				SingletonActor->RemoteRole = ROLE_Authority;
			}
		}

		Channel->SetChannelActor(SingletonActor);
		UE_LOG(LogGlobalStateManager, Log, TEXT("Started replication of Singleton Actor %s"), *SingletonActorClass->GetName());
	}
	else
	{
		// We don't have control over the GSM, but we may have received the entity id for this singleton already
		LinkExistingSingletonActor(SingletonActorClass);
	}

	return Channel;
}

void UGlobalStateManager::ExecuteInitialSingletonActorReplication()
{
	for (auto& ClassToActorChannel : NetDriver->SingletonActorChannels)
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

	Worker_ComponentUpdate Update = {};
	Update.component_id = SpatialConstants::SINGLETON_MANAGER_COMPONENT_ID;
	Update.schema_type = Schema_CreateComponentUpdate(SpatialConstants::SINGLETON_MANAGER_COMPONENT_ID);
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

void UGlobalStateManager::SetAcceptingPlayers(bool bInAcceptingPlayers)
{
	if (!NetDriver->StaticComponentView->HasAuthority(GlobalStateManagerEntityId, SpatialConstants::DEPLOYMENT_MAP_COMPONENT_ID))
	{
		UE_LOG(LogGlobalStateManager, Warning, TEXT("Tried to set AcceptingPlayers on the GSM but this worker does not have authority."));
		return;
	}

	// Send the component update that we can now accept players.
	UE_LOG(LogGlobalStateManager, Log, TEXT("Setting accepting players to '%s'"), bInAcceptingPlayers ? TEXT("true") : TEXT("false"));
	Worker_ComponentUpdate Update = {};
	Update.component_id = SpatialConstants::DEPLOYMENT_MAP_COMPONENT_ID;
	Update.schema_type = Schema_CreateComponentUpdate(SpatialConstants::DEPLOYMENT_MAP_COMPONENT_ID);
	Schema_Object* UpdateObject = Schema_GetComponentUpdateFields(Update.schema_type);

	// Set the map URL on the GSM.
	AddStringToSchema(UpdateObject, SpatialConstants::DEPLOYMENT_MAP_MAP_URL_ID, NetDriver->GetWorld()->URL.Map);

	// Set the AcceptingPlayers state on the GSM
	Schema_AddBool(UpdateObject, SpatialConstants::DEPLOYMENT_MAP_ACCEPTING_PLAYERS_ID, static_cast<uint8_t>(bInAcceptingPlayers));

	// Component updates are short circuited so we set the updated state here and then send the component update.
	bAcceptingPlayers = bInAcceptingPlayers;
	NetDriver->Connection->SendComponentUpdate(GlobalStateManagerEntityId, &Update);
}

void UGlobalStateManager::SetCanBeginPlay(bool bInCanBeginPlay)
{
	if (!NetDriver->StaticComponentView->HasAuthority(GlobalStateManagerEntityId, SpatialConstants::STARTUP_ACTOR_MANAGER_COMPONENT_ID))
	{
		UE_LOG(LogGlobalStateManager, Warning, TEXT("Tried to set CanBeginPlay on the GSM but this worker does not have authority."));
		return;
	}

	Worker_ComponentUpdate Update = {};
	Update.component_id = SpatialConstants::STARTUP_ACTOR_MANAGER_COMPONENT_ID;
	Update.schema_type = Schema_CreateComponentUpdate(SpatialConstants::STARTUP_ACTOR_MANAGER_COMPONENT_ID);
	Schema_Object* UpdateObject = Schema_GetComponentUpdateFields(Update.schema_type);

	// Set CanBeginPlay on GSM
	Schema_AddBool(UpdateObject, SpatialConstants::STARTUP_ACTOR_MANAGER_CAN_BEGIN_PLAY_ID, static_cast<uint8_t>(bInCanBeginPlay));

	bCanBeginPlay = bInCanBeginPlay;
	NetDriver->Connection->SendComponentUpdate(GlobalStateManagerEntityId, &Update);
}

void UGlobalStateManager::AuthorityChanged(bool bWorkerAuthority, Worker_EntityId CurrentEntityID)
{
	UE_LOG(LogGlobalStateManager, Log, TEXT("Authority over the GSM has changed. This worker %s authority."),  bWorkerAuthority ? TEXT("now has") : TEXT ("does not have"));

	if (bWorkerAuthority)
	{
		// Make sure we update our known entity id for the GSM when we receive authority.
		GlobalStateManagerEntityId = CurrentEntityID;

		if (!bCanBeginPlay)
		{
			SetCanBeginPlay(true);
			BecomeAuthoritativeOverAllActors();
			TriggerBeginPlay();
		}

		// Start accepting players only AFTER we've triggered BeginPlay
		SetAcceptingPlayers(true);
	}
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
			SetCanBeginPlay(false);

			// Reset the Singleton map so Singletons are recreated.
			Worker_ComponentUpdate Update = {};
			Update.component_id = SpatialConstants::SINGLETON_MANAGER_COMPONENT_ID;
			Update.schema_type = Schema_CreateComponentUpdate(SpatialConstants::SINGLETON_MANAGER_COMPONENT_ID);
			Schema_AddComponentUpdateClearedField(Update.schema_type, SpatialConstants::SINGLETON_MANAGER_SINGLETON_NAME_TO_ENTITY_ID);

			NetDriver->Connection->SendComponentUpdate(GlobalStateManagerEntityId, &Update);
		}
	}
#endif
}

bool UGlobalStateManager::HasAuthority()
{
	return NetDriver->StaticComponentView->HasAuthority(GlobalStateManagerEntityId, SpatialConstants::SINGLETON_MANAGER_COMPONENT_ID);
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

void UGlobalStateManager::TriggerBeginPlay()
{
	if (bTriggeredBeginPlay)
	{
		UE_LOG(LogGlobalStateManager, Error, TEXT("Tried to trigger BeginPlay twice! This should never happen"));
		return;
	}

	NetDriver->World->GetWorldSettings()->SetGSMReadyForPlay();
	NetDriver->World->GetWorldSettings()->NotifyBeginPlay();

	bTriggeredBeginPlay = true;
}

// Queries for the GlobalStateManager in the deployment.
// bRetryUntilAcceptingPlayers will continue querying until the state of AcceptingPlayers is true, this is so clients know when to connect to the deployment.
void UGlobalStateManager::QueryGSM(bool bRetryUntilAcceptingPlayers)
{
	Worker_ComponentConstraint GSMComponentConstraint{};
	GSMComponentConstraint.component_id = SpatialConstants::DEPLOYMENT_MAP_COMPONENT_ID;

	Worker_Constraint GSMConstraint{};
	GSMConstraint.constraint_type = WORKER_CONSTRAINT_TYPE_COMPONENT;
	GSMConstraint.component_constraint = GSMComponentConstraint;

	Worker_EntityQuery GSMQuery{};
	GSMQuery.constraint = GSMConstraint;
	GSMQuery.result_type = WORKER_RESULT_TYPE_SNAPSHOT;

	Worker_RequestId RequestID;
	RequestID = NetDriver->Connection->SendEntityQueryRequest(&GSMQuery);

	EntityQueryDelegate GSMQueryDelegate;
	GSMQueryDelegate.BindLambda([this, bRetryUntilAcceptingPlayers](Worker_EntityQueryResponseOp& Op)
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
			bool bNewAcceptingPlayers = GetAcceptingPlayersFromQueryResponse(Op);

			if (!bNewAcceptingPlayers && bRetryUntilAcceptingPlayers)
			{
				UE_LOG(LogGlobalStateManager, Log, TEXT("Not yet accepting new players. Will retry query for GSM."));
				RetryQueryGSM(bRetryUntilAcceptingPlayers);
			}
			else
			{
				ApplyDeploymentMapDataFromQueryResponse(Op);
			}

			return;
		}

		if (bRetryUntilAcceptingPlayers)
		{
			RetryQueryGSM(bRetryUntilAcceptingPlayers);
		}
	});

	Receiver->AddEntityQueryDelegate(RequestID, GSMQueryDelegate);
}

void UGlobalStateManager::ApplyDeploymentMapDataFromQueryResponse(Worker_EntityQueryResponseOp& Op)
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

bool UGlobalStateManager::GetAcceptingPlayersFromQueryResponse(Worker_EntityQueryResponseOp& Op)
{
	checkf(Op.result_count == 1, TEXT("There should never be more than one GSM"));

	// Iterate over each component on the GSM until we get the DeploymentMap component.
	for (uint32_t i = 0; i < Op.results[0].component_count; i++)
	{
		Worker_ComponentData Data = Op.results[0].components[i];
		if (Data.component_id == SpatialConstants::DEPLOYMENT_MAP_COMPONENT_ID)
		{
			Schema_Object* ComponentObject = Schema_GetComponentDataFields(Data.schema_type);

			if (Schema_GetBoolCount(ComponentObject, SpatialConstants::DEPLOYMENT_MAP_ACCEPTING_PLAYERS_ID) == 1)
			{
				bool bDataAcceptingPlayers = GetBoolFromSchema(ComponentObject, SpatialConstants::DEPLOYMENT_MAP_ACCEPTING_PLAYERS_ID);
				return bDataAcceptingPlayers;
			}
		}
	}

	UE_LOG(LogGlobalStateManager, Warning, TEXT("Entity query response for the GSM did not contain an AcceptingPlayers state."));

	return false;
}

void UGlobalStateManager::RetryQueryGSM(bool bRetryUntilAcceptingPlayers)
{
	// TODO: UNR-656 - TLDR: Hack to get around runtime not giving data on streaming queries unless you have write authority.
	// There is currently a bug in runtime which prevents clients from being able to have read access on the component via the streaming query.
	// This means that the clients never actually receive updates or data on the GSM. To get around this we are making timed entity queries to
	// find the state of the GSM and the accepting players. Remove this work-around when the runtime bug is fixed.
	float RetryTimerDelay = SpatialConstants::ENTITY_QUERY_RETRY_WAIT_SECONDS;

	// In PIE we want to retry the entity query as soon as possible.
#if WITH_EDITOR
	RetryTimerDelay = 0.1f;
#endif

	UE_LOG(LogGlobalStateManager, Log, TEXT("Retrying query for GSM in %f seconds"), RetryTimerDelay);
	FTimerHandle RetryTimer;
	TimerManager->SetTimer(RetryTimer, [this, bRetryUntilAcceptingPlayers]()
	{
		QueryGSM(bRetryUntilAcceptingPlayers);
	}, RetryTimerDelay, false);
}

void UGlobalStateManager::SetDeploymentMapURL(const FString& MapURL)
{
	UE_LOG(LogGlobalStateManager, Log, TEXT("Setting DeploymentMapURL: %s"), *MapURL);
	DeploymentMapURL = MapURL;
}
