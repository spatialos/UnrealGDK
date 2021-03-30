// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#define LOCTEXT_NAMESPACE "SpatialConstants"

using namespace SpatialGDK;

namespace SpatialConstants
{

FString RPCTypeToString(ERPCType RPCType)
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
    case ERPCType::ServerAlwaysWrite:
        return TEXT("Server, AlwaysWrite");
    case ERPCType::NetMulticast:
        return TEXT("Multicast");
    case ERPCType::CrossServer:
        return TEXT("CrossServer");
    default:
        checkNoEntry();
    }

    return FString();
}

TOptional<ERPCType> RPCStringToType(const FString& String)
{
    static const TMap<FString, ERPCType> s_NamesMap = []
    {
        TMap<FString, ERPCType> NamesMap;
        for (uint8 RPCTypeIdx = static_cast<uint8>(ERPCType::RingBufferTypeBegin);
            RPCTypeIdx <= static_cast<uint8>(ERPCType::RingBufferTypeEnd); RPCTypeIdx++)
        {
            ERPCType RPCType = static_cast<ERPCType>(RPCTypeIdx);
            NamesMap.Add(RPCTypeToString(RPCType), RPCType);
        }
        return NamesMap;
    }();

    const ERPCType* Entry = s_NamesMap.Find(String);

    if (Entry)
    {
        return *Entry;
    }

    return {};
}


const FString SERVER_AUTH_COMPONENT_SET_NAME = TEXT("ServerAuthoritativeComponentSet");
const FString CLIENT_AUTH_COMPONENT_SET_NAME = TEXT("ClientAuthoritativeComponentSet");
const FString DATA_COMPONENT_SET_NAME = TEXT("DataComponentSet");
const FString OWNER_ONLY_COMPONENT_SET_NAME = TEXT("OwnerOnlyComponentSet");
const FString HANDOVER_COMPONENT_SET_NAME = TEXT("HandoverComponentSet");
const FString ROUTING_WORKER_COMPONENT_SET_NAME = TEXT("RoutingWorkerComponentSet");
const FString INITIAL_ONLY_COMPONENT_SET_NAME = TEXT("InitialOnlyComponentSet");

const PhysicalWorkerName TRANSLATOR_UNSET_PHYSICAL_NAME = FString("UnsetWorkerName");

const FName DefaultLayer = FName(TEXT("DefaultLayer"));

const FName RoutingWorkerType(TEXT("RoutingWorker"));
const FName StrategyWorkerType(TEXT("StrategyWorker"));

const FString ClientsStayConnectedURLOption = TEXT("clientsStayConnected");
const FString SpatialSessionIdURLOption = TEXT("spatialSessionId=");

const FString LOCATOR_HOST = TEXT("locator.improbable.io");
const FString LOCATOR_HOST_CN = TEXT("locator.spatialoschina.com");

const FString CONSOLE_HOST = TEXT("console.improbable.io");
const FString CONSOLE_HOST_CN = TEXT("console.spatialoschina.com");

const FString AssemblyPattern = TEXT("^[a-zA-Z0-9_.-]{5,64}$");
const FText AssemblyPatternHint =
LOCTEXT("AssemblyPatternHint",
    "Assembly name may only contain alphanumeric characters, '_', '.', or '-', and must be between 5 and 64 characters long.");
const FString ProjectPattern = TEXT("^[a-z0-9_]{3,32}$");
const FText ProjectPatternHint =
LOCTEXT("ProjectPatternHint",
    "Project name may only contain lowercase alphanumeric characters or '_', and must be between 3 and 32 characters long.");
const FString DeploymentPattern = TEXT("^[a-z0-9_]{2,32}$");
const FText DeploymentPatternHint =
LOCTEXT("DeploymentPatternHint",
    "Deployment name may only contain lowercase alphanumeric characters or '_', and must be between 2 and 32 characters long.");
const FString Ipv4Pattern = TEXT("^(?:[0-9]{1,3}\\.){3}[0-9]{1,3}$");


const FString SPATIALOS_METRICS_DYNAMIC_FPS = TEXT("Dynamic.FPS");
const FString RECONNECT_USING_COMMANDLINE_ARGUMENTS = TEXT("0.0.0.0");
const FString URL_LOGIN_OPTION = TEXT("login=");
const FString URL_PLAYER_IDENTITY_OPTION = TEXT("playeridentity=");
const FString URL_DEV_AUTH_TOKEN_OPTION = TEXT("devauthtoken=");
const FString URL_TARGET_DEPLOYMENT_OPTION = TEXT("deployment=");
const FString URL_PLAYER_ID_OPTION = TEXT("playerid=");
const FString URL_DISPLAY_NAME_OPTION = TEXT("displayname=");
const FString URL_METADATA_OPTION = TEXT("metadata=");
const FString URL_USE_EXTERNAL_IP_FOR_BRIDGE_OPTION = TEXT("useExternalIpForBridge");

