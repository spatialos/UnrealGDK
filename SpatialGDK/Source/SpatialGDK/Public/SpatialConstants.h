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
	ServerAlwaysWrite,
	NetMulticast,
	CrossServer,

	// Helpers to iterate RPC types with ring buffers
	RingBufferTypeBegin = ClientReliable,
	RingBufferTypeEnd = CrossServer
};

enum ESchemaComponentType : int32
{
	SCHEMA_Invalid = -1,

	// Properties
	SCHEMA_Data, // Represents properties being replicated to all workers
	SCHEMA_OwnerOnly,
	SCHEMA_Handover,
	SCHEMA_InitialOnly,

	SCHEMA_Count,

	// Iteration helpers
	SCHEMA_Begin = SCHEMA_Data,
};

namespace SpatialConstants
{
FString RPCTypeToString(ERPCType RPCType);
TOptional<ERPCType> RPCStringToType(const FString& String);

enum EntityIds
{
	INITIAL_SPAWNER_ENTITY_ID = 1,
	INITIAL_GLOBAL_STATE_MANAGER_ENTITY_ID = 2,
	INITIAL_VIRTUAL_WORKER_TRANSLATOR_ENTITY_ID = 3,
	INITIAL_SNAPSHOT_PARTITION_ENTITY_ID = 4,
	INITIAL_STRATEGY_PARTITION_ENTITY_ID = 5,
	INITIAL_ROUTING_PARTITION_ENTITY_ID = 6,
	FIRST_AVAILABLE_ENTITY_ID = 7,
};

const Worker_PartitionId INVALID_PARTITION_ID = INVALID_ENTITY_ID;

const Worker_ComponentId INVALID_COMPONENT_ID = 0;

const Worker_ComponentSetId SPATIALOS_WELLKNOWN_COMPONENTSET_ID = 50;
const Worker_ComponentId METADATA_COMPONENT_ID = 53;
const Worker_ComponentId POSITION_COMPONENT_ID = 54;
const Worker_ComponentId PERSISTENCE_COMPONENT_ID = 55;
const Worker_ComponentId INTEREST_COMPONENT_ID = 58;

// This is a marker component used by the Runtime to define which entities are system entities.
const Worker_ComponentId SYSTEM_COMPONENT_ID = 59;

// This is a component on per-worker system entities.
const Worker_ComponentId WORKER_COMPONENT_ID = 60;
const Worker_ComponentId PLAYERIDENTITY_COMPONENT_ID = 61;
const Worker_ComponentId AUTHORITY_DELEGATION_COMPONENT_ID = 65;
const Worker_ComponentId PARTITION_COMPONENT_ID = 66;

const Worker_ComponentId MAX_RESERVED_SPATIAL_SYSTEM_COMPONENT_ID = 100;

const Worker_ComponentId SPAWN_DATA_COMPONENT_ID = 9999;
const Worker_ComponentId PLAYER_SPAWNER_COMPONENT_ID = 9998;
const Worker_ComponentId UNREAL_METADATA_COMPONENT_ID = 9996;
const Worker_ComponentId GDK_DEBUG_COMPONENT_ID = 9995;
const Worker_ComponentId DEPLOYMENT_MAP_COMPONENT_ID = 9994;
const Worker_ComponentId STARTUP_ACTOR_MANAGER_COMPONENT_ID = 9993;
const Worker_ComponentId GSM_SHUTDOWN_COMPONENT_ID = 9992;
const Worker_ComponentId PLAYER_CONTROLLER_COMPONENT_ID = 9991;
const Worker_ComponentId SNAPSHOT_VERSION_COMPONENT_ID = 9990;

const Worker_ComponentSetId SERVER_AUTH_COMPONENT_SET_ID = 9900;
const Worker_ComponentSetId CLIENT_AUTH_COMPONENT_SET_ID = 9901;
const Worker_ComponentSetId DATA_COMPONENT_SET_ID = 9902;
const Worker_ComponentSetId OWNER_ONLY_COMPONENT_SET_ID = 9903;
const Worker_ComponentSetId HANDOVER_COMPONENT_SET_ID = 9904;
const Worker_ComponentSetId GDK_KNOWN_ENTITY_AUTH_COMPONENT_SET_ID = 9905;
const Worker_ComponentSetId ROUTING_WORKER_AUTH_COMPONENT_SET_ID = 9906;
const Worker_ComponentSetId INITIAL_ONLY_COMPONENT_SET_ID = 9907;
const Worker_ComponentSetId SERVER_WORKER_ENTITY_AUTH_COMPONENT_SET_ID = 9908;

extern const FString SERVER_AUTH_COMPONENT_SET_NAME;
extern const FString CLIENT_AUTH_COMPONENT_SET_NAME;
extern const FString DATA_COMPONENT_SET_NAME;
extern const FString OWNER_ONLY_COMPONENT_SET_NAME;
extern const FString HANDOVER_COMPONENT_SET_NAME;
extern const FString ROUTING_WORKER_COMPONENT_SET_NAME;
extern const FString INITIAL_ONLY_COMPONENT_SET_NAME;

const Worker_ComponentId NOT_STREAMED_COMPONENT_ID = 9986;
const Worker_ComponentId DEBUG_METRICS_COMPONENT_ID = 9984;
const Worker_ComponentId ALWAYS_RELEVANT_COMPONENT_ID = 9983;
const Worker_ComponentId TOMBSTONE_COMPONENT_ID = 9982;
const Worker_ComponentId DORMANT_COMPONENT_ID = 9981;
const Worker_ComponentId AUTHORITY_INTENT_COMPONENT_ID = 9980;
const Worker_ComponentId VIRTUAL_WORKER_TRANSLATION_COMPONENT_ID = 9979;
const Worker_ComponentId VISIBLE_COMPONENT_ID = 9970;
const Worker_ComponentId SERVER_ONLY_ALWAYS_RELEVANT_COMPONENT_ID = 9968;

const Worker_ComponentId CROSSSERVER_SENDER_ENDPOINT_COMPONENT_ID = 9960;
const Worker_ComponentId CROSSSERVER_SENDER_ACK_ENDPOINT_COMPONENT_ID = 9961;
const Worker_ComponentId CROSSSERVER_RECEIVER_ENDPOINT_COMPONENT_ID = 9962;
const Worker_ComponentId CROSSSERVER_RECEIVER_ACK_ENDPOINT_COMPONENT_ID = 9963;

const Worker_ComponentId CLIENT_ENDPOINT_COMPONENT_ID = 9978;
const Worker_ComponentId SERVER_ENDPOINT_COMPONENT_ID = 9977;
const Worker_ComponentId MULTICAST_RPCS_COMPONENT_ID = 9976;
const Worker_ComponentId SPATIAL_DEBUGGING_COMPONENT_ID = 9975;
const Worker_ComponentId SERVER_WORKER_COMPONENT_ID = 9974;
const Worker_ComponentId SERVER_TO_SERVER_COMMAND_ENDPOINT_COMPONENT_ID = 9973;
const Worker_ComponentId NET_OWNING_CLIENT_WORKER_COMPONENT_ID = 9971;
const Worker_ComponentId MIGRATION_DIAGNOSTIC_COMPONENT_ID = 9969;
const Worker_ComponentId PARTITION_SHADOW_COMPONENT_ID = 9967;
const Worker_ComponentId INITIAL_ONLY_PRESENCE_COMPONENT_ID = 9966;

const Worker_ComponentId ACTOR_SET_MEMBER_COMPONENT_ID = 9965;
const Worker_ComponentId ACTOR_GROUP_MEMBER_COMPONENT_ID = 9964;

const Worker_ComponentId STARTING_GENERATED_COMPONENT_ID = 10000;

// System query tags for entity completeness
const Worker_ComponentId FIRST_EC_COMPONENT_ID = 2001;
const Worker_ComponentId ACTOR_AUTH_TAG_COMPONENT_ID = 2001;
const Worker_ComponentId ACTOR_TAG_COMPONENT_ID = 2002;
const Worker_ComponentId LB_TAG_COMPONENT_ID = 2005;

const Worker_ComponentId GDK_KNOWN_ENTITY_TAG_COMPONENT_ID = 2007;
const Worker_ComponentId TOMBSTONE_TAG_COMPONENT_ID = 2008;
const Worker_ComponentId ROUTINGWORKER_TAG_COMPONENT_ID = 2009;
const Worker_ComponentId STRATEGYWORKER_TAG_COMPONENT_ID = 2010;
// Add component ids above here, this should always be last and be equal to the previous component id
const Worker_ComponentId LAST_EC_COMPONENT_ID = 2010;

const Schema_FieldId DEPLOYMENT_MAP_MAP_URL_ID = 1;
const Schema_FieldId DEPLOYMENT_MAP_ACCEPTING_PLAYERS_ID = 2;
const Schema_FieldId DEPLOYMENT_MAP_SESSION_ID = 3;
const Schema_FieldId DEPLOYMENT_MAP_SCHEMA_HASH = 4;

const Schema_FieldId SNAPSHOT_VERSION_NUMBER_ID = 1;

const Schema_FieldId STARTUP_ACTOR_MANAGER_CAN_BEGIN_PLAY_ID = 1;

const Schema_FieldId ACTOR_COMPONENT_REPLICATES_ID = 1;
const Schema_FieldId ACTOR_TEAROFF_ID = 3;

const Schema_FieldId SHUTDOWN_MULTI_PROCESS_REQUEST_ID = 1;
const Schema_FieldId SHUTDOWN_ADDITIONAL_SERVERS_EVENT_ID = 1;

// DebugMetrics command IDs
const Schema_FieldId DEBUG_METRICS_START_RPC_METRICS_ID = 1;
const Schema_FieldId DEBUG_METRICS_STOP_RPC_METRICS_ID = 2;
const Schema_FieldId DEBUG_METRICS_MODIFY_SETTINGS_ID = 3;

// ModifySettingPayload Field IDs
const Schema_FieldId MODIFY_SETTING_PAYLOAD_NAME_ID = 1;
const Schema_FieldId MODIFY_SETTING_PAYLOAD_VALUE_ID = 2;

// UnrealObjectRef Field IDs
const Schema_FieldId UNREAL_OBJECT_REF_ENTITY_ID = 1;
const Schema_FieldId UNREAL_OBJECT_REF_OFFSET_ID = 2;
const Schema_FieldId UNREAL_OBJECT_REF_PATH_ID = 3;
const Schema_FieldId UNREAL_OBJECT_REF_NO_LOAD_ON_CLIENT_ID = 4;
const Schema_FieldId UNREAL_OBJECT_REF_OUTER_ID = 5;
const Schema_FieldId UNREAL_OBJECT_REF_USE_CLASS_PATH_TO_LOAD_ID = 6;

// UnrealRPCPayload Field IDs
const Schema_FieldId UNREAL_RPC_PAYLOAD_OFFSET_ID = 1;
const Schema_FieldId UNREAL_RPC_PAYLOAD_RPC_INDEX_ID = 2;
const Schema_FieldId UNREAL_RPC_PAYLOAD_RPC_PAYLOAD_ID = 3;
const Schema_FieldId UNREAL_RPC_PAYLOAD_TRACE_ID = 4;
const Schema_FieldId UNREAL_RPC_PAYLOAD_RPC_ID = 5;

const Schema_FieldId UNREAL_RPC_TRACE_ID = 1;
const Schema_FieldId UNREAL_RPC_SPAN_ID = 2;

// Unreal(Client|Server|Multicast)RPCEndpoint Field IDs
const Schema_FieldId UNREAL_RPC_ENDPOINT_READY_ID = 1;
const Schema_FieldId UNREAL_RPC_ENDPOINT_EVENT_ID = 1;
const Schema_FieldId UNREAL_RPC_ENDPOINT_COMMAND_ID = 1;

const Schema_FieldId PLAYER_SPAWNER_SPAWN_PLAYER_COMMAND_ID = 1;

// AuthorityIntent codes and Field IDs.
const Schema_FieldId AUTHORITY_INTENT_VIRTUAL_WORKER_ID = 1;

// VirtualWorkerTranslation Field IDs.
const Schema_FieldId VIRTUAL_WORKER_TRANSLATION_MAPPING_ID = 1;
const Schema_FieldId MAPPING_VIRTUAL_WORKER_ID = 1;
const Schema_FieldId MAPPING_PHYSICAL_WORKER_NAME_ID = 2;
const Schema_FieldId MAPPING_SERVER_WORKER_ENTITY_ID = 3;
const Schema_FieldId MAPPING_PARTITION_ID = 4;
extern const PhysicalWorkerName TRANSLATOR_UNSET_PHYSICAL_NAME;

// WorkerEntity Field IDs.
const Schema_FieldId WORKER_ID_ID = 1;
const Schema_FieldId WORKER_TYPE_ID = 2;

// WorkerEntity command IDs
const Schema_FieldId WORKER_DISCONNECT_COMMAND_ID = 1;
const Schema_FieldId WORKER_CLAIM_PARTITION_COMMAND_ID = 2;

// SpatialDebugger Field IDs.
const Schema_FieldId SPATIAL_DEBUGGING_AUTHORITATIVE_VIRTUAL_WORKER_ID = 1;
const Schema_FieldId SPATIAL_DEBUGGING_AUTHORITATIVE_COLOR = 2;
const Schema_FieldId SPATIAL_DEBUGGING_INTENT_VIRTUAL_WORKER_ID = 3;
const Schema_FieldId SPATIAL_DEBUGGING_INTENT_COLOR = 4;
const Schema_FieldId SPATIAL_DEBUGGING_IS_LOCKED = 5;

// ServerWorker Field IDs.
const Schema_FieldId SERVER_WORKER_NAME_ID = 1;
const Schema_FieldId SERVER_WORKER_READY_TO_BEGIN_PLAY_ID = 2;
const Schema_FieldId SERVER_WORKER_SYSTEM_ENTITY_ID = 3;
const Schema_FieldId SERVER_WORKER_FORWARD_SPAWN_REQUEST_COMMAND_ID = 1;

// SpawnPlayerRequest type IDs.
const Schema_FieldId SPAWN_PLAYER_URL_ID = 1;
const Schema_FieldId SPAWN_PLAYER_UNIQUE_ID = 2;
const Schema_FieldId SPAWN_PLAYER_PLATFORM_NAME_ID = 3;
const Schema_FieldId SPAWN_PLAYER_IS_SIMULATED_ID = 4;
const Schema_FieldId SPAWN_PLAYER_CLIENT_SYSTEM_ENTITY_ID = 5;

// ForwardSpawnPlayerRequest type IDs.
const Schema_FieldId FORWARD_SPAWN_PLAYER_DATA_ID = 1;
const Schema_FieldId FORWARD_SPAWN_PLAYER_START_ACTOR_ID = 2;
const Schema_FieldId FORWARD_SPAWN_PLAYER_CLIENT_SYSTEM_ENTITY_ID = 3;
const Schema_FieldId FORWARD_SPAWN_PLAYER_RESPONSE_SUCCESS_ID = 1;

// NetOwningClientWorker Field IDs.
const Schema_FieldId NET_OWNING_CLIENT_PARTITION_ENTITY_FIELD_ID = 1;

// UnrealMetadata Field IDs.
const Schema_FieldId UNREAL_METADATA_STABLY_NAMED_REF_ID = 1;
const Schema_FieldId UNREAL_METADATA_CLASS_PATH_ID = 2;
const Schema_FieldId UNREAL_METADATA_NET_STARTUP_ID = 3;

// Migration diagnostic Field IDs
const Schema_FieldId MIGRATION_DIAGNOSTIC_COMMAND_ID = 1;

// MigrationDiagnosticRequest type IDs.
const Schema_FieldId MIGRATION_DIAGNOSTIC_AUTHORITY_WORKER_ID = 1;
const Schema_FieldId MIGRATION_DIAGNOSTIC_ENTITY_ID = 2;
const Schema_FieldId MIGRATION_DIAGNOSTIC_REPLICATES_ID = 3;
const Schema_FieldId MIGRATION_DIAGNOSTIC_HAS_AUTHORITY_ID = 4;
const Schema_FieldId MIGRATION_DIAGNOSTIC_LOCKED_ID = 5;
const Schema_FieldId MIGRATION_DIAGNOSTIC_EVALUATION_ID = 6;
const Schema_FieldId MIGRATION_DIAGNOSTIC_DESTINATION_WORKER_ID = 7;
const Schema_FieldId MIGRATION_DIAGNOSTIC_OWNER_ID = 8;

// Worker component field IDs
const Schema_FieldId WORKER_COMPONENT_WORKER_ID_ID = 1;
const Schema_FieldId WORKER_COMPONENT_WORKER_TYPE_ID = 2;

// Partition component field IDs
const Schema_FieldId PARTITION_COMPONENT_WORKER_ID = 1;

// ActorSetMember field IDs
const Schema_FieldId ACTOR_SET_MEMBER_COMPONENT_LEADER_ENTITY_ID = 1;

// ActorGroupMember field IDs
const Schema_FieldId ACTOR_GROUP_MEMBER_COMPONENT_ACTOR_GROUP_ID = 1;

// Reserved entity IDs expire in 5 minutes, we will refresh them every 3 minutes to be safe.
const float ENTITY_RANGE_EXPIRATION_INTERVAL_SECONDS = 180.0f;

const float FIRST_COMMAND_RETRY_WAIT_SECONDS = 0.2f;
const uint32 MAX_NUMBER_COMMAND_ATTEMPTS = 5u;
const float FORWARD_PLAYER_SPAWN_COMMAND_WAIT_SECONDS = 0.2f;

const VirtualWorkerId INVALID_VIRTUAL_WORKER_ID = 0;
const ActorLockToken INVALID_ACTOR_LOCK_TOKEN = 0;
const FString INVALID_WORKER_NAME;

extern const FName DefaultLayer;

extern const FName StrategyWorkerType;
extern const FName RoutingWorkerType;

extern const FString ClientsStayConnectedURLOption;
extern const FString SpatialSessionIdURLOption;

extern const FString LOCATOR_HOST;
extern const FString LOCATOR_HOST_CN;
const uint16 LOCATOR_PORT = 443;

extern const FString CONSOLE_HOST;
extern const FString CONSOLE_HOST_CN;

extern const FString AssemblyPattern;
extern const FText AssemblyPatternHint;
extern const FString ProjectPattern;
extern const FText ProjectPatternHint;
extern const FString DeploymentPattern;
extern const FText DeploymentPatternHint;
extern const FString Ipv4Pattern;

inline float GetCommandRetryWaitTimeSeconds(uint32 NumAttempts)
{
	// Double the time to wait on each failure.
	uint32 WaitTimeExponentialFactor = 1u << (NumAttempts - 1);
	return FIRST_COMMAND_RETRY_WAIT_SECONDS * WaitTimeExponentialFactor;
}

const FString LOCAL_HOST = TEXT("127.0.0.1");
const uint16 DEFAULT_PORT = 7777;

const uint16 DEFAULT_SERVER_RECEPTIONIST_PROXY_PORT = 7777;

const float ENTITY_QUERY_RETRY_WAIT_SECONDS = 3.0f;

const Worker_ComponentId MIN_EXTERNAL_SCHEMA_ID = 1000;
const Worker_ComponentId MAX_EXTERNAL_SCHEMA_ID = 2000;

extern const FString SPATIALOS_METRICS_DYNAMIC_FPS;

// URL that can be used to reconnect using the command line arguments.
extern const FString RECONNECT_USING_COMMANDLINE_ARGUMENTS;
extern const FString URL_LOGIN_OPTION;
extern const FString URL_PLAYER_IDENTITY_OPTION;
extern const FString URL_DEV_AUTH_TOKEN_OPTION;
extern const FString URL_TARGET_DEPLOYMENT_OPTION;
extern const FString URL_PLAYER_ID_OPTION;
extern const FString URL_DISPLAY_NAME_OPTION;
extern const FString URL_METADATA_OPTION;
extern const FString URL_USE_EXTERNAL_IP_FOR_BRIDGE_OPTION;

extern const FString SHUTDOWN_PREPARATION_WORKER_FLAG;

extern const FString DEVELOPMENT_AUTH_PLAYER_ID;

extern const FString SCHEMA_DATABASE_FILE_PATH;
extern const FString SCHEMA_DATABASE_ASSET_PATH;

// An empty map with the game mode override set to GameModeBase.
extern const FString EMPTY_TEST_MAP_PATH;

extern const FString DEV_LOGIN_TAG;

// A list of components clients require on top of any generated data components in order to handle non-authoritative actors correctly.
extern const TArray<Worker_ComponentId> REQUIRED_COMPONENTS_FOR_NON_AUTH_CLIENT_INTEREST;

// A list of components clients require on entities they are authoritative over on top of the components already checked out by the interest
// query.
extern const TArray<Worker_ComponentId> REQUIRED_COMPONENTS_FOR_AUTH_CLIENT_INTEREST;

// A list of components servers require on top of any generated data and handover components in order to handle non-authoritative actors
// correctly.
extern const TArray<Worker_ComponentId> REQUIRED_COMPONENTS_FOR_NON_AUTH_SERVER_INTEREST;

// A list of components servers require on entities they are authoritative over on top of the components already checked out by the interest
// query.
extern const TArray<Worker_ComponentId> REQUIRED_COMPONENTS_FOR_AUTH_SERVER_INTEREST;

inline bool IsEntityCompletenessComponent(Worker_ComponentId ComponentId)
{
	return ComponentId >= SpatialConstants::FIRST_EC_COMPONENT_ID && ComponentId <= SpatialConstants::LAST_EC_COMPONENT_ID;
}

// TODO: These containers should be cleaned up when we move to reading component set data directly from schema bundle - UNR-4666
extern const TArray<FString> ServerAuthorityWellKnownSchemaImports;
extern const TMap<Worker_ComponentId, FString> ServerAuthorityWellKnownComponents;
extern const TArray<FString> ClientAuthorityWellKnownSchemaImports;
extern const TMap<Worker_ComponentId, FString> ClientAuthorityWellKnownComponents;
extern const TMap<Worker_ComponentId, FString> RoutingWorkerComponents;
extern const TArray<FString> RoutingWorkerSchemaImports;
extern const TArray<Worker_ComponentId> KnownEntityAuthorityComponents;

//
// SPATIAL_SNAPSHOT_VERSION is the current version of supported snapshots.
//
// Snapshots can become invalid for multiple reasons, including but limited too:
//	- Any schema changes that affect snapshots
//	- New entities added to the default snapshot
//
// If you make any schema changes (that affect snapshots), a test will fail and provide the expected hash value that matches the new schema:
//	- Change SPATIAL_SNAPSHOT_SCHEMA_HASH (below) to this new hash value.
//
// The test that will fail is:
// 'GIVEN_snapshot_affecting_schema_files_WHEN_hash_of_file_contents_is_generated_THEN_hash_matches_expected_snapshot_version_hash'
//
// If you make *any* change that affects snapshots, including schema changes, adding new entities, etc:
//	- Increment SPATIAL_SNAPSHOT_VERSION_INC (below)
//

constexpr uint32 SPATIAL_SNAPSHOT_SCHEMA_HASH = 679237978;
constexpr uint32 SPATIAL_SNAPSHOT_VERSION_INC = 3;
constexpr uint64 SPATIAL_SNAPSHOT_VERSION = ((((uint64)SPATIAL_SNAPSHOT_SCHEMA_HASH) << 32) | SPATIAL_SNAPSHOT_VERSION_INC);

} // namespace SpatialConstants

DECLARE_STATS_GROUP(TEXT("SpatialNet"), STATGROUP_SpatialNet, STATCAT_Advanced);

#undef LOCTEXT_NAMESPACE
