// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "Interop/SpatialPlayerSpawner.h"

#include "EngineClasses/SpatialNetDriver.h"
#include "Interop/Connection/SpatialWorkerConnection.h"
#include "Interop/SpatialReceiver.h"
#include "LoadBalancing/AbstractLBStrategy.h"
#include "Schema/PlayerSpawner.h"
#include "Schema/ServerWorker.h"
#include "Schema/UnrealObjectRef.h"
#include "SpatialConstants.h"
#include "SpatialGDKSettings.h"
#include "Utils/SchemaUtils.h"

#include "Engine/Engine.h"
#include "Engine/LocalPlayer.h"
#include "GameFramework/GameModeBase.h"
#include "GameFramework/PlayerStart.h"
#include "Kismet/GameplayStatics.h"
#include "TimerManager.h"

#include <WorkerSDK/improbable/c_schema.h>
#include <WorkerSDK/improbable/c_worker.h>

DEFINE_LOG_CATEGORY(LogSpatialPlayerSpawner);

using namespace SpatialGDK;

void USpatialPlayerSpawner::Init(USpatialNetDriver* InNetDriver, FTimerManager* InTimerManager)
{
	NetDriver = InNetDriver;
	TimerManager = InTimerManager;

	NumberOfAttempts = 0;
}

void USpatialPlayerSpawner::ReceivePlayerSpawnRequestOnServer(const Worker_CommandRequestOp& Op)
{
	FString WorkerID(UTF8_TO_TCHAR(Op.caller_worker_id));

	// Accept the player if we have not already accepted a player from this worker.
	bool bAlreadyHasPlayer;
	WorkersWithPlayersSpawned.Emplace(WorkerID, &bAlreadyHasPlayer);
	if (bAlreadyHasPlayer)
	{
		return;
	}

	Schema_Object* RequestPayload = Schema_GetCommandRequestObject(Op.request.schema_type);
	FindPlayerStartAndProcessPlayerSpawn(RequestPayload);

	const Worker_CommandResponse Response = PlayerSpawner::CreatePlayerSpawnResponse();
	NetDriver->Connection->SendCommandResponse(Op.request_id, &Response);
}

void USpatialPlayerSpawner::FindPlayerStartAndProcessPlayerSpawn(Schema_Object* SpawnPlayerRequest)
{
	// We need to specifically extract the URL from the PlayerSpawn request for finding a PlayerStart.
	const FURL Url = PlayerSpawner::ExtractUrlFromPlayerSpawnParams(SpawnPlayerRequest);
	const AActor* PlayerStartActor = NetDriver->GetWorld()->GetAuthGameMode()->FindPlayerStart(nullptr, Url.Portal);

	// If load-balancing is enabled AND the strategy dictates that another worker should have authority over
	// the chosen PlayerStart THEN the spawn request is forwarded to that worker to prevent an initial player
	// migration. Immediate player migrations can still happen if
	// 1) the load-balancing strategy has different rules for PlayerStart Actors and Characters / Controllers /
	// Player States or,
	// 2) the load-balancing strategy can change the authoritative virtual worker ID for a PlayerStart Actor
	// during the lifetime of a deployment.
	if (GetDefault<USpatialGDKSettings>()->bEnableUnrealLoadBalancer)
	{
		check(NetDriver->LoadBalanceStrategy != nullptr);
		if (!NetDriver->LoadBalanceStrategy->ShouldHaveAuthority(*PlayerStartActor))
		{
			// If we fail to forward the spawn request, we default to the normal player spawning flow.
			const bool bSuccessfullyForwardedRequest = ForwardSpawnRequestToStrategizedServer(SpawnPlayerRequest, PlayerStartActor);
			if (bSuccessfullyForwardedRequest)
			{
				return;
			}
		}
	}

	PassSpawnRequestToNetDriver(SpawnPlayerRequest, PlayerStartActor);
}

