// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "Schema/Component.h"
#include "SpatialConstants.h"
#include "Utils/SchemaUtils.h"

#include "Containers/UnrealString.h"
#include "GameFramework/OnlineReplStructs.h"
#include "Engine/EngineBaseTypes.h"
#include "Engine/GameInstance.h"
#include "Kismet/GameplayStatics.h"
#include "UObject/CoreNet.h"

#include <WorkerSDK/improbable/c_schema.h>
#include <WorkerSDK/improbable/c_worker.h>

namespace SpatialGDK
{

struct PlayerSpawner : Component
{
	static const Worker_ComponentId ComponentId = SpatialConstants::PLAYER_SPAWNER_COMPONENT_ID;

	PlayerSpawner() = default;

	static Worker_CommandRequest CreatePlayerSpawnRequest(const FURL& LoginURL, FUniqueNetIdRepl& UniqueId, const FName& OnlinePlatformName, const bool bIsSimulatedPlayer, const FString& ClientWorkerId)
	{
		Worker_CommandRequest CommandRequest = {};
		CommandRequest.component_id = SpatialConstants::PLAYER_SPAWNER_COMPONENT_ID;
		CommandRequest.command_index = SpatialConstants::PLAYER_SPAWNER_SPAWN_PLAYER_COMMAND_ID;
		CommandRequest.schema_type = Schema_CreateCommandRequest();

		Schema_Object* RequestFields = Schema_GetCommandRequestObject(CommandRequest.schema_type);
		AddSpawnPlayerData(RequestFields, LoginURL, UniqueId, OnlinePlatformName, bIsSimulatedPlayer, ClientWorkerId);

		return CommandRequest;
	}

	static Worker_CommandResponse CreatePlayerSpawnResponse()
	{
		Worker_CommandResponse CommandResponse = {};
		CommandResponse.component_id = SpatialConstants::PLAYER_SPAWNER_COMPONENT_ID;
		CommandResponse.command_index = SpatialConstants::PLAYER_SPAWNER_SPAWN_PLAYER_COMMAND_ID;
		CommandResponse.schema_type = Schema_CreateCommandResponse();
		return CommandResponse;
	}

	static void AddSpawnPlayerData(Schema_Object* RequestObject, const FURL& LoginURL, FUniqueNetIdRepl& UniqueId, const FName& OnlinePlatformName, const bool bIsSimulatedPlayer, const FString& ClientWorkerId)
	{
		AddStringToSchema(RequestObject, SpatialConstants::SPAWN_PLAYER_URL_ID, *LoginURL.ToString(true));

		// Write player identity information.
		FNetBitWriter UniqueIdWriter(0);
		UniqueIdWriter << UniqueId;
		AddBytesToSchema(RequestObject, SpatialConstants::SPAWN_PLAYER_UNIQUE_ID, UniqueIdWriter);
		AddStringToSchema(RequestObject, SpatialConstants::SPAWN_PLAYER_PLATFORM_NAME_ID, OnlinePlatformName.ToString());
		Schema_AddBool(RequestObject, SpatialConstants::SPAWN_PLAYER_IS_SIMULATED_ID, bIsSimulatedPlayer);
		AddStringToSchema(RequestObject, SpatialConstants::SPAWN_PLAYER_CLIENT_WORKER_ID, ClientWorkerId);
	}

	static FURL ExtractUrlFromPlayerSpawnParams(const Schema_Object* Payload)
	{
		FString URLString = GetStringFromSchema(Payload, SpatialConstants::SPAWN_PLAYER_URL_ID);

		FString ClientWorkerId = GetStringFromSchema(Payload, SpatialConstants::SPAWN_PLAYER_CLIENT_WORKER_ID);
		URLString.Append(TEXT("?workerAttribute=")).Append(ClientWorkerId);

		const bool bIsSimulatedPlayer = GetBoolFromSchema(Payload, SpatialConstants::SPAWN_PLAYER_IS_SIMULATED_ID);
		if (bIsSimulatedPlayer)
		{
			URLString += TEXT("?simulatedPlayer=1");
		}

		return FURL(nullptr, *URLString, TRAVEL_Absolute);
	}

	static void ExtractPlayerSpawnParams(const Schema_Object* CommandRequestPayload, FURL& LoginURL, FUniqueNetIdRepl& UniqueId, FName& OnlinePlatformName, FString& ClientWorkerID)
	{
		LoginURL = ExtractUrlFromPlayerSpawnParams(CommandRequestPayload);

		TArray<uint8> UniqueIdBytes = GetBytesFromSchema(CommandRequestPayload, SpatialConstants::SPAWN_PLAYER_UNIQUE_ID);
		FNetBitReader UniqueIdReader(nullptr, UniqueIdBytes.GetData(), UniqueIdBytes.Num() * 8);
		UniqueIdReader << UniqueId;

		OnlinePlatformName = FName(*GetStringFromSchema(CommandRequestPayload, SpatialConstants::SPAWN_PLAYER_PLATFORM_NAME_ID));

		ClientWorkerID = GetStringFromSchema(CommandRequestPayload, SpatialConstants::SPAWN_PLAYER_CLIENT_WORKER_ID);
	}

	static void CopySpawnDataBetweenObjects(const Schema_Object* SpawnPlayerDataSource, Schema_Object* SpawnPlayerDataDestination)
	{
		AddStringToSchema(SpawnPlayerDataDestination, SpatialConstants::SPAWN_PLAYER_URL_ID, GetStringFromSchema(SpawnPlayerDataSource, SpatialConstants::SPAWN_PLAYER_URL_ID));
		TArray<uint8> UniqueId = GetBytesFromSchema(SpawnPlayerDataSource, SpatialConstants::SPAWN_PLAYER_UNIQUE_ID);
		AddBytesToSchema(SpawnPlayerDataDestination, SpatialConstants::SPAWN_PLAYER_UNIQUE_ID, UniqueId.GetData(), UniqueId.Num());
		AddStringToSchema(SpawnPlayerDataDestination, SpatialConstants::SPAWN_PLAYER_PLATFORM_NAME_ID, GetStringFromSchema(SpawnPlayerDataSource, SpatialConstants::SPAWN_PLAYER_PLATFORM_NAME_ID));
		Schema_AddBool(SpawnPlayerDataDestination, SpatialConstants::SPAWN_PLAYER_IS_SIMULATED_ID, GetBoolFromSchema(SpawnPlayerDataSource, SpatialConstants::SPAWN_PLAYER_IS_SIMULATED_ID));
		AddStringToSchema(SpawnPlayerDataDestination, SpatialConstants::SPAWN_PLAYER_CLIENT_WORKER_ID, GetStringFromSchema(SpawnPlayerDataSource, SpatialConstants::SPAWN_PLAYER_CLIENT_WORKER_ID));
	}
};

} // namespace SpatialGDK
