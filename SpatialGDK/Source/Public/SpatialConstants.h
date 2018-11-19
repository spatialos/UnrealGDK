// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "UObject/improbable/UnrealObjectRef.h"

#include <WorkerSDK/improbable/c_schema.h>
#include <WorkerSDK/improbable/c_worker.h>

// IMPORTANT: This is required for Linux builds to succeed - don't remove!
// Worker_EntityId from the Worker SDK resolves to a long on Linux.
// These are not a type of key supported by TMap.
using Worker_EntityId_Key = int64;

enum ESchemaComponentType : int32
{
	SCHEMA_Invalid = -1,

	// Properties
	SCHEMA_Data, // Represents properties being replicated to all workers
	SCHEMA_OwnerOnly,
	SCHEMA_Handover,

	// RPCs
	SCHEMA_ClientRPC,
	SCHEMA_ServerRPC,
	SCHEMA_NetMulticastRPC,
	SCHEMA_CrossServerRPC,

	SCHEMA_Count,

	// Iteration helpers
	SCHEMA_Begin = SCHEMA_Data,
	SCHEMA_FirstRPC = SCHEMA_ClientRPC,
	SCHEMA_LastRPC = SCHEMA_CrossServerRPC,
};

namespace SpatialConstants
{
	enum EntityIds
	{
		INVALID_ENTITY_ID = 0,
		INITIAL_SPAWNER_ENTITY_ID = 1,
		INITIAL_GLOBAL_STATE_MANAGER_ENTITY_ID = 2,
		PLACEHOLDER_ENTITY_ID_FIRST = 3,
		PLACEHOLDER_ENTITY_ID_LAST = PLACEHOLDER_ENTITY_ID_FIRST + 35, // 36 placeholder entities.
	};

	const Worker_ComponentId INVALID_COMPONENT_ID				= 0;

	const Worker_ComponentId ENTITY_ACL_COMPONENT_ID			= 50;
	const Worker_ComponentId METADATA_COMPONENT_ID				= 53;
	const Worker_ComponentId POSITION_COMPONENT_ID				= 54;
	const Worker_ComponentId PERSISTENCE_COMPONENT_ID			= 55;

	const Worker_ComponentId ROTATION_COMPONENT_ID							= 100001;
	const Worker_ComponentId PLAYER_SPAWNER_COMPONENT_ID					= 100002;
	const Worker_ComponentId SINGLETON_COMPONENT_ID							= 100003;
	const Worker_ComponentId UNREAL_METADATA_COMPONENT_ID					= 100004;
	const Worker_ComponentId GLOBAL_STATE_MANAGER_COMPONENT_ID				= 100005;
	const Worker_ComponentId GLOBAL_STATE_MANAGER_DEPLOYMENT_COMPONENT_ID	= 100006;
	const Worker_ComponentId SERVER_ONLY_SINGLETON_COMPONENT_ID				= 100007;
	const Worker_ComponentId STARTING_GENERATED_COMPONENT_ID				= 100010;

	const Schema_FieldId GLOBAL_STATE_MANAGER_MAP_URL_ID			= 1;
	const Schema_FieldId GLOBAL_STATE_MANAGER_ACCEPTING_PLAYERS_ID	= 2;

	const float FIRST_COMMAND_RETRY_WAIT_SECONDS = 0.2f;
	const float REPLICATED_STABLY_NAMED_ACTORS_DELETION_TIMEOUT_SECONDS = 5.0f;
	const uint32 MAX_NUMBER_COMMAND_ATTEMPTS = 5u;

	const FUnrealObjectRef NULL_OBJECT_REF(0, 0);
	const FUnrealObjectRef UNRESOLVED_OBJECT_REF(0, 1);

	static const FString ClientWorkerType = TEXT("UnrealClient");
	static const FString ServerWorkerType = TEXT("UnrealServer");

	static const FString ClientsStayConnectedURLOption = TEXT("clientsStayConnected");
	static const FString SnapshotURLOption = TEXT("snapshot=");

	inline float GetCommandRetryWaitTimeSeconds(uint32 NumAttempts)
	{
		// Double the time to wait on each failure.
		uint32 WaitTimeExponentialFactor = 1u << (NumAttempts - 1);
		return FIRST_COMMAND_RETRY_WAIT_SECONDS * WaitTimeExponentialFactor;
	}

	const FString LOCAL_HOST = TEXT("127.0.0.1");
	const uint16 DEFAULT_PORT = 7777;

	const float ENTITY_QUERY_RETRY_WAIT_SECONDS = 3.0f;
}