void USpatialPlayerSpawner::ReceiveForwardedPlayerSpawnRequest(const Worker_CommandRequestOp& Op)
{
	Schema_Object* Payload = Schema_GetCommandRequestObject(Op.request.schema_type);
	Schema_Object* PlayerSpawnData = Schema_GetObject(Payload, SpatialConstants::FORWARD_SPAWN_PLAYER_DATA_ID);
	FString ClientWorkerID = GetStringFromSchema(PlayerSpawnData, SpatialConstants::SPAWN_PLAYER_CLIENT_WORKER_ID);

	// Accept the player if we have not already accepted a player from this worker.
	bool bAlreadyHasPlayer;
	WorkersWithPlayersSpawned.Emplace(ClientWorkerID, &bAlreadyHasPlayer);
	if (bAlreadyHasPlayer)
	{
		return;
	}

	FUnrealObjectRef PlayerStartRef = GetObjectRefFromSchema(Payload, SpatialConstants::FORWARD_SPAWN_PLAYER_START_ACTOR_ID);
	bool bRefIsUnresolved;
	AActor* PlayerStart = Cast<AActor>(FUnrealObjectRef::ToObjectPtr(PlayerStartRef, NetDriver->PackageMap, bRefIsUnresolved));
	if (PlayerStart == nullptr || bRefIsUnresolved)
	{
		UE_LOG(LogSpatialPlayerSpawner, Error, TEXT("PlayerStart Actor UnrealObjectRef was invalid on forwarded player spawn request worker: %s. Defaulting to normal player spawning flow."), *ClientWorkerID);
	}

	PassSpawnRequestToNetDriver(PlayerSpawnData, PlayerStart);

	Worker_CommandResponse Response = ServerWorker::CreateForwardPlayerSpawnReponse();
	NetDriver->Connection->SendCommandResponse(Op.request_id, &Response);
}

void USpatialPlayerSpawner::PassSpawnRequestToNetDriver(Schema_Object* PlayerSpawnData, const AActor* PlayerStart)
{
	FURL Url;
	FUniqueNetIdRepl UniqueId;
	FName OnlinePlatformName;
	FString ClientWorkerId;
	PlayerSpawner::ExtractPlayerSpawnParams(PlayerSpawnData, Url, UniqueId, OnlinePlatformName, ClientWorkerId);

	NetDriver->AcceptNewPlayer(Url, UniqueId, OnlinePlatformName);
}

// Copies the fields from the SpawnPlayerRequest argument into a ForwardSpawnPlayerRequest (along with the PlayerStart UnrealObjectRef).
bool USpatialPlayerSpawner::ForwardSpawnRequestToStrategizedServer(const Schema_Object* OriginalPlayerSpawnRequest, const AActor* PlayerStart)
{
	// Find which virtual worker should have authority of the PlayerStart.
	const VirtualWorkerId SpawningVirtualWorker = NetDriver->LoadBalanceStrategy->WhoShouldHaveAuthority(*PlayerStart);
	if (SpawningVirtualWorker == SpatialConstants::INVALID_VIRTUAL_WORKER_ID)
	{
		UE_LOG(LogSpatialPlayerSpawner, Error, TEXT("Load-balance strategy returned invalid virtual worker ID for selected PlayerStart Actor: %s. Defaulting to normal player spawning flow."), *GetNameSafe(PlayerStart));
		return false;
	}

	// Find the server worker entity corresponding to the PlayerStart strategized virtual worker.
	const Worker_EntityId ServerWorkerEntity = NetDriver->VirtualWorkerTranslator->GetServerWorkerEntityForVirtualWorker(SpawningVirtualWorker);
	if (ServerWorkerEntity == SpatialConstants::INVALID_ENTITY_ID)
	{
		UE_LOG(LogSpatialPlayerSpawner, Error, TEXT("Virtual worker translator returned invalid server worker entity ID. Virtual worker: %d. Defaulting to normal player spawning flow."), SpawningVirtualWorker);
		return false;
	}

	// To pass the PlayerStart Actor to another worker we use a FUnrealObjectRef.
	FUnrealObjectRef PlayerStartObjectRef = NetDriver->PackageMap->GetUnrealObjectRefFromObject(PlayerStart);

	// Create a request using the PlayerStart reference and by copying the data from the player spawn request from the client.
	// The Schema_CommandRequest is constructed separately from the Worker_CommandRequest so we can store it in the outgoing
	// map for future retries.
	Schema_CommandRequest* ForwardSpawnPlayerSchemaRequest = Schema_CreateCommandRequest();
	ServerWorker::CreateForwardPlayerSpawnSchemaRequest(ForwardSpawnPlayerSchemaRequest, PlayerStartObjectRef, OriginalPlayerSpawnRequest);
	Worker_CommandRequest ForwardSpawnPlayerRequest = ServerWorker::CreateForwardPlayerSpawnRequest(Schema_CopyCommandRequest(ForwardSpawnPlayerSchemaRequest));

	Worker_RequestId RequestId = NetDriver->Connection->SendCommandRequest(ServerWorkerEntity, &ForwardSpawnPlayerRequest, SpatialConstants::SERVER_WORKER_FORWARD_SPAWN_REQUEST_COMMAND_ID);

	OutgoingForwardPlayerSpawnRequests.Add(RequestId, TUniquePtr<Schema_CommandRequest, ForwardSpawnRequestDeleter>(ForwardSpawnPlayerSchemaRequest));

	return true;
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

	Worker_RequestId RequestID;
	RequestID = NetDriver->Connection->SendEntityQueryRequest(&SpatialSpawnerQuery);

	EntityQueryDelegate SpatialSpawnerQueryDelegate;
	SpatialSpawnerQueryDelegate.BindLambda([this, RequestID](const Worker_EntityQueryResponseOp& Op)
	{
		if (Op.status_code != WORKER_STATUS_CODE_SUCCESS)
		{
			UE_LOG(LogSpatialPlayerSpawner, Error, TEXT("Entity query for SpatialSpawner failed: %s"), UTF8_TO_TCHAR(Op.message));
		}
		else if (Op.result_count == 0)
		{
			UE_LOG(LogSpatialPlayerSpawner, Error, TEXT("Could not find SpatialSpawner via entity query: %s"), UTF8_TO_TCHAR(Op.message));
		}
		else
		{
			checkf(Op.result_count == 1, TEXT("There should never be more than one SpatialSpawner entity."));

			// Construct and send the player spawn request.
			FURL LoginURL;
			FUniqueNetIdRepl UniqueId;
			FName OnlinePlatformName;
			bool bIsSimulatedPlayer;
			FString ClientWorkerId;
			ObtainPlayerParams(LoginURL, UniqueId, OnlinePlatformName, bIsSimulatedPlayer, ClientWorkerId);

			Worker_CommandRequest SpawnPlayerCommandRequest = PlayerSpawner::CreatePlayerSpawnRequest(LoginURL, UniqueId, OnlinePlatformName, bIsSimulatedPlayer, ClientWorkerId);
			NetDriver->Connection->SendCommandRequest(Op.results[0].entity_id, &SpawnPlayerCommandRequest, SpatialConstants::PLAYER_SPAWNER_SPAWN_PLAYER_COMMAND_ID);
		}
	});

	UE_LOG(LogSpatialPlayerSpawner, Log, TEXT("Sending player spawn request"));
	NetDriver->Receiver->AddEntityQueryDelegate(RequestID, SpatialSpawnerQueryDelegate);

	++NumberOfAttempts;
}

