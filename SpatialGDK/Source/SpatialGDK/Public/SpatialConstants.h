// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "Improbable/SpatialEngineConstants.h"
#include "Schema/UnrealObjectRef.h"
#include "SpatialCommonTypes.h"
#include "UObject/Script.h"

#include <WorkerSDK/improbable/c_schema.h>
#include <WorkerSDK/improbable/c_worker.h>

enum ESchemaComponentType : int32
{
	SCHEMA_Invalid = -1,

	// Properties
	SCHEMA_Data, // Represents properties being replicated to all workers
	SCHEMA_OwnerOnly,
	SCHEMA_Handover,

	SCHEMA_Count,

	// RPCs
	SCHEMA_ClientReliableRPC,
	SCHEMA_ClientUnreliableRPC,
	SCHEMA_ServerReliableRPC,
	SCHEMA_ServerUnreliableRPC,
	SCHEMA_NetMulticastRPC,
	SCHEMA_CrossServerRPC,


	// Iteration helpers
	SCHEMA_Begin = SCHEMA_Data,
};

FORCEINLINE ESchemaComponentType FunctionFlagsToRPCSchemaType(EFunctionFlags FunctionFlags)
{
	if (FunctionFlags & FUNC_NetClient)
	{
		return SCHEMA_ClientReliableRPC;
	}
	else if (FunctionFlags & FUNC_NetServer)
	{
		return SCHEMA_ServerReliableRPC;
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
	case SCHEMA_ClientReliableRPC:
		return TEXT("Client, Reliable");
	case SCHEMA_ClientUnreliableRPC:
		return TEXT("Client, Unreliable");
	case SCHEMA_ServerReliableRPC:
		return TEXT("Server, Reliable");
	case SCHEMA_ServerUnreliableRPC:
		return TEXT("Server, Unreliable");
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
		INITIAL_VIRTUAL_WORKER_TRANSLATOR_ENTITY_ID = 3,
		FIRST_AVAILABLE_ENTITY_ID = 4,
	};

	const Worker_ComponentId INVALID_COMPONENT_ID							= 0;

	const Worker_ComponentId ENTITY_ACL_COMPONENT_ID						= 50;
	const Worker_ComponentId METADATA_COMPONENT_ID							= 53;
	const Worker_ComponentId POSITION_COMPONENT_ID							= 54;
	const Worker_ComponentId PERSISTENCE_COMPONENT_ID						= 55;
	const Worker_ComponentId INTEREST_COMPONENT_ID							= 58;
	// This is a component on per-worker system entities.
	const Worker_ComponentId WORKER_COMPONENT_ID							= 60;

	const Worker_ComponentId MAX_RESERVED_SPATIAL_SYSTEM_COMPONENT_ID		= 100;

	const Worker_ComponentId SPAWN_DATA_COMPONENT_ID						= 9999;
	const Worker_ComponentId PLAYER_SPAWNER_COMPONENT_ID					= 9998;
	const Worker_ComponentId SINGLETON_COMPONENT_ID							= 9997;
	const Worker_ComponentId UNREAL_METADATA_COMPONENT_ID					= 9996;
	const Worker_ComponentId SINGLETON_MANAGER_COMPONENT_ID					= 9995;
	const Worker_ComponentId DEPLOYMENT_MAP_COMPONENT_ID					= 9994;
	const Worker_ComponentId STARTUP_ACTOR_MANAGER_COMPONENT_ID			    = 9993;
	const Worker_ComponentId GSM_SHUTDOWN_COMPONENT_ID						= 9992;
	const Worker_ComponentId HEARTBEAT_COMPONENT_ID							= 9991;
	const Worker_ComponentId CLIENT_RPC_ENDPOINT_COMPONENT_ID				= 9990;
	const Worker_ComponentId SERVER_RPC_ENDPOINT_COMPONENT_ID				= 9989;
	const Worker_ComponentId NETMULTICAST_RPCS_COMPONENT_ID					= 9987;
	const Worker_ComponentId NOT_STREAMED_COMPONENT_ID						= 9986;
	const Worker_ComponentId RPCS_ON_ENTITY_CREATION_ID						= 9985;
	const Worker_ComponentId DEBUG_METRICS_COMPONENT_ID						= 9984;
	const Worker_ComponentId ALWAYS_RELEVANT_COMPONENT_ID					= 9983;
	const Worker_ComponentId TOMBSTONE_COMPONENT_ID                         = 9982;
	const Worker_ComponentId DORMANT_COMPONENT_ID							= 9981;
	const Worker_ComponentId AUTHORITY_INTENT_COMPONENT_ID                  = 9980;
	const Worker_ComponentId VIRTUAL_WORKER_TRANSLATION_COMPONENT_ID        = 9979;

	const Worker_ComponentId STARTING_GENERATED_COMPONENT_ID				= 10000;

	const Schema_FieldId SINGLETON_MANAGER_SINGLETON_NAME_TO_ENTITY_ID		= 1;

	const Schema_FieldId DEPLOYMENT_MAP_MAP_URL_ID							= 1;
	const Schema_FieldId DEPLOYMENT_MAP_ACCEPTING_PLAYERS_ID				= 2;

	const Schema_FieldId STARTUP_ACTOR_MANAGER_CAN_BEGIN_PLAY_ID			= 1;

	const Schema_FieldId ACTOR_COMPONENT_REPLICATES_ID                      = 1;
	const Schema_FieldId ACTOR_TEAROFF_ID									= 3;

	const Schema_FieldId HEARTBEAT_EVENT_ID                                 = 1;
	const Schema_FieldId HEARTBEAT_CLIENT_HAS_QUIT_ID						= 1;

	const Schema_FieldId SHUTDOWN_MULTI_PROCESS_REQUEST_ID					= 1;
	const Schema_FieldId SHUTDOWN_ADDITIONAL_SERVERS_EVENT_ID				= 1;

	const Schema_FieldId CLEAR_RPCS_ON_ENTITY_CREATION						= 1;

	// DebugMetrics command IDs
	const Schema_FieldId DEBUG_METRICS_START_RPC_METRICS_ID					= 1;
	const Schema_FieldId DEBUG_METRICS_STOP_RPC_METRICS_ID					= 2;
	const Schema_FieldId DEBUG_METRICS_MODIFY_SETTINGS_ID					= 3;

	// ModifySettingPayload Field IDs
	const Schema_FieldId MODIFY_SETTING_PAYLOAD_NAME_ID						= 1;
	const Schema_FieldId MODIFY_SETTING_PAYLOAD_VALUE_ID					= 2;

	// UnrealObjectRef Field IDs
	const Schema_FieldId UNREAL_OBJECT_REF_ENTITY_ID						= 1;
	const Schema_FieldId UNREAL_OBJECT_REF_OFFSET_ID						= 2;
	const Schema_FieldId UNREAL_OBJECT_REF_PATH_ID							= 3;
	const Schema_FieldId UNREAL_OBJECT_REF_NO_LOAD_ON_CLIENT_ID				= 4;
	const Schema_FieldId UNREAL_OBJECT_REF_OUTER_ID							= 5;
	const Schema_FieldId UNREAL_OBJECT_REF_USE_SINGLETON_CLASS_PATH_ID		= 6;

	// UnrealRPCPayload Field IDs
	const Schema_FieldId UNREAL_RPC_PAYLOAD_OFFSET_ID						= 1;
	const Schema_FieldId UNREAL_RPC_PAYLOAD_RPC_INDEX_ID					= 2;
	const Schema_FieldId UNREAL_RPC_PAYLOAD_RPC_PAYLOAD_ID					= 3;
	// UnrealPackedRPCPayload additional Field ID
	const Schema_FieldId UNREAL_PACKED_RPC_PAYLOAD_ENTITY_ID				= 4;

	// Unreal(Client|Server|Multicast)RPCEndpoint Field IDs
	const Schema_FieldId UNREAL_RPC_ENDPOINT_READY_ID 						= 1;
	const Schema_FieldId UNREAL_RPC_ENDPOINT_EVENT_ID						= 1;
	const Schema_FieldId UNREAL_RPC_ENDPOINT_PACKED_EVENT_ID				= 2;
	const Schema_FieldId UNREAL_RPC_ENDPOINT_COMMAND_ID						= 1;

	const Schema_FieldId PLAYER_SPAWNER_SPAWN_PLAYER_COMMAND_ID = 1;

	// AuthorityIntent codes and Field IDs.
	const Schema_FieldId AUTHORITY_INTENT_VIRTUAL_WORKER_ID					= 1;
	const VirtualWorkerId INVALID_VIRTUAL_WORKER_ID							= 0;

	// VirtualWorkerTranslation Field IDs.
	const Schema_FieldId VIRTUAL_WORKER_TRANSLATION_MAPPING_ID				= 1;
	const Schema_FieldId MAPPING_VIRTUAL_WORKER_ID							= 1;
	const Schema_FieldId MAPPING_PHYSICAL_WORKER_NAME						= 2;

	// WorkerEntity Field IDs.
	const Schema_FieldId WORKER_ID_ID										= 1;
	const Schema_FieldId WORKER_TYPE_ID										= 2;

	// Reserved entity IDs expire in 5 minutes, we will refresh them every 3 minutes to be safe.
	const float ENTITY_RANGE_EXPIRATION_INTERVAL_SECONDS = 180.0f;

	const float FIRST_COMMAND_RETRY_WAIT_SECONDS = 0.2f;
	const uint32 MAX_NUMBER_COMMAND_ATTEMPTS = 5u;

	const FName DefaultActorGroup = FName(TEXT("Default"));

	const WorkerAttributeSet UnrealServerAttributeSet = TArray<FString>{DefaultServerWorkerType.ToString()};
	const WorkerAttributeSet UnrealClientAttributeSet = TArray<FString>{DefaultClientWorkerType.ToString()};

	const WorkerRequirementSet UnrealServerPermission{ {UnrealServerAttributeSet} };
	const WorkerRequirementSet UnrealClientPermission{ {UnrealClientAttributeSet} };
	const WorkerRequirementSet ClientOrServerPermission{ {UnrealClientAttributeSet, UnrealServerAttributeSet} };

	const FString ClientsStayConnectedURLOption = TEXT("clientsStayConnected");
	const FString SnapshotURLOption = TEXT("snapshot=");

	const FString AssemblyPattern = TEXT("^[a-zA-Z0-9_.-]{5,64}$");
	const FString ProjectPattern = TEXT("^[a-z0-9_]{3,32}$");
	const FString DeploymentPattern = TEXT("^[a-z0-9_]{2,32}$");

	inline float GetCommandRetryWaitTimeSeconds(uint32 NumAttempts)
	{
		// Double the time to wait on each failure.
		uint32 WaitTimeExponentialFactor = 1u << (NumAttempts - 1);
		return FIRST_COMMAND_RETRY_WAIT_SECONDS * WaitTimeExponentialFactor;
	}

	const FString LOCAL_HOST  = TEXT("127.0.0.1");
	const uint16 DEFAULT_PORT = 7777;

	const float ENTITY_QUERY_RETRY_WAIT_SECONDS = 3.0f;

	const Worker_ComponentId MIN_EXTERNAL_SCHEMA_ID = 1000;
	const Worker_ComponentId MAX_EXTERNAL_SCHEMA_ID = 2000;

	const FString SPATIALOS_METRICS_DYNAMIC_FPS = TEXT("Dynamic.FPS");

	const FString LOCATOR_HOST = TEXT("locator.improbable.io");
	const uint16 LOCATOR_PORT  = 444;

	const FString DEVELOPMENT_AUTH_PLAYER_ID = TEXT("Player Id");

	const FString SCHEMA_DATABASE_FILE_PATH  = TEXT("Spatial/SchemaDatabase");
	const FString SCHEMA_DATABASE_ASSET_PATH = TEXT("/Game/Spatial/SchemaDatabase");

	const FString ZoningAttribute = DefaultServerWorkerType.ToString();

} // ::SpatialConstants

FORCEINLINE Worker_ComponentId SchemaComponentTypeToWorkerComponentId(ESchemaComponentType SchemaType)
{
	switch (SchemaType)
	{
	case SCHEMA_CrossServerRPC:
	{
		return SpatialConstants::SERVER_RPC_ENDPOINT_COMPONENT_ID;
	}
	case SCHEMA_NetMulticastRPC:
	{
		return SpatialConstants::NETMULTICAST_RPCS_COMPONENT_ID;
	}
	case SCHEMA_ClientReliableRPC:
	case SCHEMA_ClientUnreliableRPC:
	{
		return SpatialConstants::SERVER_RPC_ENDPOINT_COMPONENT_ID;
	}
	case SCHEMA_ServerReliableRPC:
	case SCHEMA_ServerUnreliableRPC:
	{
		return SpatialConstants::CLIENT_RPC_ENDPOINT_COMPONENT_ID;
	}
	default:
		checkNoEntry();
		return SpatialConstants::INVALID_COMPONENT_ID;
	}
}
