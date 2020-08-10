// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "Interop/SpatialPlayerSpawner.h"

#include "EngineClasses/SpatialNetDriver.h"
#include "Interop/Connection/SpatialWorkerConnection.h"
#include "Interop/SpatialReceiver.h"
#include "LoadBalancing/AbstractLBStrategy.h"
#include "Schema/ServerWorker.h"
#include "Schema/UnrealObjectRef.h"
#include "SpatialCommonTypes.h"
#include "SpatialConstants.h"
#include "SpatialGDKSettings.h"
#include "Utils/SchemaUtils.h"

#include "Containers/StringConv.h"
#include "Engine/Engine.h"
#include "Engine/LocalPlayer.h"
#include "GameFramework/GameModeBase.h"
#include "GameFramework/PlayerStart.h"
#include "HAL/Platform.h"
#include "Kismet/GameplayStatics.h"
#include "TimerManager.h"
#include "UObject/SoftObjectPath.h"

#include <WorkerSDK/improbable/c_schema.h>
#include <WorkerSDK/improbable/c_worker.h>

#include <string>

DEFINE_LOG_CATEGORY(LogSpatialPlayerSpawner);

using namespace SpatialGDK;

void USpatialPlayerSpawner::Init(USpatialNetDriver* InNetDriver, FTimerManager* InTimerManager)
{
	NetDriver = InNetDriver;
	TimerManager = InTimerManager;

	NumberOfAttempts = 0;
}

void USpatialPlayerSpawner::SendPlayerSpawnRequest()
{
	// Send an entity query for the SpatialSpawner and bind a delegate so that once it's found, we send a spawn command.
	Worker_Constraint SpatialSpawnerConstraint;
	SpatialSpawnerConstraint.constraint_type = WORKER_CONSTRAINT_TYPE_COMPONENT;
	SpatialSpawnerConstraint.constraint.component_constraint.component_id = SpatialConstants::PLAYER_SPAWNER_COMPONENT_ID;

	Worker_EntityQuery SpatialSpawnerQuery{};
	SpatialSpawnerQuery.constraint = SpatialSpawnerConstraint;
	SpatialSpawnerQuery.result_type = WORKER_RESULT_TYPE_SNAPSHOT;

	const Worker_RequestId RequestID = NetDriver->Connection->SendEntityQueryRequest(&SpatialSpawnerQuery);

	EntityQueryDelegate SpatialSpawnerQueryDelegate;
	SpatialSpawnerQueryDelegate.BindLambda([this, RequestID](const Worker_EntityQueryResponseOp& Op) {
		FString Reason;

		if (Op.status_code != WORKER_STATUS_CODE_SUCCESS)
		{
			Reason = FString::Printf(TEXT("Entity query for SpatialSpawner failed: %s"), UTF8_TO_TCHAR(Op.message));
		}
		else if (Op.result_count == 0)
		{
			Reason = FString::Printf(TEXT("Could not find SpatialSpawner via entity query: %s"), UTF8_TO_TCHAR(Op.message));
		}
		else
		{
			checkf(Op.result_count == 1, TEXT("There should never be more than one SpatialSpawner entity."));

			SpatialGDK::SpawnPlayerRequest SpawnRequest = ObtainPlayerParams();
			Worker_CommandRequest SpawnPlayerCommandRequest = PlayerSpawner::CreatePlayerSpawnRequest(SpawnRequest);
			NetDriver->Connection->SendCommandRequest(Op.results[0].entity_id, &SpawnPlayerCommandRequest,
													  SpatialConstants::PLAYER_SPAWNER_SPAWN_PLAYER_COMMAND_ID);
		}

		if (!Reason.IsEmpty())
		{
			UE_LOG(LogSpatialPlayerSpawner, Error, TEXT("%s"), *Reason);
			OnPlayerSpawnFailed.ExecuteIfBound(Reason);
		}
	});

	UE_LOG(LogSpatialPlayerSpawner, Log, TEXT("Sending player spawn request"));
	NetDriver->Receiver->AddEntityQueryDelegate(RequestID, SpatialSpawnerQueryDelegate);

	++NumberOfAttempts;
}