void USpatialPlayerSpawner::ReceivePlayerSpawnResponseOnClient(const Worker_CommandResponseOp& Op)
{
	if (Op.status_code == WORKER_STATUS_CODE_SUCCESS)
	{
		UE_LOG(LogSpatialPlayerSpawner, Display, TEXT("Player spawned sucessfully"));
	}
	else if (NumberOfAttempts < SpatialConstants::MAX_NUMBER_COMMAND_ATTEMPTS)
	{
		UE_LOG(LogSpatialPlayerSpawner, Warning, TEXT("Player spawn request failed: \"%s\""),
			UTF8_TO_TCHAR(Op.message));

		FTimerHandle RetryTimer;
		TimerManager->SetTimer(RetryTimer, [WeakThis = TWeakObjectPtr<USpatialPlayerSpawner>(this)]()
		{
			if (USpatialPlayerSpawner* Spawner = WeakThis.Get())
			{
				Spawner->SendPlayerSpawnRequest();
			}
		}, SpatialConstants::GetCommandRetryWaitTimeSeconds(NumberOfAttempts), false);
	}
	else
	{
		UE_LOG(LogSpatialPlayerSpawner, Error, TEXT("Player spawn request failed too many times. (%u attempts)"),
			SpatialConstants::MAX_NUMBER_COMMAND_ATTEMPTS);
	}
}