const FString SHUTDOWN_PREPARATION_WORKER_FLAG = TEXT("PrepareShutdown");

const FString DEVELOPMENT_AUTH_PLAYER_ID = TEXT("Player Id");

const FString SCHEMA_DATABASE_FILE_PATH = TEXT("Spatial/SchemaDatabase");
const FString SCHEMA_DATABASE_ASSET_PATH = TEXT("/Game/Spatial/SchemaDatabase");

const FString EMPTY_TEST_MAP_PATH = TEXT("/SpatialGDK/Maps/Empty");

const FString DEV_LOGIN_TAG = TEXT("dev_login");

const TArray<Worker_ComponentId> REQUIRED_COMPONENTS_FOR_NON_AUTH_CLIENT_INTEREST =
TArray<Worker_ComponentId>{ // Actor components
                            UNREAL_METADATA_COMPONENT_ID, SPAWN_DATA_COMPONENT_ID, TOMBSTONE_COMPONENT_ID, TOMBSTONE_TAG_COMPONENT_ID,
                            DORMANT_COMPONENT_ID, INITIAL_ONLY_PRESENCE_COMPONENT_ID,

                            // Multicast RPCs
                            MULTICAST_RPCS_COMPONENT_ID,

                            // Global state components
                            DEPLOYMENT_MAP_COMPONENT_ID, STARTUP_ACTOR_MANAGER_COMPONENT_ID, GSM_SHUTDOWN_COMPONENT_ID,

                            // Debugging information
                            DEBUG_METRICS_COMPONENT_ID, SPATIAL_DEBUGGING_COMPONENT_ID,

                            // Strategy Worker LB components
                            ACTOR_SET_MEMBER_COMPONENT_ID, ACTOR_GROUP_MEMBER_COMPONENT_ID,

                            // Actor tag
                            ACTOR_TAG_COMPONENT_ID
};

const TArray<Worker_ComponentId> REQUIRED_COMPONENTS_FOR_AUTH_CLIENT_INTEREST =
TArray<Worker_ComponentId>{ // RPCs from the server
                            SERVER_ENDPOINT_COMPONENT_ID,

                            // Actor tags
                            ACTOR_TAG_COMPONENT_ID, ACTOR_AUTH_TAG_COMPONENT_ID
};

const TArray<Worker_ComponentId> REQUIRED_COMPONENTS_FOR_NON_AUTH_SERVER_INTEREST = TArray<Worker_ComponentId>{
    // Actor components
    UNREAL_METADATA_COMPONENT_ID, SPAWN_DATA_COMPONENT_ID, TOMBSTONE_COMPONENT_ID, DORMANT_COMPONENT_ID,
    NET_OWNING_CLIENT_WORKER_COMPONENT_ID,

    // Multicast RPCs
    MULTICAST_RPCS_COMPONENT_ID,

    // Global state components
    DEPLOYMENT_MAP_COMPONENT_ID, STARTUP_ACTOR_MANAGER_COMPONENT_ID, GSM_SHUTDOWN_COMPONENT_ID, SNAPSHOT_VERSION_COMPONENT_ID,

    // Unreal load balancing components
    VIRTUAL_WORKER_TRANSLATION_COMPONENT_ID,

    // Authority intent component to handle scattered hierarchies
    AUTHORITY_INTENT_COMPONENT_ID,

    // Tags: Well known entities, non-auth actors, and tombstone tags
    GDK_KNOWN_ENTITY_TAG_COMPONENT_ID, ACTOR_TAG_COMPONENT_ID, TOMBSTONE_TAG_COMPONENT_ID,

    // Strategy Worker LB components
    ACTOR_SET_MEMBER_COMPONENT_ID, ACTOR_GROUP_MEMBER_COMPONENT_ID,

    PLAYER_CONTROLLER_COMPONENT_ID, PARTITION_COMPONENT_ID
};

const TArray<Worker_ComponentId> REQUIRED_COMPONENTS_FOR_AUTH_SERVER_INTEREST =
TArray<Worker_ComponentId>{ // RPCs from clients
                            CLIENT_ENDPOINT_COMPONENT_ID,

                            // Player controller
                            PLAYER_CONTROLLER_COMPONENT_ID,

                            // Cross server endpoint
                            CROSSSERVER_SENDER_ACK_ENDPOINT_COMPONENT_ID, CROSSSERVER_RECEIVER_ENDPOINT_COMPONENT_ID,

                            // Actor tags
                            ACTOR_TAG_COMPONENT_ID, ACTOR_AUTH_TAG_COMPONENT_ID,

                            PARTITION_COMPONENT_ID
};

