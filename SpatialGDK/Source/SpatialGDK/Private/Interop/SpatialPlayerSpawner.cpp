// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "Interop/SpatialPlayerSpawner.h"

#include "Engine/Engine.h"
#include "Engine/LocalPlayer.h"
#include "SocketSubsystem.h"
#include "TimerManager.h"

#include "EngineClasses/SpatialNetConnection.h"
#include "EngineClasses/SpatialNetDriver.h"
#include "Interop/Connection/SpatialWorkerConnection.h"
#include "Interop/SpatialReceiver.h"
#include "SpatialConstants.h"
#include "Utils/SchemaUtils.h"

#include <WorkerSDK/improbable/c_schema.h>
#include <WorkerSDK/improbable/c_worker.h>

DEFINE_LOG_CATEGORY(LogSpatialPlayerSpawner);

using namespace improbable;

void USpatialPlayerSpawner::Init(USpatialNetDriver* InNetDriver, FTimerManager* InTimerManager)
{
	NetDriver = InNetDriver;
	TimerManager = InTimerManager;

	NumberOfAttempts = 0;
}

void USpatialPlayerSpawner::ReceivePlayerSpawnRequest(Schema_Object* Payload, const char* CallerAttribute, Worker_RequestId RequestId )
{
	// Extract spawn parameters.
	FString URLString = GetStringFromSchema(Payload, 1);

	FUniqueNetIdRepl UniqueId;
	TArray<uint8> UniqueIdBytes = GetBytesFromSchema(Payload, 2);
	FNetBitReader UniqueIdReader(nullptr, UniqueIdBytes.GetData(), UniqueIdBytes.Num() * 8);
	UniqueIdReader << UniqueId;

	FName OnlinePlatformName = FName(*GetStringFromSchema(Payload, 3));

	// Accept new player.
	URLString.Append(TEXT("?workerAttribute=")).Append(UTF8_TO_TCHAR(CallerAttribute));

	NetDriver->AcceptNewPlayer(FURL(nullptr, *URLString, TRAVEL_Absolute), UniqueId, OnlinePlatformName, false);

	Worker_CommandResponse CommandResponse = {};
	CommandResponse.component_id = SpatialConstants::PLAYER_SPAWNER_COMPONENT_ID;
	CommandResponse.schema_type = Schema_CreateCommandResponse(SpatialConstants::PLAYER_SPAWNER_COMPONENT_ID, 1);
	Schema_Object* ResponseObject = Schema_GetCommandResponseObject(CommandResponse.schema_type);
	Schema_AddBool(ResponseObject, 1, true);

	NetDriver->Connection->SendCommandResponse(RequestId, &CommandResponse);
}

void USpatialPlayerSpawner::SendPlayerSpawnRequest()
{
	// Send an entity query for the SpatialSpawner and bind a delegate so that once it's found, we send a spawn command.
	Worker_Constraint SpatialSpawnerConstraint;
	SpatialSpawnerConstraint.constraint_type = WORKER_CONSTRAINT_TYPE_COMPONENT;
	SpatialSpawnerConstraint.component_constraint.component_id = SpatialConstants::PLAYER_SPAWNER_COMPONENT_ID;

	Worker_EntityQuery SpatialSpawnerQuery{};
	SpatialSpawnerQuery.constraint = SpatialSpawnerConstraint;
	SpatialSpawnerQuery.result_type = WORKER_RESULT_TYPE_SNAPSHOT;

	Worker_RequestId RequestID;
	RequestID = NetDriver->Connection->SendEntityQueryRequest(&SpatialSpawnerQuery);

	EntityQueryDelegate SpatialSpawnerQueryDelegate;
	SpatialSpawnerQueryDelegate.BindLambda([this, RequestID](Worker_EntityQueryResponseOp& Op)
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
			ObtainPlayerParams(LoginURL, UniqueId, OnlinePlatformName);

			Worker_CommandRequest CommandRequest = {};
			CommandRequest.component_id = SpatialConstants::PLAYER_SPAWNER_COMPONENT_ID;
			CommandRequest.schema_type = Schema_CreateCommandRequest(SpatialConstants::PLAYER_SPAWNER_COMPONENT_ID, 1);
			Schema_Object* RequestObject = Schema_GetCommandRequestObject(CommandRequest.schema_type);
			AddStringToSchema(RequestObject, 1, LoginURL.ToString(true));

			// Write player identity information.
			FNetBitWriter UniqueIdWriter(0);
			UniqueIdWriter << UniqueId;
			AddBytesToSchema(RequestObject, 2, UniqueIdWriter);
			AddStringToSchema(RequestObject, 3, OnlinePlatformName.ToString());

			NetDriver->Connection->SendCommandRequest(Op.results[0].entity_id, &CommandRequest, 1);
		}
	});

	UE_LOG(LogSpatialPlayerSpawner, Log, TEXT("Sending player spawn request"));
	NetDriver->Receiver->AddEntityQueryDelegate(RequestID, SpatialSpawnerQueryDelegate);

	++NumberOfAttempts;
}

void USpatialPlayerSpawner::ReceivePlayerSpawnResponse(Worker_CommandResponseOp& Op)
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
		TimerManager->SetTimer(RetryTimer, [this]()
		{
			SendPlayerSpawnRequest();
		}, SpatialConstants::GetCommandRetryWaitTimeSeconds(NumberOfAttempts), false);
	}
	else
	{
		UE_LOG(LogSpatialPlayerSpawner, Error, TEXT("Player spawn request failed too many times. (%u attempts)"),
			SpatialConstants::MAX_NUMBER_COMMAND_ATTEMPTS)
	}
}

void USpatialPlayerSpawner::ObtainPlayerParams(FURL& LoginURL, FUniqueNetIdRepl& OutUniqueId, FName& OutOnlinePlatformName)
{
	const FWorldContext* const WorldContext = GEngine->GetWorldContextFromWorld(NetDriver->GetWorld());
	check(WorldContext->OwningGameInstance);

	// This code is adapted from PendingNetGame.cpp:242
	if (ULocalPlayer* LocalPlayer = WorldContext->OwningGameInstance->GetFirstGamePlayer())
	{
		// Send the player nickname if available
		FString OverrideName = LocalPlayer->GetNickname();
		if (OverrideName.Len() > 0)
		{
			LoginURL.AddOption(*FString::Printf(TEXT("Name=%s"), *OverrideName));
		}

		// Send any game-specific url options for this player
		FString GameUrlOptions = LocalPlayer->GetGameLoginOptions();
		if (GameUrlOptions.Len() > 0)
		{
			LoginURL.AddOption(*FString::Printf(TEXT("%s"), *GameUrlOptions));
		}

		// Send the player unique Id at login
		OutUniqueId = LocalPlayer->GetPreferredUniqueNetId();
	}

	OutOnlinePlatformName = WorldContext->OwningGameInstance->GetOnlinePlatformName();
}