SpatialGDK::SpawnPlayerRequest USpatialPlayerSpawner::ObtainPlayerParams() const
{
	FURL LoginURL;
	FUniqueNetIdRepl UniqueId;

	const FWorldContext* const WorldContext = GEngine->GetWorldContextFromWorld(NetDriver->GetWorld());
	check(WorldContext->OwningGameInstance);

	const UGameInstance* GameInstance = UGameplayStatics::GetGameInstance(NetDriver);
	const bool bIsSimulatedPlayer = GameInstance ? GameInstance->IsSimulatedPlayer() : false;

	// This code is adapted from PendingNetGame.cpp:242
	if (const ULocalPlayer* LocalPlayer = WorldContext->OwningGameInstance->GetFirstGamePlayer())
	{
		// Send the player nickname if available
		const FString OverrideName = LocalPlayer->GetNickname();
		if (OverrideName.Len() > 0)
		{
			LoginURL.AddOption(*FString::Printf(TEXT("Name=%s"), *OverrideName));
		}

		LoginURL.AddOption(
			*FString::Printf(TEXT("workerAttribute=%s"), *FString::Format(TEXT("workerId:{0}"), { NetDriver->Connection->GetWorkerId() })));

		if (bIsSimulatedPlayer)
		{
			LoginURL.AddOption(*FString::Printf(TEXT("simulatedPlayer=1")));
		}

		// Send any game-specific url options for this player
		const FString GameUrlOptions = LocalPlayer->GetGameLoginOptions();
		if (GameUrlOptions.Len() > 0)
		{
			LoginURL.AddOption(*FString::Printf(TEXT("%s"), *GameUrlOptions));
		}
		// Pull in options from the current world URL (to preserve options added to a travel URL)
		const TArray<FString>& LastURLOptions = WorldContext->LastURL.Op;
		for (const FString& Op : LastURLOptions)
		{
			LoginURL.AddOption(*Op);
		}
		LoginURL.Portal = WorldContext->LastURL.Portal;

		// Send the player unique Id at login
		UniqueId = LocalPlayer->GetPreferredUniqueNetId();
	}
	else
	{
		UE_LOG(LogSpatialPlayerSpawner, Error, TEXT("Couldn't get LocalPlayer data from game instance when trying to spawn player."));
	}

	const FName OnlinePlatformName = WorldContext->OwningGameInstance->GetOnlinePlatformName();

	return { LoginURL, UniqueId, OnlinePlatformName, bIsSimulatedPlayer };
}

void USpatialPlayerSpawner::ReceivePlayerSpawnResponseOnClient(const Worker_CommandResponseOp& Op)
{
	if (Op.status_code == WORKER_STATUS_CODE_SUCCESS)
	{
		UE_LOG(LogSpatialPlayerSpawner, Display, TEXT("PlayerSpawn returned from server sucessfully"));
	}
	else if (NumberOfAttempts < SpatialConstants::MAX_NUMBER_COMMAND_ATTEMPTS)
	{
		UE_LOG(LogSpatialPlayerSpawner, Warning, TEXT("Player spawn request failed: \"%s\""), UTF8_TO_TCHAR(Op.message));

		FTimerHandle RetryTimer;
		TimerManager->SetTimer(
			RetryTimer,
			[WeakThis = TWeakObjectPtr<USpatialPlayerSpawner>(this)]() {
				if (USpatialPlayerSpawner* Spawner = WeakThis.Get())
				{
					Spawner->SendPlayerSpawnRequest();
				}
			},
			SpatialConstants::GetCommandRetryWaitTimeSeconds(NumberOfAttempts), false);
	}
	else
	{
		FString Reason = FString::Printf(TEXT("Player spawn request failed too many times. (%u attempts)"),
										 SpatialConstants::MAX_NUMBER_COMMAND_ATTEMPTS);
		UE_LOG(LogSpatialPlayerSpawner, Error, TEXT("%s"), *Reason);
		OnPlayerSpawnFailed.ExecuteIfBound(Reason);
	}
}

