// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "Interop/GlobalStateManager.h"

#if WITH_EDITOR
#include "Editor.h"
#include "Settings/LevelEditorPlaySettings.h"
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
#include "LoadBalancing/AbstractLBStrategy.h"
#include "Schema/ServerWorker.h"
#include "Schema/UnrealMetadata.h"
#include "SpatialConstants.h"
#include "UObject/UObjectGlobals.h"
#include "Utils/EntityPool.h"
#include "Utils/SpatialStatics.h"

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
	bTranslationQueryInFlight = false;
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
	const bool bHasReceivedStartupActorData =
		StaticComponentView->HasComponent(GlobalStateManagerEntityId, SpatialConstants::STARTUP_ACTOR_MANAGER_COMPONENT_ID);
	const bool bWorkerEntityReady =
		NetDriver->WorkerEntityId != SpatialConstants::INVALID_ENTITY_ID
		&& StaticComponentView->HasAuthority(NetDriver->WorkerEntityId, SpatialConstants::SERVER_WORKER_COMPONENT_ID);

	if (bHasSentReadyForVirtualWorkerAssignment || !bHasReceivedStartupActorData || !bWorkerEntityReady)
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

	NetDriver->Connection->SendCommandRequest(GlobalStateManagerEntityId, &CommandRequest,
											  SpatialConstants::SHUTDOWN_MULTI_PROCESS_REQUEST_ID);
}

