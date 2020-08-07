// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "Schema/Component.h"
#include "SpatialConstants.h"
#include "Utils/SchemaUtils.h"

#include "Containers/UnrealString.h"
#include "Engine/EngineBaseTypes.h"
#include "Engine/GameInstance.h"
#include "GameFramework/OnlineReplStructs.h"
#include "Kismet/GameplayStatics.h"
#include "UObject/CoreNet.h"

#include <WorkerSDK/improbable/c_schema.h>
#include <WorkerSDK/improbable/c_worker.h>

namespace SpatialGDK
{
struct SpawnPlayerRequest
{
	FURL LoginURL;
	FUniqueNetIdRepl UniqueId;
	FName OnlinePlatformName;
	bool bIsSimulatedPlayer;
};

struct PlayerSpawner : Component
{
	static const Worker_ComponentId ComponentId = SpatialConstants::PLAYER_SPAWNER_COMPONENT_ID;

	PlayerSpawner() = default;

	static Worker_CommandRequest CreatePlayerSpawnRequest(SpawnPlayerRequest& SpawnRequest)
	{
		Worker_CommandRequest CommandRequest = {};
		CommandRequest.component_id = SpatialConstants::PLAYER_SPAWNER_COMPONENT_ID;
		CommandRequest.command_index = SpatialConstants::PLAYER_SPAWNER_SPAWN_PLAYER_COMMAND_ID;
		CommandRequest.schema_type = Schema_CreateCommandRequest();

		Schema_Object* RequestFields = Schema_GetCommandRequestObject(CommandRequest.schema_type);
		AddSpawnPlayerData(RequestFields, SpawnRequest);

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

	static void AddSpawnPlayerData(Schema_Object* RequestObject, SpawnPlayerRequest& SpawnRequest)
	{
		AddStringToSchema(RequestObject, SpatialConstants::SPAWN_PLAYER_URL_ID, SpawnRequest.LoginURL.ToString(true));

		// Write player identity information.
		FNetBitWriter UniqueIdWriter(0);
		UniqueIdWriter << SpawnRequest.UniqueId;
		AddBytesToSchema(RequestObject, SpatialConstants::SPAWN_PLAYER_UNIQUE_ID, UniqueIdWriter);
		AddStringToSchema(RequestObject, SpatialConstants::SPAWN_PLAYER_PLATFORM_NAME_ID, SpawnRequest.OnlinePlatformName.ToString());
		Schema_AddBool(RequestObject, SpatialConstants::SPAWN_PLAYER_IS_SIMULATED_ID, SpawnRequest.bIsSimulatedPlayer);
	}

	static FURL ExtractUrlFromPlayerSpawnParams(const Schema_Object* Payload)
	{
		return FURL(nullptr, *GetStringFromSchema(Payload, SpatialConstants::SPAWN_PLAYER_URL_ID), TRAVEL_Absolute);
	}

	static SpawnPlayerRequest ExtractPlayerSpawnParams(const Schema_Object* CommandRequestPayload)
	{
		const FURL LoginURL = ExtractUrlFromPlayerSpawnParams(CommandRequestPayload);

		FUniqueNetIdRepl UniqueId;
		TArray<uint8> UniqueIdBytes = GetBytesFromSchema(CommandRequestPayload, SpatialConstants::SPAWN_PLAYER_UNIQUE_ID);
		FNetBitReader UniqueIdReader(nullptr, UniqueIdBytes.GetData(), UniqueIdBytes.Num() * 8);
		UniqueIdReader << UniqueId;

		const FName OnlinePlatformName =
			FName(*GetStringFromSchema(CommandRequestPayload, SpatialConstants::SPAWN_PLAYER_PLATFORM_NAME_ID));

		const bool bIsSimulated = GetBoolFromSchema(CommandRequestPayload, SpatialConstants::SPAWN_PLAYER_IS_SIMULATED_ID);

		return { LoginURL, UniqueId, OnlinePlatformName, bIsSimulated };
	}

	static void CopySpawnDataBetweenObjects(const Schema_Object* SpawnPlayerDataSource, Schema_Object* SpawnPlayerDataDestination)
	{
		AddStringToSchema(SpawnPlayerDataDestination, SpatialConstants::SPAWN_PLAYER_URL_ID,
						  GetStringFromSchema(SpawnPlayerDataSource, SpatialConstants::SPAWN_PLAYER_URL_ID));
		TArray<uint8> UniqueId = GetBytesFromSchema(SpawnPlayerDataSource, SpatialConstants::SPAWN_PLAYER_UNIQUE_ID);
		AddBytesToSchema(SpawnPlayerDataDestination, SpatialConstants::SPAWN_PLAYER_UNIQUE_ID, UniqueId.GetData(), UniqueId.Num());
		AddStringToSchema(SpawnPlayerDataDestination, SpatialConstants::SPAWN_PLAYER_PLATFORM_NAME_ID,
						  GetStringFromSchema(SpawnPlayerDataSource, SpatialConstants::SPAWN_PLAYER_PLATFORM_NAME_ID));
		Schema_AddBool(SpawnPlayerDataDestination, SpatialConstants::SPAWN_PLAYER_IS_SIMULATED_ID,
					   GetBoolFromSchema(SpawnPlayerDataSource, SpatialConstants::SPAWN_PLAYER_IS_SIMULATED_ID));
	}
};

} // namespace SpatialGDK