void USpatialPlayerSpawner::ReceivePlayerSpawnRequestOnServer(const Worker_CommandRequestOp& Op)
{
	UE_LOG(LogSpatialPlayerSpawner, Log, TEXT("Received PlayerSpawn request on server"));

	const FUTF8ToTCHAR FStringConversion(reinterpret_cast<const ANSICHAR*>(Op.caller_worker_id), strlen(Op.caller_worker_id));
	FString ClientWorkerId(FStringConversion.Length(), FStringConversion.Get());

	// Accept the player if we have not already accepted a player from this worker.
	bool bAlreadyHasPlayer;
	WorkersWithPlayersSpawned.Emplace(ClientWorkerId, &bAlreadyHasPlayer);
	if (bAlreadyHasPlayer)
	{
		UE_LOG(LogSpatialPlayerSpawner, Verbose, TEXT("Ignoring duplicate PlayerSpawn request. Client worker ID: %s"), *ClientWorkerId);
		return;
	}

	Schema_Object* RequestPayload = Schema_GetCommandRequestObject(Op.request.schema_type);
	FindPlayerStartAndProcessPlayerSpawn(RequestPayload, ClientWorkerId);

	Worker_CommandResponse Response = PlayerSpawner::CreatePlayerSpawnResponse();
	NetDriver->Connection->SendCommandResponse(Op.request_id, &Response);
}

void USpatialPlayerSpawner::FindPlayerStartAndProcessPlayerSpawn(Schema_Object* SpawnPlayerRequest,
																 const PhysicalWorkerName& ClientWorkerId)
{
	// If the load balancing strategy dictates that this worker should have authority over the chosen PlayerStart THEN the spawn is handled
	// locally, Else if the the PlayerStart is handled by another worker THEN forward the request to that worker to prevent an initial
	// player migration, Else if a PlayerStart can't be found THEN we could be on the wrong worker type, so forward to the GameMode
	// authoritative server.
	//
	// This implementation depends on:
	// 1) the load-balancing strategy having the same rules for PlayerStart Actors and Characters / Controllers / Player States or,
	// 2) the authoritative virtual worker ID for a PlayerStart Actor not changing during the lifetime of a deployment.
	check(NetDriver->LoadBalanceStrategy != nullptr)

		// We need to specifically extract the URL from the PlayerSpawn request for finding a PlayerStart.
		const FURL Url = PlayerSpawner::ExtractUrlFromPlayerSpawnParams(SpawnPlayerRequest);

	// Find a PlayerStart Actor on this server.
	AActor* PlayerStartActor = NetDriver->GetWorld()->GetAuthGameMode()->FindPlayerStart(nullptr, Url.Portal);

	// If the PlayerStart is authoritative locally, spawn the player locally.
	if (PlayerStartActor != nullptr && NetDriver->LoadBalanceStrategy->ShouldHaveAuthority(*PlayerStartActor))
	{
		UE_LOG(LogSpatialPlayerSpawner, Verbose, TEXT("Handling SpawnPlayerRequest request locally. Client worker ID: %s."),
			   *ClientWorkerId);
		PassSpawnRequestToNetDriver(SpawnPlayerRequest, PlayerStartActor);
		return;
	}

	VirtualWorkerId VirtualWorkerToForwardTo = SpatialConstants::INVALID_VIRTUAL_WORKER_ID;

	// If we can't find a PlayerStart Actor, the PlayerSpawner authoritative worker may be part of a layer
	// which has a limited view of the world and/or shouldn't be processing player spawning. In this case,
	// we attempt to forward to the worker authoritative over the GameMode, as we assume the FindPlayerStart
	// implementation may depend on authoritative game mode logic. We pass a null object ref so that the
	// forwarded worker knows to search for a PlayerStart.
	if (PlayerStartActor == nullptr)
	{
		VirtualWorkerToForwardTo = NetDriver->LoadBalanceStrategy->WhoShouldHaveAuthority(*UGameplayStatics::GetGameMode(GetWorld()));
		if (VirtualWorkerToForwardTo == SpatialConstants::INVALID_VIRTUAL_WORKER_ID)
		{
			UE_LOG(LogSpatialPlayerSpawner, Error,
				   TEXT("The server authoritative over the GameMode could not locate any PlayerStart, this is unsupported."));
		}
	}
	else if (!NetDriver->LoadBalanceStrategy->ShouldHaveAuthority(*PlayerStartActor))
	{
		VirtualWorkerToForwardTo = NetDriver->LoadBalanceStrategy->WhoShouldHaveAuthority(*PlayerStartActor);
		if (VirtualWorkerToForwardTo == SpatialConstants::INVALID_VIRTUAL_WORKER_ID)
		{
			UE_LOG(LogSpatialPlayerSpawner, Error,
				   TEXT("Load-balance strategy returned invalid virtual worker ID for selected PlayerStart Actor: %s"),
				   *GetNameSafe(PlayerStartActor));
		}
	}

	// If the load balancing strategy returns invalid virtual worker IDs for the PlayerStart, we should error.
	if (VirtualWorkerToForwardTo == SpatialConstants::INVALID_VIRTUAL_WORKER_ID)
	{
		UE_LOG(LogSpatialPlayerSpawner, Error, TEXT("Defaulting to normal player spawning flow."));
		PassSpawnRequestToNetDriver(SpawnPlayerRequest, nullptr);
		return;
	}

	ForwardSpawnRequestToStrategizedServer(SpawnPlayerRequest, PlayerStartActor, ClientWorkerId, VirtualWorkerToForwardTo);
}

