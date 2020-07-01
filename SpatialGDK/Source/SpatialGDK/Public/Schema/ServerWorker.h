// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "Schema/Component.h"
#include "Schema/PlayerSpawner.h"
#include "SpatialCommonTypes.h"
#include "SpatialConstants.h"
#include "Utils/SchemaUtils.h"

#include "Containers/UnrealString.h"

#include <WorkerSDK/improbable/c_schema.h>
#include <WorkerSDK/improbable/c_worker.h>

namespace SpatialGDK
{

// The ServerWorker component exists to hold the physical worker name corresponding to a
// server worker entity. This is so that the translator can make virtual workers to physical
// worker names using the server worker entities.
struct ServerWorker : Component
{
	static const Worker_ComponentId ComponentId = SpatialConstants::SERVER_WORKER_COMPONENT_ID;

	ServerWorker()
		: WorkerName(SpatialConstants::INVALID_WORKER_NAME)
		, bReadyToBeginPlay(false)
		, SystemEntityId(SpatialConstants::INVALID_ENTITY_ID)
	{}

	ServerWorker(const PhysicalWorkerName& InWorkerName, const bool bInReadyToBeginPlay, const Worker_EntityId InSystemEntityId)
	{
		WorkerName = InWorkerName;
		bReadyToBeginPlay = bInReadyToBeginPlay;
		SystemEntityId = InSystemEntityId;
	}

	ServerWorker(const Worker_ComponentData& Data)
	{
		Schema_Object* ComponentObject = Schema_GetComponentDataFields(Data.schema_type);

		WorkerName = GetStringFromSchema(ComponentObject, SpatialConstants::SERVER_WORKER_NAME_ID);
		bReadyToBeginPlay = GetBoolFromSchema(ComponentObject, SpatialConstants::SERVER_WORKER_READY_TO_BEGIN_PLAY_ID);
		SystemEntityId = Schema_GetEntityId(ComponentObject, SpatialConstants::SERVER_WORKER_SYSTEM_ENTITY_ID);
	}

	Worker_ComponentData CreateServerWorkerData()
	{
		Worker_ComponentData Data = {};
		Data.component_id = ComponentId;
		Data.schema_type = Schema_CreateComponentData();
		Schema_Object* ComponentObject = Schema_GetComponentDataFields(Data.schema_type);

		AddStringToSchema(ComponentObject, SpatialConstants::SERVER_WORKER_NAME_ID, WorkerName);
		Schema_AddBool(ComponentObject, SpatialConstants::SERVER_WORKER_READY_TO_BEGIN_PLAY_ID, bReadyToBeginPlay);
		Schema_AddEntityId(ComponentObject, SpatialConstants::SERVER_WORKER_SYSTEM_ENTITY_ID, SystemEntityId);

		return Data;
	}

	Worker_ComponentUpdate CreateServerWorkerUpdate()
	{
		Worker_ComponentUpdate Update = {};
		Update.component_id = ComponentId;
		Update.schema_type = Schema_CreateComponentUpdate();
		Schema_Object* ComponentObject = Schema_GetComponentUpdateFields(Update.schema_type);

		AddStringToSchema(ComponentObject, SpatialConstants::SERVER_WORKER_NAME_ID, WorkerName);
		Schema_AddBool(ComponentObject, SpatialConstants::SERVER_WORKER_READY_TO_BEGIN_PLAY_ID, bReadyToBeginPlay);
		Schema_AddEntityId(ComponentObject, SpatialConstants::SERVER_WORKER_SYSTEM_ENTITY_ID, SystemEntityId);

		return Update;
	}

	void ApplyComponentUpdate(const Worker_ComponentUpdate& Update)
	{
		Schema_Object* ComponentObject = Schema_GetComponentUpdateFields(Update.schema_type);

		WorkerName = GetStringFromSchema(ComponentObject, SpatialConstants::SERVER_WORKER_NAME_ID);
		bReadyToBeginPlay = GetBoolFromSchema(ComponentObject, SpatialConstants::SERVER_WORKER_READY_TO_BEGIN_PLAY_ID);
		SystemEntityId = Schema_GetEntityId(ComponentObject, SpatialConstants::SERVER_WORKER_SYSTEM_ENTITY_ID);
	}

	static Worker_CommandRequest CreateForwardPlayerSpawnRequest(Schema_CommandRequest* SchemaCommandRequest)
	{
		Worker_CommandRequest CommandRequest = {};
		CommandRequest.component_id = SpatialConstants::SERVER_WORKER_COMPONENT_ID;
		CommandRequest.command_index = SpatialConstants::SERVER_WORKER_FORWARD_SPAWN_REQUEST_COMMAND_ID;
		CommandRequest.schema_type = SchemaCommandRequest;
		return CommandRequest;
	}

	static Worker_CommandResponse CreateForwardPlayerSpawnResponse(const bool bSuccess)
	{
		Worker_CommandResponse CommandResponse = {};
		CommandResponse.component_id = SpatialConstants::SERVER_WORKER_COMPONENT_ID;
		CommandResponse.command_index = SpatialConstants::SERVER_WORKER_FORWARD_SPAWN_REQUEST_COMMAND_ID;
		CommandResponse.schema_type = Schema_CreateCommandResponse();
		Schema_Object* ResponseObject = Schema_GetCommandResponseObject(CommandResponse.schema_type);

		Schema_AddBool(ResponseObject, SpatialConstants::FORWARD_SPAWN_PLAYER_RESPONSE_SUCCESS_ID, bSuccess);

		return CommandResponse;
	}

	static void CreateForwardPlayerSpawnSchemaRequest(Schema_CommandRequest* Request, const FUnrealObjectRef& PlayerStartObjectRef, const Schema_Object* OriginalPlayerSpawnRequest, const PhysicalWorkerName& ClientWorkerID)
	{
		Schema_Object* RequestFields = Schema_GetCommandRequestObject(Request);

		AddObjectRefToSchema(RequestFields, SpatialConstants::FORWARD_SPAWN_PLAYER_START_ACTOR_ID, PlayerStartObjectRef);

		Schema_Object* PlayerSpawnData = Schema_AddObject(RequestFields, SpatialConstants::FORWARD_SPAWN_PLAYER_DATA_ID);
		PlayerSpawner::CopySpawnDataBetweenObjects(OriginalPlayerSpawnRequest, PlayerSpawnData);

		AddStringToSchema(RequestFields, SpatialConstants::FORWARD_SPAWN_PLAYER_CLIENT_WORKER_ID, ClientWorkerID);
	}

	PhysicalWorkerName WorkerName;
	bool bReadyToBeginPlay;
	Worker_EntityId SystemEntityId;
};

} // namespace SpatialGDK