void UGlobalStateManager::ReceiveShutdownMultiProcessRequest()
{
	if (NetDriver && NetDriver->GetNetMode() == NM_DedicatedServer)
	{
		UE_LOG(LogGlobalStateManager, Log, TEXT("Received shutdown multi-process request."));

		// Since the server works are shutting down, set reset the accepting_players flag to false to prevent race conditions  where the
		// client connects quicker than the server.
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
		UE_LOG(LogGlobalStateManager, Warning,
			   TEXT("Tried to send shutdown_additional_servers event on the GSM but this worker does not have authority."));
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

void UGlobalStateManager::SetDeploymentState()
{
	check(NetDriver->StaticComponentView->HasAuthority(GlobalStateManagerEntityId, SpatialConstants::DEPLOYMENT_MAP_COMPONENT_ID));

	UWorld* CurrentWorld = NetDriver->GetWorld();

	// Send the component update that we can now accept players.
	UE_LOG(LogGlobalStateManager, Log, TEXT("Setting deployment URL to '%s'"), *CurrentWorld->URL.Map);
	UE_LOG(LogGlobalStateManager, Log, TEXT("Setting schema hash to '%u'"),
		   NetDriver->ClassInfoManager->SchemaDatabase->SchemaDescriptorHash);

	FWorkerComponentUpdate Update = {};
	Update.component_id = SpatialConstants::DEPLOYMENT_MAP_COMPONENT_ID;
	Update.schema_type = Schema_CreateComponentUpdate();
	Schema_Object* UpdateObject = Schema_GetComponentUpdateFields(Update.schema_type);

	// Set the map URL on the GSM.
	AddStringToSchema(UpdateObject, SpatialConstants::DEPLOYMENT_MAP_MAP_URL_ID, CurrentWorld->RemovePIEPrefix(CurrentWorld->URL.Map));

	// Set the schema hash for connecting workers to check against
	Schema_AddUint32(UpdateObject, SpatialConstants::DEPLOYMENT_MAP_SCHEMA_HASH,
					 NetDriver->ClassInfoManager->SchemaDatabase->SchemaDescriptorHash);

	// Component updates are short circuited so we set the updated state here and then send the component update.
	NetDriver->Connection->SendComponentUpdate(GlobalStateManagerEntityId, &Update);
}

void UGlobalStateManager::SetAcceptingPlayers(bool bInAcceptingPlayers)
{
	// We should only be able to change whether we're accepting players if:
	// - we're authoritative over the DeploymentMap which has the acceptingPlayers property,
	// - we've called BeginPlay (so startup Actors can do initialization before any spawn requests are received),
	// - we aren't duplicating the current state.
	const bool bHasDeploymentMapAuthority =
		NetDriver->StaticComponentView->HasAuthority(GlobalStateManagerEntityId, SpatialConstants::DEPLOYMENT_MAP_COMPONENT_ID);
	const bool bHasBegunPlay = NetDriver->GetWorld()->HasBegunPlay();
	const bool bIsDuplicatingCurrentState = bAcceptingPlayers == bInAcceptingPlayers;
	if (!bHasDeploymentMapAuthority || !bHasBegunPlay || bIsDuplicatingCurrentState)
	{
		return;
	}

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
	UE_LOG(LogGlobalStateManager, Verbose, TEXT("Authority over the GSM component %d has changed. This worker %s authority."),
		   AuthOp.component_id, AuthOp.authority == WORKER_AUTHORITY_AUTHORITATIVE ? TEXT("now has") : TEXT("does not have"));

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
		SetAcceptingPlayers(true);
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
	UE_LOG(LogGlobalStateManager, Display,
		   TEXT("GlobalStateManager not accepting players and resetting BeginPlay lifecycle properties. Session restarting."));

	SetAcceptingPlayers(false);

	// Reset the BeginPlay flag so Startup Actors are properly managed.
	SendCanBeginPlayUpdate(false);
}

void UGlobalStateManager::BeginDestroy()
{
	Super::BeginDestroy();

#if WITH_EDITOR
	if (NetDriver != nullptr
		&& NetDriver->StaticComponentView->HasAuthority(GlobalStateManagerEntityId, SpatialConstants::STARTUP_ACTOR_MANAGER_COMPONENT_ID))
	{
		// If we are deleting dynamically spawned entities, we need to
		if (GetDefault<ULevelEditorPlaySettings>()->GetDeleteDynamicEntities())
		{
			// Reset the BeginPlay flag so Startup Actors are properly managed.
			SendCanBeginPlayUpdate(false);

			// Flush the connection and wait a moment to allow the message to propagate.
			// TODO: UNR-3697 - This needs to be handled more correctly
			NetDriver->Connection->Flush();
			FPlatformProcess::Sleep(0.1f);
		}
	}
#endif
}

void UGlobalStateManager::SetAllActorRolesBasedOnLBStrategy()
{
	for (TActorIterator<AActor> It(NetDriver->World); It; ++It)
	{
		AActor* Actor = *It;
		if (Actor != nullptr && !Actor->IsPendingKill())
		{
			if (Actor->GetIsReplicated())
			{
				const bool bAuthoritative = NetDriver->LoadBalanceStrategy->ShouldHaveAuthority(*Actor);
				Actor->Role = bAuthoritative ? ROLE_Authority : ROLE_SimulatedProxy;
				Actor->RemoteRole = bAuthoritative ? ROLE_SimulatedProxy : ROLE_Authority;
			}
		}
	}
}

void UGlobalStateManager::TriggerBeginPlay()
{
	const bool bHasStartupActorAuthority =
		NetDriver->StaticComponentView->HasAuthority(GlobalStateManagerEntityId, SpatialConstants::STARTUP_ACTOR_MANAGER_COMPONENT_ID);
	if (bHasStartupActorAuthority)
	{
		SendCanBeginPlayUpdate(true);
	}

	// This method has early exits internally to ensure the logic is only executed on the correct worker.
	SetAcceptingPlayers(true);

	// If we're loading from a snapshot, we shouldn't try and call BeginPlay with authority.
	if (bCanSpawnWithAuthority)
	{
		SetAllActorRolesBasedOnLBStrategy();
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
	return GetCanBeginPlay()
		   || NetDriver->StaticComponentView->HasAuthority(GlobalStateManagerEntityId,
														   SpatialConstants::STARTUP_ACTOR_MANAGER_COMPONENT_ID);
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
// bRetryUntilRecievedExpectedValues will continue querying until the state of AcceptingPlayers and SessionId are the same as the given
// arguments This is so clients know when to connect to the deployment.
void UGlobalStateManager::QueryGSM(const QueryDelegate& Callback)
{
	// Build a constraint for the GSM.
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
	GSMQueryDelegate.BindLambda([this, Callback](const Worker_EntityQueryResponseOp& Op) {
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
			ApplyDeploymentMapDataFromQueryResponse(Op);
			Callback.ExecuteIfBound(Op);
		}
	});

	Receiver->AddEntityQueryDelegate(RequestID, GSMQueryDelegate);
}

void UGlobalStateManager::QueryTranslation()
{
	if (bTranslationQueryInFlight)
	{
		// Only allow one in flight query. Retries will be handled by the SpatialNetDriver.
		return;
	}

	// Build a constraint for the Virtual Worker Translation.
	Worker_ComponentConstraint TranslationComponentConstraint{};
	TranslationComponentConstraint.component_id = SpatialConstants::VIRTUAL_WORKER_TRANSLATION_COMPONENT_ID;

	Worker_Constraint TranslationConstraint{};
	TranslationConstraint.constraint_type = WORKER_CONSTRAINT_TYPE_COMPONENT;
	TranslationConstraint.constraint.component_constraint = TranslationComponentConstraint;

	Worker_EntityQuery TranslationQuery{};
	TranslationQuery.constraint = TranslationConstraint;
	TranslationQuery.result_type = WORKER_RESULT_TYPE_SNAPSHOT;

	Worker_RequestId RequestID = NetDriver->Connection->SendEntityQueryRequest(&TranslationQuery);
	bTranslationQueryInFlight = true;

	TWeakObjectPtr<UGlobalStateManager> WeakGlobalStateManager(this);
	EntityQueryDelegate TranslationQueryDelegate;
	TranslationQueryDelegate.BindLambda([WeakGlobalStateManager](const Worker_EntityQueryResponseOp& Op) {
		if (!WeakGlobalStateManager.IsValid())
		{
			// The GSM was destroyed before receiving the response.
			return;
		}

		UGlobalStateManager* GlobalStateManager = WeakGlobalStateManager.Get();
		if (Op.status_code == WORKER_STATUS_CODE_SUCCESS)
		{
			if (GlobalStateManager->NetDriver->VirtualWorkerTranslator.IsValid())
			{
				GlobalStateManager->ApplyVirtualWorkerMappingFromQueryResponse(Op);
			}
		}
		GlobalStateManager->bTranslationQueryInFlight = false;
	});
	Receiver->AddEntityQueryDelegate(RequestID, TranslationQueryDelegate);
}

void UGlobalStateManager::ApplyVirtualWorkerMappingFromQueryResponse(const Worker_EntityQueryResponseOp& Op) const
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

bool UGlobalStateManager::GetAcceptingPlayersAndSessionIdFromQueryResponse(const Worker_EntityQueryResponseOp& Op,
																		   bool& OutAcceptingPlayers, int32& OutSessionId)
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

	UE_LOG(LogGlobalStateManager, Warning,
		   TEXT("Entity query response for the GSM did not contain both AcceptingPlayers and SessionId states."));

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
