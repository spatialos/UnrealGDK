// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "Interop/GlobalStateManager.h"

#if WITH_EDITOR
#include "Editor.h"
#include "Settings/LevelEditorPlaySettings.h"
#endif

#include "Engine/Classes/AI/AISystemBase.h"
#include "Engine/LevelStreaming.h"
#include "Engine/World.h"
#include "EngineClasses/SpatialActorChannel.h"
#include "EngineClasses/SpatialNetConnection.h"
#include "EngineClasses/SpatialNetDriver.h"
#include "EngineClasses/SpatialPackageMapClient.h"
#include "EngineClasses/SpatialVirtualWorkerTranslator.h"
#include "EngineUtils.h"
#include "Interop/Connection/SpatialWorkerConnection.h"
#include "Interop/SpatialReceiver.h"
#include "Interop/SpatialSender.h"
#include "Kismet/GameplayStatics.h"
#include "LoadBalancing/AbstractLBStrategy.h"
#include "Schema/ServerWorker.h"
#include "SpatialConstants.h"
#include "UObject/UObjectGlobals.h"
#include "Utils/SpatialDebugger.h"
#include "Utils/SpatialMetricsDisplay.h"
#include "Utils/SpatialStatics.h"

DEFINE_LOG_CATEGORY(LogGlobalStateManager);

using namespace SpatialGDK;

void UGlobalStateManager::Init(USpatialNetDriver* InNetDriver)
{
	NetDriver = InNetDriver;
	ViewCoordinator = &InNetDriver->Connection->GetCoordinator();
	GlobalStateManagerEntityId = SpatialConstants::INITIAL_GLOBAL_STATE_MANAGER_ENTITY_ID;

#if WITH_EDITOR
	RequestHandler.AddRequestHandler(
		SpatialConstants::GSM_SHUTDOWN_COMPONENT_ID, SpatialConstants::SHUTDOWN_MULTI_PROCESS_REQUEST_ID,
		FOnCommandRequestWithOp::FDelegate::CreateUObject(this, &UGlobalStateManager::OnReceiveShutdownCommand));

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

	bCanSpawnWithAuthority = false;
	bTranslationQueryInFlight = false;
}

