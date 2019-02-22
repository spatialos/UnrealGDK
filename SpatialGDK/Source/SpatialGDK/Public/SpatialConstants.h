// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "UObject/Script.h"

#include "Schema/UnrealObjectRef.h"

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

FORCEINLINE ESchemaComponentType FunctionFlagsToRPCSchemaType(EFunctionFlags FunctionFlags)
{
	if (FunctionFlags & FUNC_NetClient)
	{
		return SCHEMA_ClientRPC;
	}
	else if (FunctionFlags & FUNC_NetServer)
	{
		return SCHEMA_ServerRPC;
	}
	else if (FunctionFlags & FUNC_NetMulticast)
	{
		return SCHEMA_NetMulticastRPC;
	}
	else if (FunctionFlags & FUNC_NetCrossServer)
	{
		return SCHEMA_CrossServerRPC;
	}
	else
	{
		return SCHEMA_Invalid;
	}
}

FORCEINLINE FString RPCSchemaTypeToString(ESchemaComponentType RPCType)
{
	switch (RPCType)
	{
	case SCHEMA_ClientRPC:
		return TEXT("Client");
	case SCHEMA_ServerRPC:
		return TEXT("Server");
	case SCHEMA_NetMulticastRPC:
		return TEXT("Multicast");
	case SCHEMA_CrossServerRPC:
		return TEXT("CrossServer");
	}

	checkNoEntry();
	return FString();
}

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

	const Worker_ComponentId INVALID_COMPONENT_ID							= 0;

	const Worker_ComponentId ENTITY_ACL_COMPONENT_ID						= 50;
	const Worker_ComponentId METADATA_COMPONENT_ID							= 53;
	const Worker_ComponentId POSITION_COMPONENT_ID							= 54;
	const Worker_ComponentId PERSISTENCE_COMPONENT_ID						= 55;
	const Worker_ComponentId INTEREST_COMPONENT_ID							= 58;

	const Worker_ComponentId SPAWN_DATA_COMPONENT_ID						= 9999;
	const Worker_ComponentId PLAYER_SPAWNER_COMPONENT_ID					= 9998;
	const Worker_ComponentId SINGLETON_COMPONENT_ID							= 9997;
	const Worker_ComponentId UNREAL_METADATA_COMPONENT_ID					= 9996;
	const Worker_ComponentId SINGLETON_MANAGER_COMPONENT_ID					= 9995;
	const Worker_ComponentId DEPLOYMENT_MAP_COMPONENT_ID					= 9994;
	const Worker_ComponentId STARTUP_ACTOR_MANAGER_COMPONENT_ID			    = 9993;
	const Worker_ComponentId GSM_SHUTDOWN_COMPONENT_ID						= 9992;
	const Worker_ComponentId HEARTBEAT_COMPONENT_ID							= 9991;
	const Worker_ComponentId STARTING_GENERATED_COMPONENT_ID				= 10000;

	const Schema_FieldId SINGLETON_MANAGER_SINGLETON_NAME_TO_ENTITY_ID		= 1;

	const Schema_FieldId DEPLOYMENT_MAP_MAP_URL_ID							= 1;
	const Schema_FieldId DEPLOYMENT_MAP_ACCEPTING_PLAYERS_ID				= 2;

	const Schema_FieldId STARTUP_ACTOR_MANAGER_CAN_BEGIN_PLAY_ID			= 1;

	const Schema_FieldId ACTOR_COMPONENT_REPLICATES_ID                      = 1;

	const Schema_FieldId HEARTBEAT_EVENT_ID                                 = 1;

	// TODO: Make these easily configurable: UNR-984
	const float HEARTBEAT_INTERVAL_SECONDS = 2.0f;
	const float HEARTBEAT_TIMEOUT_SECONDS = 10.0f;

	const float FIRST_COMMAND_RETRY_WAIT_SECONDS = 0.2f;
	const float REPLICATED_STABLY_NAMED_ACTORS_DELETION_TIMEOUT_SECONDS = 5.0f;
	const uint32 MAX_NUMBER_COMMAND_ATTEMPTS = 5u;

	static const FString ClientWorkerType = TEXT("UnrealClient");
	static const FString ServerWorkerType = TEXT("UnrealWorker");

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