void USpatialPlayerSpawner::PassSpawnRequestToNetDriver(const Schema_Object* PlayerSpawnData, AActor* PlayerStart)
{
	const SpatialGDK::SpawnPlayerRequest SpawnRequest = PlayerSpawner::ExtractPlayerSpawnParams(PlayerSpawnData);

	AGameModeBase* GameMode = NetDriver->GetWorld()->GetAuthGameMode();

	// Set a prioritized PlayerStart for the new player to spawn at. Passing nullptr is a no-op.
	GameMode->SetPrioritizedPlayerStart(PlayerStart);
	NetDriver->AcceptNewPlayer(SpawnRequest.LoginURL, SpawnRequest.UniqueId, SpawnRequest.OnlinePlatformName);
	GameMode->SetPrioritizedPlayerStart(nullptr);
}

void USpatialPlayerSpawner::ForwardSpawnRequestToStrategizedServer(const Schema_Object* OriginalPlayerSpawnRequest, AActor* PlayerStart,
																   const PhysicalWorkerName& ClientWorkerId,
																   const VirtualWorkerId SpawningVirtualWorker)
{
	UE_LOG(LogSpatialPlayerSpawner, Log,
		   TEXT("Forwarding player spawn request to strategized worker. Client ID: %s. PlayerStart: %s. Strategeized virtual worker %d"),
		   *ClientWorkerId, *GetNameSafe(PlayerStart), SpawningVirtualWorker);

	// Find the server worker entity corresponding to the PlayerStart strategized virtual worker.
	const Worker_EntityId ServerWorkerEntity =
		NetDriver->VirtualWorkerTranslator->GetServerWorkerEntityForVirtualWorker(SpawningVirtualWorker);
	if (ServerWorkerEntity == SpatialConstants::INVALID_ENTITY_ID)
	{
		UE_LOG(LogSpatialPlayerSpawner, Error,
			   TEXT("Player spawning failed. Virtual worker translator returned invalid server worker entity ID. Virtual worker: %d. "
					"Defaulting to normal player spawning flow."),
			   SpawningVirtualWorker);
		PassSpawnRequestToNetDriver(OriginalPlayerSpawnRequest, nullptr);
		return;
	}

	// To pass the PlayerStart Actor to another worker we use a FUnrealObjectRef.
	// The PlayerStartObjectRef can be null if we are trying to just forward the spawn request to the correct worker layer, rather than some
	// specific PlayerStart authoritative worker.
	FUnrealObjectRef PlayerStartObjectRef = FUnrealObjectRef::NULL_OBJECT_REF;
	if (PlayerStart != nullptr)
	{
		const FNetworkGUID PlayerStartGuid = NetDriver->PackageMap->ResolveStablyNamedObject(PlayerStart);
		PlayerStartObjectRef = NetDriver->PackageMap->GetUnrealObjectRefFromNetGUID(PlayerStartGuid);
	}

	// Create a request using the PlayerStart reference and by copying the data from the PlayerSpawn request from the client.
	// The Schema_CommandRequest is constructed separately from the Worker_CommandRequest so we can store it in the outgoing
	// map for future retries.
	Schema_CommandRequest* ForwardSpawnPlayerSchemaRequest = Schema_CreateCommandRequest();
	ServerWorker::CreateForwardPlayerSpawnSchemaRequest(ForwardSpawnPlayerSchemaRequest, PlayerStartObjectRef, OriginalPlayerSpawnRequest,
														ClientWorkerId);
	Worker_CommandRequest ForwardSpawnPlayerRequest =
		ServerWorker::CreateForwardPlayerSpawnRequest(Schema_CopyCommandRequest(ForwardSpawnPlayerSchemaRequest));

	const Worker_RequestId RequestId = NetDriver->Connection->SendCommandRequest(
		ServerWorkerEntity, &ForwardSpawnPlayerRequest, SpatialConstants::SERVER_WORKER_FORWARD_SPAWN_REQUEST_COMMAND_ID);

	OutgoingForwardPlayerSpawnRequests.Add(RequestId,
										   TUniquePtr<Schema_CommandRequest, ForwardSpawnRequestDeleter>(ForwardSpawnPlayerSchemaRequest));
}