void UGlobalStateManager::ApplySessionId(int32 InSessionId)
{
	DeploymentSessionId = InSessionId;
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

	NetDriver->Connection->SendCommandRequest(GlobalStateManagerEntityId, &CommandRequest, RETRY_UNTIL_COMPLETE, {});
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

void UGlobalStateManager::OnReceiveShutdownCommand(const Worker_Op& Op, const Worker_CommandRequestOp& CommandRequestOp)
{
	ReceiveShutdownMultiProcessRequest();

	SpatialEventTracer* EventTracer = NetDriver->Connection->GetEventTracer();

	if (EventTracer != nullptr)
	{
		Worker_RequestId RequestId = Op.op.command_request.request_id;
		EventTracer->TraceEvent(RECEIVE_COMMAND_REQUEST_EVENT_NAME, "", Op.span_id, /* NumCauses */ 1,
								[RequestId](FSpatialTraceEventDataBuilder& EventBuilder) {
									EventBuilder.AddCommand("SHUTDOWN_MULTI_PROCESS_REQUEST");
									EventBuilder.AddRequestId(RequestId);
								});
	}
}

void UGlobalStateManager::OnShutdownComponentUpdate(Schema_ComponentUpdate* Update)
{
	Schema_Object* EventsObject = Schema_GetComponentUpdateEvents(Update);
	// TODO(UNR-4395): Probably should be a bool in state - probably a non-persistent entity
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
	if (!ViewCoordinator->HasAuthority(GlobalStateManagerEntityId, SpatialConstants::GDK_KNOWN_ENTITY_AUTH_COMPONENT_SET_ID))
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

bool UGlobalStateManager::HasAuthority() const
{
	check(ViewCoordinator->HasEntity(GlobalStateManagerEntityId));
	return ViewCoordinator->HasAuthority(GlobalStateManagerEntityId, SpatialConstants::GDK_KNOWN_ENTITY_AUTH_COMPONENT_SET_ID);
}

void UGlobalStateManager::SetDeploymentState()
{
	check(ViewCoordinator->HasAuthority(GlobalStateManagerEntityId, SpatialConstants::GDK_KNOWN_ENTITY_AUTH_COMPONENT_SET_ID));

	UWorld* CurrentWorld = NetDriver->GetWorld();

	// Send the component update that we can now accept players.
	UE_LOG(LogGlobalStateManager, Log, TEXT("Setting deployment URL to '%s'"), *CurrentWorld->URL.Map);
	UE_LOG(LogGlobalStateManager, Log, TEXT("Setting schema hash to '%u'"), NetDriver->ClassInfoManager->SchemaDatabase->SchemaBundleHash);

	FWorkerComponentUpdate Update = {};
	Update.component_id = SpatialConstants::DEPLOYMENT_MAP_COMPONENT_ID;
	Update.schema_type = Schema_CreateComponentUpdate();
	Schema_Object* UpdateObject = Schema_GetComponentUpdateFields(Update.schema_type);

	// Set the map URL on the GSM.
	AddStringToSchema(UpdateObject, SpatialConstants::DEPLOYMENT_MAP_MAP_URL_ID, CurrentWorld->RemovePIEPrefix(CurrentWorld->URL.Map));

	// Set the schema hash for connecting workers to check against
	Schema_AddUint32(UpdateObject, SpatialConstants::DEPLOYMENT_MAP_SCHEMA_HASH,
					 NetDriver->ClassInfoManager->SchemaDatabase->SchemaBundleHash);

	// Component updates are short circuited so we set the updated state here and then send the component update.
	NetDriver->Connection->SendComponentUpdate(GlobalStateManagerEntityId, &Update);
}

void UGlobalStateManager::SetAcceptingPlayers(bool bInAcceptingPlayers)
{
	// We should only be able to change whether we're accepting players if:
	// - we're authoritative over the DeploymentMap which has the acceptingPlayers property,
	// - we've called BeginPlay (so startup Actors can do initialization before any spawn requests are received),
	const bool bHasDeploymentMapAuthority =
		ViewCoordinator->HasAuthority(GlobalStateManagerEntityId, SpatialConstants::GDK_KNOWN_ENTITY_AUTH_COMPONENT_SET_ID);
	const bool bHasBegunPlay = NetDriver->GetWorld()->HasBegunPlay();
	if (!ensure(bHasDeploymentMapAuthority) || !ensure(bHasBegunPlay))
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
	NetDriver->Connection->SendComponentUpdate(GlobalStateManagerEntityId, &Update);
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
		&& ViewCoordinator->HasAuthority(GlobalStateManagerEntityId, SpatialConstants::GDK_KNOWN_ENTITY_AUTH_COMPONENT_SET_ID))
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

void UGlobalStateManager::HandleActorBasedOnLoadBalancer(AActor* Actor) const
{
	if (Actor == nullptr || Actor->IsPendingKill())
	{
		return;
	}

	if (USpatialStatics::IsSpatialOffloadingEnabled(Actor->GetWorld()) && !USpatialStatics::IsActorGroupOwnerForActor(Actor)
		&& !Actor->bNetLoadOnNonAuthServer)
	{
		Actor->Destroy(true);
		return;
	}

	if (!Actor->GetIsReplicated())
	{
		return;
	}

	if (USpatialStatics::IsStrategyWorkerEnabled())
	{
		return;
	}

	// Replicated level Actors should only be initially authority if:
	//  - these are workers starting as part of a fresh deployment (tracked by the bCanSpawnWithAuthority bool),
	//  - these actors are marked as NotPersistent and we're loading from a saved snapshot (which means bCanSpawnWithAuthority is false)
	//  - the load balancing strategy says this server should be authoritative (as opposed to some other server).
	const bool bAuthoritative = (bCanSpawnWithAuthority || Actor->GetClass()->HasAnySpatialClassFlags(SPATIALCLASS_NotPersistent))
								&& NetDriver->LoadBalanceStrategy->ShouldHaveAuthority(*Actor);

	Actor->Role = bAuthoritative ? ROLE_Authority : ROLE_SimulatedProxy;
	Actor->RemoteRole = bAuthoritative ? ROLE_SimulatedProxy : ROLE_Authority;

	UE_LOG(LogGlobalStateManager, Verbose, TEXT("GSM updated actor authority: %s %s."), *Actor->GetPathName(),
		   bAuthoritative ? TEXT("authoritative") : TEXT("not authoritative"));

	// Allocate entity ids for all startup actors
	if (Actor->HasAuthority())
	{
		NetDriver->PackageMap->TryResolveObjectAsEntity(Actor);
	}
}

Worker_EntityId UGlobalStateManager::GetLocalServerWorkerEntityId() const
{
	if (ensure(NetDriver != nullptr))
	{
		return NetDriver->WorkerEntityId;
	}

	return SpatialConstants::INVALID_ENTITY_ID;
}

void UGlobalStateManager::ClaimSnapshotPartition()
{
	CommandsHandler.ClaimPartition(*ViewCoordinator, NetDriver->Connection->GetWorkerSystemEntityId(),
								   SpatialConstants::INITIAL_SNAPSHOT_PARTITION_ENTITY_ID);
}

void UGlobalStateManager::TriggerBeginPlay()
{
#if !UE_BUILD_SHIPPING
	const USpatialGDKSettings* SpatialSettings = GetDefault<USpatialGDKSettings>();
	if (NetDriver->IsServer())
	{
		// If metrics display is enabled, spawn an Actor to replicate the information to each client.
		if (SpatialSettings->bEnableMetricsDisplay)
		{
			NetDriver->SpatialMetricsDisplay = NetDriver->World->SpawnActor<ASpatialMetricsDisplay>();
		}
		if (SpatialSettings->SpatialDebugger != nullptr)
		{
			NetDriver->SpatialDebugger = NetDriver->World->SpawnActor<ASpatialDebugger>(SpatialSettings->SpatialDebugger);
			if (!HasAuthority())
			{
				NetDriver->SpatialDebugger->Role = ROLE_SimulatedProxy;
				NetDriver->SpatialDebugger->RemoteRole = ROLE_Authority;
			}
		}
	}
#endif

	// If we're loading from a snapshot, we shouldn't try and call BeginPlay with authority.
	// We don't use TActorIterator here as it has custom code to ignore sublevel world settings actors, which we want to handle,
	// so we just iterate over all level actors directly.
	for (ULevel* Level : NetDriver->World->GetLevels())
	{
		if (Level != nullptr)
		{
			for (AActor* Actor : Level->Actors)
			{
				HandleActorBasedOnLoadBalancer(Actor);
			}
		}
	}

	const TArray<ULevelStreaming*>& StreamingLevels = NetDriver->World->GetStreamingLevels();
	for (ULevelStreaming* StreamingLevel : StreamingLevels)
	{
		const ULevel* Level = StreamingLevel->GetLoadedLevel();
		if (Level == nullptr)
		{
			UE_LOG(LogGlobalStateManager, Warning, TEXT("Loaded level was nullptr on StreamingLevel %s"), *StreamingLevel->GetPathName());
			continue;
		}
		for (AActor* Actor : Level->Actors)
		{
			HandleActorBasedOnLoadBalancer(Actor);
		}
	}

	NetDriver->World->GetWorldSettings()->SetGSMReadyForPlay();
	NetDriver->World->GetWorldSettings()->NotifyBeginPlay();
}

void UGlobalStateManager::SendCanBeginPlayUpdate(const bool bInCanBeginPlay)
{
	check(ViewCoordinator->HasAuthority(GlobalStateManagerEntityId, SpatialConstants::GDK_KNOWN_ENTITY_AUTH_COMPONENT_SET_ID));

	FWorkerComponentUpdate Update = {};
	Update.component_id = SpatialConstants::STARTUP_ACTOR_MANAGER_COMPONENT_ID;
	Update.schema_type = Schema_CreateComponentUpdate();
	Schema_Object* UpdateObject = Schema_GetComponentUpdateFields(Update.schema_type);

	Schema_AddBool(UpdateObject, SpatialConstants::STARTUP_ACTOR_MANAGER_CAN_BEGIN_PLAY_ID, static_cast<uint8_t>(bInCanBeginPlay));

	NetDriver->Connection->SendComponentUpdate(GlobalStateManagerEntityId, &Update);
}

void UGlobalStateManager::IncrementSessionID()
{
	DeploymentSessionId++;
	SendSessionIdUpdate();
}

void UGlobalStateManager::Advance()
{
	const TArray<Worker_Op>& Ops = NetDriver->Connection->GetCoordinator().GetViewDelta().GetWorkerMessages();

	CommandsHandler.ProcessOps(Ops);

#if WITH_EDITOR
	RequestHandler.ProcessOps(Ops);
#endif // WITH_EDITOR
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