void USpatialPlayerSpawner::ReceiveForwardPlayerSpawnResponse(const Worker_CommandResponseOp& Op)
{
	// If forwarding the player spawn request succeeded, clean up our outgoing request map.
	if (Op.status_code == WORKER_STATUS_CODE_SUCCESS)
	{
		UE_LOG(LogSpatialPlayerSpawner, Display, TEXT("Forwarding player spawn suceeded"));
		OutgoingForwardPlayerSpawnRequests.Remove(Op.request_id);
		return;
	}

	UE_LOG(LogSpatialPlayerSpawner, Warning, TEXT("Retrying ForwardPlayerSpawn request. Previously it failed: \"%s\""),
			UTF8_TO_TCHAR(Op.message));

	FTimerHandle RetryTimer;
	TimerManager->SetTimer(RetryTimer, [EntityId = Op.entity_id, RequestId = Op.request_id, WeakThis = TWeakObjectPtr<USpatialPlayerSpawner>(this)]()
	{
		if (USpatialPlayerSpawner* Spawner = WeakThis.Get())
		{
			// If the forward request data doesn't exist, we assume the command actually succeeded previously and this failure is spurious.
			if (Spawner->OutgoingForwardPlayerSpawnRequests.Contains(RequestId))
			{
				Schema_CommandRequest* OldRequest = Spawner->OutgoingForwardPlayerSpawnRequests.FindAndRemoveChecked(RequestId).Get();
				Schema_Object* OldRequestPayload = Schema_GetCommandRequestObject(OldRequest);

				// If the chosen PlayerStart is deleted or being deleted, we will pick another.
				const FUnrealObjectRef PlayerStartRef = GetObjectRefFromSchema(OldRequestPayload, SpatialConstants::FORWARD_SPAWN_PLAYER_START_ACTOR_ID);
				const TWeakObjectPtr<UObject> PlayerStart = Spawner->NetDriver->PackageMap->GetObjectFromUnrealObjectRef(PlayerStartRef);
				if (!PlayerStart.IsValid() || PlayerStart->IsPendingKill())
				{
					Schema_Object* SpawnPlayerData = Schema_GetObject(OldRequestPayload, SpatialConstants::FORWARD_SPAWN_PLAYER_DATA_ID);
					Spawner->FindPlayerStartAndProcessPlayerSpawn(SpawnPlayerData);
					return;
				}

				// Resend the forward spawn player request.
				Worker_CommandRequest ForwardSpawnPlayerRequest = ServerWorker::CreateForwardPlayerSpawnRequest(Schema_CopyCommandRequest(OldRequest));
				Worker_RequestId NewRequestId = Spawner->NetDriver->Connection->SendCommandRequest(EntityId, &ForwardSpawnPlayerRequest, SpatialConstants::SERVER_WORKER_FORWARD_SPAWN_REQUEST_COMMAND_ID);

				// Move the request data from the old request ID map entry across to the new ID entry.
				Spawner->OutgoingForwardPlayerSpawnRequests.Add(NewRequestId, TUniquePtr<Schema_CommandRequest, ForwardSpawnRequestDeleter>(OldRequest));
			}
		}
	}, SpatialConstants::GetCommandRetryWaitTimeSeconds(SpatialConstants::FORWARD_PLAYER_SPAWN_COMMAND_WAIT_SECONDS), false);
}

void USpatialPlayerSpawner::ObtainPlayerParams(FURL& OutLoginURL, FUniqueNetIdRepl& OutUniqueId, FName& OutOnlinePlatformName, bool& OutIsSimulatedPlayer, FString& OutClientWorkerId) const
{
	const FWorldContext* WorldContext = GEngine->GetWorldContextFromWorld(NetDriver->GetWorld());
	check(WorldContext->OwningGameInstance);

	// This code is adapted from PendingNetGame.cpp:242
	if (const ULocalPlayer* LocalPlayer = WorldContext->OwningGameInstance->GetFirstGamePlayer())
	{
		// Send the player nickname if available
		FString OverrideName = LocalPlayer->GetNickname();
		if (OverrideName.Len() > 0)
		{
			OutLoginURL.AddOption(*FString::Printf(TEXT("Name=%s"), *OverrideName));
		}

		// Send any game-specific url options for this player
		const FString GameUrlOptions = LocalPlayer->GetGameLoginOptions();
		if (GameUrlOptions.Len() > 0)
		{
			OutLoginURL.AddOption(*FString::Printf(TEXT("%s"), *GameUrlOptions));
		}
		// Pull in options from the current world URL (to preserve options added to a travel URL)
		const TArray<FString>& LastURLOptions = WorldContext->LastURL.Op;
		for (const FString& Op : LastURLOptions)
		{
			OutLoginURL.AddOption(*Op);
		}
		LoginURL.Portal = WorldContext->LastURL.Portal;

		// Send the player unique Id at login
		OutUniqueId = LocalPlayer->GetPreferredUniqueNetId();
	}

	OutOnlinePlatformName = WorldContext->OwningGameInstance->GetOnlinePlatformName();

	const UGameInstance* GameInstance = UGameplayStatics::GetGameInstance(NetDriver);
	OutIsSimulatedPlayer = GameInstance ? GameInstance->IsSimulatedPlayer() : false;

	OutClientWorkerId = NetDriver->Connection->GetWorkerId();
}