void USpatialPlayerSpawner::ReceiveForwardedPlayerSpawnRequest(const Worker_CommandRequestOp& Op)
{
	Schema_Object* Payload = Schema_GetCommandRequestObject(Op.request.schema_type);
	Schema_Object* PlayerSpawnData = Schema_GetObject(Payload, SpatialConstants::FORWARD_SPAWN_PLAYER_DATA_ID);
	FString ClientWorkerId = GetStringFromSchema(Payload, SpatialConstants::FORWARD_SPAWN_PLAYER_CLIENT_WORKER_ID);

	// Accept the player if we have not already accepted a player from this worker.
	bool bAlreadyHasPlayer;
	WorkersWithPlayersSpawned.Emplace(ClientWorkerId, &bAlreadyHasPlayer);
	if (bAlreadyHasPlayer)
	{
		UE_LOG(LogSpatialPlayerSpawner, Verbose, TEXT("Ignoring duplicate forward player spawn request. Client worker ID: %s"),
			   *ClientWorkerId);
		return;
	}

	bool bRequestHandledSuccessfully = true;

	const FUnrealObjectRef PlayerStartRef = GetObjectRefFromSchema(Payload, SpatialConstants::FORWARD_SPAWN_PLAYER_START_ACTOR_ID);
	if (PlayerStartRef != FUnrealObjectRef::NULL_OBJECT_REF)
	{
		bool bUnresolvedRef = false;
		AActor* PlayerStart = Cast<AActor>(FUnrealObjectRef::ToObjectPtr(PlayerStartRef, NetDriver->PackageMap, bUnresolvedRef));
		bRequestHandledSuccessfully = !bUnresolvedRef;

		if (bRequestHandledSuccessfully)
		{
			UE_LOG(LogSpatialPlayerSpawner, Log, TEXT("Received ForwardPlayerSpawn request. Client worker ID: %s. PlayerStart: %s"),
				   *ClientWorkerId, *PlayerStart->GetName());
			PassSpawnRequestToNetDriver(PlayerSpawnData, PlayerStart);
		}
		else
		{
			UE_LOG(LogSpatialPlayerSpawner, Error,
				   TEXT("PlayerStart Actor UnrealObjectRef was invalid on forwarded player spawn request worker: %s"), *ClientWorkerId);
		}
	}
	else
	{
		UE_LOG(
			LogSpatialPlayerSpawner, Log,
			TEXT("PlayerStart Actor was null object ref in forward spawn request. This is intentional when handing request to the correct "
				 "load balancing layer. Attempting to find a player start again."));
		FindPlayerStartAndProcessPlayerSpawn(PlayerSpawnData, ClientWorkerId);
	}

	Worker_CommandResponse Response = ServerWorker::CreateForwardPlayerSpawnResponse(bRequestHandledSuccessfully);
	NetDriver->Connection->SendCommandResponse(Op.request_id, &Response);
}