const TArray<FString> ServerAuthorityWellKnownSchemaImports = {
    "improbable/standard_library.schema",
    "unreal/gdk/authority_intent.schema",
    "unreal/gdk/debug_component.schema",
    "unreal/gdk/debug_metrics.schema",
    "unreal/gdk/net_owning_client_worker.schema",
    "unreal/gdk/not_streamed.schema",
    "unreal/gdk/query_tags.schema",
    "unreal/gdk/relevant.schema",
    "unreal/gdk/rpc_components.schema",
    "unreal/gdk/spatial_debugging.schema",
    "unreal/gdk/spawndata.schema",
    "unreal/gdk/tombstone.schema",
    "unreal/gdk/unreal_metadata.schema",
    "unreal/gdk/actor_group_member.schema",
    "unreal/gdk/actor_set_member.schema",
    "unreal/generated/rpc_endpoints.schema",
    "unreal/generated/NetCullDistance/ncdcomponents.schema",
};

const TMap<Worker_ComponentId, FString> ServerAuthorityWellKnownComponents = {
    { POSITION_COMPONENT_ID, "improbable.Position" },
    { INTEREST_COMPONENT_ID, "improbable.Interest" },
    { AUTHORITY_DELEGATION_COMPONENT_ID, "improbable.AuthorityDelegation" },
    { AUTHORITY_INTENT_COMPONENT_ID, "unreal.AuthorityIntent" },
    { GDK_DEBUG_COMPONENT_ID, "unreal.DebugComponent" },
    { DEBUG_METRICS_COMPONENT_ID, "unreal.DebugMetrics" },
    { NET_OWNING_CLIENT_WORKER_COMPONENT_ID, "unreal.NetOwningClientWorker" },
    { NOT_STREAMED_COMPONENT_ID, "unreal.NotStreamed" },
    { ALWAYS_RELEVANT_COMPONENT_ID, "unreal.AlwaysRelevant" },
    { DORMANT_COMPONENT_ID, "unreal.Dormant" },
    { VISIBLE_COMPONENT_ID, "unreal.Visible" },
    { SERVER_TO_SERVER_COMMAND_ENDPOINT_COMPONENT_ID, "unreal.UnrealServerToServerCommandEndpoint" },
    { SPATIAL_DEBUGGING_COMPONENT_ID, "unreal.SpatialDebugging" },
    { SPAWN_DATA_COMPONENT_ID, "unreal.SpawnData" },
    { TOMBSTONE_COMPONENT_ID, "unreal.Tombstone" },
    { UNREAL_METADATA_COMPONENT_ID, "unreal.UnrealMetadata" },
    { ACTOR_GROUP_MEMBER_COMPONENT_ID, "unreal.ActorGroupMember" },
    { ACTOR_SET_MEMBER_COMPONENT_ID, "unreal.ActorSetMember" },
    { SERVER_ENDPOINT_COMPONENT_ID, "unreal.generated.UnrealServerEndpoint" },
    { MULTICAST_RPCS_COMPONENT_ID, "unreal.generated.UnrealMulticastRPCs" },
    { SERVER_ENDPOINT_COMPONENT_ID, "unreal.generated.UnrealServerEndpoint" },
    { CROSSSERVER_SENDER_ENDPOINT_COMPONENT_ID, "unreal.generated.UnrealCrossServerSenderRPCs" },
    { CROSSSERVER_RECEIVER_ACK_ENDPOINT_COMPONENT_ID, "unreal.generated.UnrealCrossServerReceiverACKRPCs" },
};

const TArray<FString> ClientAuthorityWellKnownSchemaImports = { "unreal/gdk/player_controller.schema", "unreal/gdk/rpc_components.schema",
                                                                "unreal/generated/rpc_endpoints.schema" };

const TMap<Worker_ComponentId, FString> ClientAuthorityWellKnownComponents = {
    { PLAYER_CONTROLLER_COMPONENT_ID, "unreal.PlayerController" },
    { CLIENT_ENDPOINT_COMPONENT_ID, "unreal.generated.UnrealClientEndpoint" },
};

const TMap<Worker_ComponentId, FString> RoutingWorkerComponents = {
    { CROSSSERVER_SENDER_ACK_ENDPOINT_COMPONENT_ID, "unreal.generated.UnrealCrossServerSenderACKRPCs" },
    { CROSSSERVER_RECEIVER_ENDPOINT_COMPONENT_ID, "unreal.generated.UnrealCrossServerReceiverRPCs" },
};

const TArray<FString> RoutingWorkerSchemaImports = { "unreal/gdk/rpc_components.schema", "unreal/generated/rpc_endpoints.schema" };

const TArray<Worker_ComponentId> KnownEntityAuthorityComponents = { POSITION_COMPONENT_ID,		 METADATA_COMPONENT_ID,
                                                                    INTEREST_COMPONENT_ID,		 PLAYER_SPAWNER_COMPONENT_ID,
                                                                    DEPLOYMENT_MAP_COMPONENT_ID, STARTUP_ACTOR_MANAGER_COMPONENT_ID,
                                                                    GSM_SHUTDOWN_COMPONENT_ID,	 VIRTUAL_WORKER_TRANSLATION_COMPONENT_ID};

}

#undef LOCTEXT_NAMESPACE
