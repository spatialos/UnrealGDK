// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "Improbable/SpatialEngineConstants.h"
#include "Schema/UnrealObjectRef.h"
#include "SpatialCommonTypes.h"
#include "UObject/Script.h"

#include <WorkerSDK/improbable/c_schema.h>
#include <WorkerSDK/improbable/c_worker.h>

#include "SpatialConstants.generated.h"

UENUM()
enum class ERPCType : uint8
{
	Invalid,
	ClientReliable,
	ClientUnreliable,
	ServerReliable,
	ServerUnreliable,
	NetMulticast,
	CrossServer
};

enum ESchemaComponentType : int32
{
	SCHEMA_Invalid = -1,

	// Properties
	SCHEMA_Data, // Represents properties being replicated to all workers
	SCHEMA_OwnerOnly,
	SCHEMA_Handover,

	SCHEMA_Count,

	// Iteration helpers
	SCHEMA_Begin = SCHEMA_Data,
};

namespace SpatialConstants
{

inline FString RPCTypeToString(ERPCType RPCType)
{
	switch (RPCType)
	{
	case ERPCType::ClientReliable:
		return TEXT("Client, Reliable");
	case ERPCType::ClientUnreliable:
		return TEXT("Client, Unreliable");
	case ERPCType::ServerReliable:
		return TEXT("Server, Reliable");
	case ERPCType::ServerUnreliable:
		return TEXT("Server, Unreliable");
	case ERPCType::NetMulticast:
		return TEXT("Multicast");
	case ERPCType::CrossServer:
		return TEXT("CrossServer");
	}

	checkNoEntry();
	return FString();
}

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
const Worker_ComponentId PLAYERIDENTITY_COMPONENT_ID			= 61;

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
// Marking the event-based RPC components as legacy while the ring buffer
// implementation is under a feature flag.
const Worker_ComponentId CLIENT_RPC_ENDPOINT_COMPONENT_ID_LEGACY		= 9990;
const Worker_ComponentId SERVER_RPC_ENDPOINT_COMPONENT_ID_LEGACY		= 9989;
const Worker_ComponentId NETMULTICAST_RPCS_COMPONENT_ID_LEGACY			= 9987;

const Worker_ComponentId NOT_STREAMED_COMPONENT_ID						= 9986;
const Worker_ComponentId RPCS_ON_ENTITY_CREATION_ID						= 9985;
const Worker_ComponentId DEBUG_METRICS_COMPONENT_ID						= 9984;
const Worker_ComponentId ALWAYS_RELEVANT_COMPONENT_ID					= 9983;
const Worker_ComponentId TOMBSTONE_COMPONENT_ID                         = 9982;
const Worker_ComponentId DORMANT_COMPONENT_ID							= 9981;
const Worker_ComponentId AUTHORITY_INTENT_COMPONENT_ID                  = 9980;
const Worker_ComponentId VIRTUAL_WORKER_TRANSLATION_COMPONENT_ID        = 9979;

const Worker_ComponentId CLIENT_ENDPOINT_COMPONENT_ID					= 9978;
const Worker_ComponentId SERVER_ENDPOINT_COMPONENT_ID					= 9977;
const Worker_ComponentId MULTICAST_RPCS_COMPONENT_ID					= 9976;
const Worker_ComponentId SPATIAL_DEBUGGING_COMPONENT_ID					= 9975;
const Worker_ComponentId SERVER_WORKER_COMPONENT_ID						= 9974;
const Worker_ComponentId SERVER_TO_SERVER_COMMAND_ENDPOINT_COMPONENT_ID = 9973;

const Worker_ComponentId STARTING_GENERATED_COMPONENT_ID				= 10000;

const Schema_FieldId SINGLETON_MANAGER_SINGLETON_NAME_TO_ENTITY_ID		= 1;

const Schema_FieldId DEPLOYMENT_MAP_MAP_URL_ID							= 1;
const Schema_FieldId DEPLOYMENT_MAP_ACCEPTING_PLAYERS_ID				= 2;
const Schema_FieldId DEPLOYMENT_MAP_SESSION_ID							= 3;
const Schema_FieldId DEPLOYMENT_MAP_SCHEMA_HASH							= 4;

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
const Schema_FieldId UNREAL_RPC_PAYLOAD_TRACE_ID						= 4;
// UnrealPackedRPCPayload additional Field ID
const Schema_FieldId UNREAL_PACKED_RPC_PAYLOAD_ENTITY_ID				= 5;

const Schema_FieldId UNREAL_RPC_TRACE_ID								= 1;
const Schema_FieldId UNREAL_RPC_SPAN_ID									= 2;

// Unreal(Client|Server|Multicast)RPCEndpoint Field IDs
const Schema_FieldId UNREAL_RPC_ENDPOINT_READY_ID 						= 1;
const Schema_FieldId UNREAL_RPC_ENDPOINT_EVENT_ID						= 1;
const Schema_FieldId UNREAL_RPC_ENDPOINT_PACKED_EVENT_ID				= 2;
const Schema_FieldId UNREAL_RPC_ENDPOINT_COMMAND_ID						= 1;

const Schema_FieldId PLAYER_SPAWNER_SPAWN_PLAYER_COMMAND_ID = 1;

// AuthorityIntent codes and Field IDs.
const Schema_FieldId AUTHORITY_INTENT_VIRTUAL_WORKER_ID					= 1;

// VirtualWorkerTranslation Field IDs.
const Schema_FieldId VIRTUAL_WORKER_TRANSLATION_MAPPING_ID				= 1;
const Schema_FieldId MAPPING_VIRTUAL_WORKER_ID							= 1;
const Schema_FieldId MAPPING_PHYSICAL_WORKER_NAME						= 2;
const Schema_FieldId MAPPING_SERVER_WORKER_ENTITY_ID					= 3;
const PhysicalWorkerName TRANSLATOR_UNSET_PHYSICAL_NAME = FString("UnsetWorkerName");

// WorkerEntity Field IDs.
const Schema_FieldId WORKER_ID_ID										= 1;
const Schema_FieldId WORKER_TYPE_ID										= 2;

// SpatialDebugger Field IDs.
const Schema_FieldId SPATIAL_DEBUGGING_AUTHORITATIVE_VIRTUAL_WORKER_ID   = 1;
const Schema_FieldId SPATIAL_DEBUGGING_AUTHORITATIVE_COLOR               = 2;
const Schema_FieldId SPATIAL_DEBUGGING_INTENT_VIRTUAL_WORKER_ID          = 3;
const Schema_FieldId SPATIAL_DEBUGGING_INTENT_COLOR                      = 4;
const Schema_FieldId SPATIAL_DEBUGGING_IS_LOCKED                         = 5;

// ServerWorker Field IDs.
const Schema_FieldId SERVER_WORKER_NAME_ID								 = 1;
const Schema_FieldId SERVER_WORKER_READY_TO_BEGIN_PLAY_ID				 = 2;
const Schema_FieldId SERVER_WORKER_FORWARD_SPAWN_REQUEST_COMMAND_ID		 = 1;

// SpawnPlayerRequest type IDs.
const Schema_FieldId SPAWN_PLAYER_URL_ID								 = 1;
const Schema_FieldId SPAWN_PLAYER_UNIQUE_ID								 = 2;
const Schema_FieldId SPAWN_PLAYER_PLATFORM_NAME_ID						 = 3;
const Schema_FieldId SPAWN_PLAYER_IS_SIMULATED_ID						 = 4;
const Schema_FieldId SPAWN_PLAYER_CLIENT_WORKER_ID						 = 5;

// ForwardSpawnPlayerRequest type IDs.
const Schema_FieldId FORWARD_SPAWN_PLAYER_START_ACTOR_ID				 = 1;
const Schema_FieldId FORWARD_SPAWN_PLAYER_DATA_ID						 = 2;
const Schema_FieldId FORWARD_SPAWN_PLAYER_RESPONSE_SUCCESS_ID			 = 1;

// Reserved entity IDs expire in 5 minutes, we will refresh them every 3 minutes to be safe.
const float ENTITY_RANGE_EXPIRATION_INTERVAL_SECONDS = 180.0f;

const float FIRST_COMMAND_RETRY_WAIT_SECONDS = 0.2f;
const uint32 MAX_NUMBER_COMMAND_ATTEMPTS = 5u;
const float FORWARD_PLAYER_SPAWN_COMMAND_WAIT_SECONDS = 0.2f;

const FName DefaultActorGroup = FName(TEXT("Default"));

const VirtualWorkerId INVALID_VIRTUAL_WORKER_ID = 0;
const ActorLockToken INVALID_ACTOR_LOCK_TOKEN = 0;
const FString INVALID_WORKER_NAME = TEXT("");

const WorkerAttributeSet UnrealServerAttributeSet = TArray<FString>{DefaultServerWorkerType.ToString()};
const WorkerAttributeSet UnrealClientAttributeSet = TArray<FString>{DefaultClientWorkerType.ToString()};

const WorkerRequirementSet UnrealServerPermission{ {UnrealServerAttributeSet} };
const WorkerRequirementSet UnrealClientPermission{ {UnrealClientAttributeSet} };
const WorkerRequirementSet ClientOrServerPermission{ {UnrealClientAttributeSet, UnrealServerAttributeSet} };

const FString ClientsStayConnectedURLOption = TEXT("clientsStayConnected");
const FString SpatialSessionIdURLOption = TEXT("spatialSessionId=");

const FString LOCATOR_HOST    = TEXT("locator.improbable.io");
const FString LOCATOR_HOST_CN = TEXT("locator.spatialoschina.com");
const uint16 LOCATOR_PORT     = 443;

const FString AssemblyPattern   = TEXT("^[a-zA-Z0-9_.-]{5,64}$");
const FString ProjectPattern    = TEXT("^[a-z0-9_]{3,32}$");
const FString DeploymentPattern = TEXT("^[a-z0-9_]{2,32}$");

inline float GetCommandRetryWaitTimeSeconds(uint32 NumAttempts)
{
	// Double the time to wait on each failure.
	uint32 WaitTimeExponentialFactor = 1u << (NumAttempts - 1);
	return FIRST_COMMAND_RETRY_WAIT_SECONDS * WaitTimeExponentialFactor;
}

const FString LOCAL_HOST   = TEXT("127.0.0.1");
const uint16  DEFAULT_PORT = 7777;

const float ENTITY_QUERY_RETRY_WAIT_SECONDS = 3.0f;

const Worker_ComponentId MIN_EXTERNAL_SCHEMA_ID = 1000;
const Worker_ComponentId MAX_EXTERNAL_SCHEMA_ID = 2000;

const FString SPATIALOS_METRICS_DYNAMIC_FPS = TEXT("Dynamic.FPS");

// URL that can be used to reconnect using the command line arguments.
const FString RECONNECT_USING_COMMANDLINE_ARGUMENTS = TEXT("0.0.0.0");
const FString URL_LOGIN_OPTION = TEXT("login=");
const FString URL_PLAYER_IDENTITY_OPTION = TEXT("playeridentity=");
const FString URL_DEV_AUTH_TOKEN_OPTION = TEXT("devauthtoken=");
const FString URL_TARGET_DEPLOYMENT_OPTION = TEXT("deployment=");
const FString URL_PLAYER_ID_OPTION = TEXT("playerid=");
const FString URL_DISPLAY_NAME_OPTION = TEXT("displayname=");
const FString URL_METADATA_OPTION = TEXT("metadata=");

const FString DEVELOPMENT_AUTH_PLAYER_ID = TEXT("Player Id");

const FString SCHEMA_DATABASE_FILE_PATH  = TEXT("Spatial/SchemaDatabase");
const FString SCHEMA_DATABASE_ASSET_PATH = TEXT("/Game/Spatial/SchemaDatabase");

// A list of components clients require on top of any generated data components in order to handle non-authoritative actors correctly.
const TArray<Worker_ComponentId> REQUIRED_COMPONENTS_FOR_NON_AUTH_CLIENT_INTEREST = TArray<Worker_ComponentId>
{
	// Actor components
	UNREAL_METADATA_COMPONENT_ID,
	SPAWN_DATA_COMPONENT_ID,
	RPCS_ON_ENTITY_CREATION_ID,

	// Multicast RPCs
	MULTICAST_RPCS_COMPONENT_ID,
	NETMULTICAST_RPCS_COMPONENT_ID_LEGACY,

	// Global state components
	SINGLETON_MANAGER_COMPONENT_ID,
	DEPLOYMENT_MAP_COMPONENT_ID,
	STARTUP_ACTOR_MANAGER_COMPONENT_ID,
	GSM_SHUTDOWN_COMPONENT_ID,

	// Debugging information
	DEBUG_METRICS_COMPONENT_ID,
	SPATIAL_DEBUGGING_COMPONENT_ID
};

// A list of components clients require on entities they are authoritative over on top of the components already checked out by the interest query.
const TArray<Worker_ComponentId> REQUIRED_COMPONENTS_FOR_AUTH_CLIENT_INTEREST = TArray<Worker_ComponentId>
{
	// RPCs from the server
	SERVER_ENDPOINT_COMPONENT_ID,
	SERVER_RPC_ENDPOINT_COMPONENT_ID_LEGACY
};

// A list of components servers require on top of any generated data and handover components in order to handle non-authoritative actors correctly.
const TArray<Worker_ComponentId> REQUIRED_COMPONENTS_FOR_NON_AUTH_SERVER_INTEREST = TArray<Worker_ComponentId>
{
	// Actor components
	UNREAL_METADATA_COMPONENT_ID,
	SPAWN_DATA_COMPONENT_ID,
	RPCS_ON_ENTITY_CREATION_ID,

	// Multicast RPCs
	MULTICAST_RPCS_COMPONENT_ID,
	NETMULTICAST_RPCS_COMPONENT_ID_LEGACY,

	// Global state components
	SINGLETON_MANAGER_COMPONENT_ID,
	DEPLOYMENT_MAP_COMPONENT_ID,
	STARTUP_ACTOR_MANAGER_COMPONENT_ID,
	GSM_SHUTDOWN_COMPONENT_ID,

	// Unreal load balancing components
	VIRTUAL_WORKER_TRANSLATION_COMPONENT_ID
};

// A list of components servers require on entities they are authoritative over on top of the components already checked out by the interest query.
const TArray<Worker_ComponentId> REQUIRED_COMPONENTS_FOR_AUTH_SERVER_INTEREST = TArray<Worker_ComponentId>
{
	// RPCs from clients
	CLIENT_ENDPOINT_COMPONENT_ID,
	CLIENT_RPC_ENDPOINT_COMPONENT_ID_LEGACY,

	// Heartbeat
	HEARTBEAT_COMPONENT_ID
};

inline Worker_ComponentId RPCTypeToWorkerComponentIdLegacy(ERPCType RPCType)
{
	switch (RPCType)
	{
	case ERPCType::CrossServer:
	{
		return SpatialConstants::SERVER_TO_SERVER_COMMAND_ENDPOINT_COMPONENT_ID;
	}
	case ERPCType::NetMulticast:
	{
		return SpatialConstants::NETMULTICAST_RPCS_COMPONENT_ID_LEGACY;
	}
	case ERPCType::ClientReliable:
	case ERPCType::ClientUnreliable:
	{
		return SpatialConstants::SERVER_RPC_ENDPOINT_COMPONENT_ID_LEGACY;
	}
	case ERPCType::ServerReliable:
	case ERPCType::ServerUnreliable:
	{
		return SpatialConstants::CLIENT_RPC_ENDPOINT_COMPONENT_ID_LEGACY;
	}
	default:
		checkNoEntry();
		return SpatialConstants::INVALID_COMPONENT_ID;
	}
}

inline Worker_ComponentId GetClientAuthorityComponent(bool bUsingRingBuffers)
{
	return bUsingRingBuffers ? CLIENT_ENDPOINT_COMPONENT_ID : CLIENT_RPC_ENDPOINT_COMPONENT_ID_LEGACY;
}

inline WorkerAttributeSet GetLoadBalancerAttributeSet(FName LoadBalancingWorkerType)
{
	if (LoadBalancingWorkerType == "")
	{
		return { DefaultServerWorkerType.ToString() };
	}
	return { LoadBalancingWorkerType.ToString() };
}

} // ::SpatialConstants

DECLARE_STATS_GROUP(TEXT("SpatialNet"), STATGROUP_SpatialNet, STATCAT_Advanced);