void USpatialPlayerSpawner::ReceiveForwardPlayerSpawnResponse(const Worker_CommandResponseOp& Op)
{
	if (Op.status_code == WORKER_STATUS_CODE_SUCCESS)
	{
		const bool bForwardingSucceeding = GetBoolFromSchema(Schema_GetCommandResponseObject(Op.response.schema_type),
															 SpatialConstants::FORWARD_SPAWN_PLAYER_RESPONSE_SUCCESS_ID);
		if (bForwardingSucceeding)
		{
			// If forwarding the player spawn request succeeded, clean up our outgoing request map.
			UE_LOG(LogSpatialPlayerSpawner, Display, TEXT("Forwarding player spawn succeeded"));
			OutgoingForwardPlayerSpawnRequests.Remove(Op.request_id);
		}
		else
		{
			// If the forwarding failed, e.g. if the chosen PlayerStart Actor was deleted on the other server,
			// then try spawning again.
			RetryForwardSpawnPlayerRequest(Op.entity_id, Op.request_id, true);
		}
		return;
	}

	UE_LOG(LogSpatialPlayerSpawner, Warning, TEXT("ForwardPlayerSpawn request failed: \"%s\". Retrying"), UTF8_TO_TCHAR(Op.message));

	FTimerHandle RetryTimer;
	TimerManager->SetTimer(
		RetryTimer,
		[EntityId = Op.entity_id, RequestId = Op.request_id, WeakThis = TWeakObjectPtr<USpatialPlayerSpawner>(this)]() {
			if (USpatialPlayerSpawner* Spawner = WeakThis.Get())
			{
				Spawner->RetryForwardSpawnPlayerRequest(EntityId, RequestId);
			}
		},
		SpatialConstants::GetCommandRetryWaitTimeSeconds(SpatialConstants::FORWARD_PLAYER_SPAWN_COMMAND_WAIT_SECONDS), false);
}

void USpatialPlayerSpawner::RetryForwardSpawnPlayerRequest(const Worker_EntityId EntityId, const Worker_RequestId RequestId,
														   const bool bShouldTryDifferentPlayerStart)
{
	// If the forward request data doesn't exist, we assume the command actually succeeded previously and this failure is spurious.
	if (!OutgoingForwardPlayerSpawnRequests.Contains(RequestId))
	{
		return;
	}

	const auto OldRequest = OutgoingForwardPlayerSpawnRequests.FindAndRemoveChecked(RequestId);
	Schema_Object* OldRequestPayload = Schema_GetCommandRequestObject(OldRequest.Get());

	// If the chosen PlayerStart is deleted or being deleted, we will pick another.
	const FUnrealObjectRef PlayerStartRef =
		GetObjectRefFromSchema(OldRequestPayload, SpatialConstants::FORWARD_SPAWN_PLAYER_START_ACTOR_ID);
	const TWeakObjectPtr<UObject> PlayerStart = NetDriver->PackageMap->GetObjectFromUnrealObjectRef(PlayerStartRef);
	if (bShouldTryDifferentPlayerStart || !PlayerStart.IsValid() || PlayerStart->IsPendingKill())
	{
		UE_LOG(LogSpatialPlayerSpawner, Warning,
			   TEXT("Target PlayerStart to spawn player was no longer valid after forwarding failed. Finding another PlayerStart."));
		Schema_Object* SpawnPlayerData = Schema_GetObject(OldRequestPayload, SpatialConstants::FORWARD_SPAWN_PLAYER_DATA_ID);
		const PhysicalWorkerName& ClientWorkerId =
			GetStringFromSchema(OldRequestPayload, SpatialConstants::FORWARD_SPAWN_PLAYER_CLIENT_WORKER_ID);
		FindPlayerStartAndProcessPlayerSpawn(SpawnPlayerData, ClientWorkerId);
		return;
	}

	// Resend the ForwardSpawnPlayer request.
	Worker_CommandRequest ForwardSpawnPlayerRequest =
		ServerWorker::CreateForwardPlayerSpawnRequest(Schema_CopyCommandRequest(OldRequest.Get()));
	const Worker_RequestId NewRequestId = NetDriver->Connection->SendCommandRequest(
		EntityId, &ForwardSpawnPlayerRequest, SpatialConstants::SERVER_WORKER_FORWARD_SPAWN_REQUEST_COMMAND_ID);

	// Move the request data from the old request ID map entry across to the new ID entry.
	OutgoingForwardPlayerSpawnRequests.Add(NewRequestId, TUniquePtr<Schema_CommandRequest, ForwardSpawnRequestDeleter>(OldRequest.Get()));
}
