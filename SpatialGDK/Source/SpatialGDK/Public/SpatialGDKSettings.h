// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "Engine/EngineTypes.h"
#include "Misc/Paths.h"
#include "Utils/GDKPropertyMacros.h"
#include "Utils/RPCContainer.h"

#include "SpatialGDKSettings.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(LogSpatialGDKSettings, Log, All);

class ASpatialDebugger;

/**
 * Enum that maps Unreal's log verbosity to allow use in settings.
 */
UENUM()
namespace ESettingsWorkerLogVerbosity
{
enum Type
{
	NoLogging = 0,
	Fatal,
	Error,
	Warning,
	Display,
	Log,
	Verbose,
	VeryVerbose,
};
}

UENUM()
namespace EServicesRegion
{
enum Type
{
	Default,
	CN
};
}

USTRUCT(BlueprintType)
struct FDistanceFrequencyPair
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "SpatialGDK")
	float DistanceRatio;

	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "SpatialGDK")
	float Frequency;
};

UCLASS(config = SpatialGDKSettings, defaultconfig)
class SPATIALGDK_API USpatialGDKSettings : public UObject
{
	GENERATED_BODY()

public:
	USpatialGDKSettings(const FObjectInitializer& ObjectInitializer);

#if WITH_EDITOR
	virtual void PostEditChangeProperty(struct FPropertyChangedEvent& PropertyChangedEvent) override;
#endif

	virtual void PostInitProperties() override;

	/**
	 * The number of entity IDs to be reserved when the entity pool is first created. Ensure that the number of entity IDs
	 * reserved is greater than the number of Actors that you expect the server-worker instances to spawn at game deployment
	 */
	UPROPERTY(EditAnywhere, config, Category = "Entity Pool", meta = (DisplayName = "Initial Entity ID Reservation Count"))
	uint32 EntityPoolInitialReservationCount;

	/**
	 * Specifies when the SpatialOS Runtime should reserve a new batch of entity IDs: the value is the number of un-used entity
	 * IDs left in the entity pool which triggers the SpatialOS Runtime to reserve new entity IDs
	 */
	UPROPERTY(EditAnywhere, config, Category = "Entity Pool", meta = (DisplayName = "Pool Refresh Threshold"))
	uint32 EntityPoolRefreshThreshold;

	/**
	 * Specifies the number of new entity IDs the SpatialOS Runtime reserves when `Pool refresh threshold` triggers a new batch.
	 */
	UPROPERTY(EditAnywhere, config, Category = "Entity Pool", meta = (DisplayName = "Refresh Count"))
	uint32 EntityPoolRefreshCount;

	/**
	 * Specifies the amount of time, in seconds, between heartbeat events sent from a game client to notify the server-worker instances
	 * that it's connected.
	 */
	UPROPERTY(EditAnywhere, config, Category = "Heartbeat", meta = (DisplayName = "Heartbeat Interval (seconds)"))
	float HeartbeatIntervalSeconds;

	/**
	 * Specifies the maximum amount of time, in seconds, that the server-worker instances wait for a game client to send heartbeat events.
	 * (If the timeout expires, the game client has disconnected.)
	 */
	UPROPERTY(EditAnywhere, config, Category = "Heartbeat", meta = (DisplayName = "Heartbeat Timeout (seconds)"))
	float HeartbeatTimeoutSeconds;

	/**
	 * Same as HeartbeatTimeoutSeconds, but used if WITH_EDITOR is defined.
	 */
	UPROPERTY(EditAnywhere, config, Category = "Heartbeat", meta = (DisplayName = "Heartbeat Timeout With Editor (seconds)"))
	float HeartbeatTimeoutWithEditorSeconds;

	/**
	 * Specifies the maximum number of Actors replicated per tick. Not respected when using the Replication Graph.
	 * Default: `0` per tick  (no limit)
	 * (If you set the value to ` 0`, the SpatialOS Runtime replicates every Actor per tick; this forms a large SpatialOS  world, affecting
	 * the performance of both game clients and server-worker instances.) You can use the `stat Spatial` flag when you run project builds to
	 * find the number of calls to `ReplicateActor`, and then use this number for reference.
	 */
	UPROPERTY(EditAnywhere, config, Category = "Replication", meta = (DisplayName = "Maximum Actors replicated per tick"))
	uint32 ActorReplicationRateLimit;

	/**
	 * Specifies the maximum number of entities created by the SpatialOS Runtime per tick. Not respected when using the Replication Graph.
	 * (The SpatialOS Runtime handles entity creation separately from Actor replication to ensure it can handle entity creation requests
	 * under load.) Note: if you set the value to 0, there is no limit to the number of entities created per tick. However, too many
	 * entities created at the same time might overload the SpatialOS Runtime, which can negatively affect your game. Default: `0` per tick
	 * (no limit)
	 */
	UPROPERTY(EditAnywhere, config, Category = "Replication", meta = (DisplayName = "Maximum entities created per tick"))
	uint32 EntityCreationRateLimit;

	/**
	 * When enabled, only entities which are in the net relevancy range of player controllers will be replicated to SpatialOS. Not respected
	 * when using the Replication Graph. This should only be used in single server configurations. The state of the world in the inspector
	 * will no longer be up to date.
	 */
	UPROPERTY(EditAnywhere, config, Category = "Replication", meta = (DisplayName = "Only Replicate Net Relevant Actors"))
	bool bUseIsActorRelevantForConnection;

	/**
	 * Specifies the rate, in number of times per second, at which server-worker instance updates are sent to and received from the
	 * SpatialOS Runtime. Default:1000/s
	 */
	UPROPERTY(EditAnywhere, config, Category = "Replication", meta = (DisplayName = "SpatialOS Network Update Rate"))
	float OpsUpdateRate;

	/**
	 * Maximum NetCullDistanceSquared value used in Spatial networking. Not respected when using the Replication Graph.
	 * Set to 0.0 to disable. This is temporary and will be removed when the runtime issue is resolved.
	 */
	UPROPERTY(EditAnywhere, config, Category = "Replication")
	float MaxNetCullDistanceSquared;

	/** Seconds to wait before executing a received RPC substituting nullptr for unresolved UObjects*/
	UPROPERTY(EditAnywhere, config, Category = "Replication",
			  meta = (DisplayName = "Wait Time Before Processing Received RPC With Unresolved Refs"))
	float QueuedIncomingRPCWaitTime;

	/** Seconds to wait before attempting to reprocess queued incoming RPCs */
	UPROPERTY(EditAnywhere, config, Category = "Replication",
			  meta = (DisplayName = "Wait Time Before Attempting To Reprocess Queued Incoming RPCs"))
	float QueuedIncomingRPCRetryTime;

	/** Seconds to wait before retying all queued outgoing RPCs. If 0 there will not be retried on a timer. */
	UPROPERTY(EditAnywhere, config, Category = "Replication", meta = (DisplayName = "Wait Time Before Retrying Outoing RPC"))
	float QueuedOutgoingRPCRetryTime;

	/**
	 * Minimum time, in seconds, required to pass before an Actor will update its SpatialOS Position, if it has also traveled more than the
	 * PositionUpdateLowerThresholdCentimeters since its last update.
	 */
	UPROPERTY(EditAnywhere, config, Category = "SpatialOS Position Updates")
	float PositionUpdateLowerThresholdSeconds;

	/**
	 * Minimum distance, in centimeters, required for an Actor to move before its SpatialOS Position is updated, if more than
	 * PositionUpdateLowerThresholdSeconds seconds have also passed since its last update.
	 */
	UPROPERTY(EditAnywhere, config, Category = "SpatialOS Position Updates")
	float PositionUpdateLowerThresholdCentimeters;

	/**
	 * Maximum time, in seconds, that can pass before an Actor will update its SpatialOS Position, if it has also traveled any non-null
	 * amount of centimeters since its last update.
	 */
	UPROPERTY(EditAnywhere, config, Category = "SpatialOS Position Updates")
	float PositionUpdateThresholdMaxSeconds;

	/** Maximum distance, in centimeters, an Actor can move before its SpatialOS Position is updated.*/
	UPROPERTY(EditAnywhere, config, Category = "SpatialOS Position Updates")
	float PositionUpdateThresholdMaxCentimeters;

	/** Metrics about client and server performance can be reported to SpatialOS to monitor a deployments health.*/
	UPROPERTY(EditAnywhere, config, Category = "Metrics")
	bool bEnableMetrics;

	/** Display server metrics on clients.*/
	UPROPERTY(EditAnywhere, config, Category = "Metrics")
	bool bEnableMetricsDisplay;

	/** Frequency that metrics are reported to SpatialOS.*/
	UPROPERTY(EditAnywhere, config, Category = "Metrics", meta = (DisplayName = "Metrics Report Rate (seconds)"))
	float MetricsReportRate;

	/**
	 * By default the SpatialOS Runtime reports server-worker instance’s load in frames per second (FPS).
	 * Select this to switch so it reports as seconds per frame.
	 * This value is visible as 'Load' in the Inspector, next to each worker.
	 */
	UPROPERTY(EditAnywhere, config, Category = "Metrics")
	bool bUseFrameTimeAsLoad;

	/** Batch entity position updates to be processed on a single frame.*/
	UPROPERTY(config)
	bool bBatchSpatialPositionUpdates;

	/** Maximum number of ActorComponents/Subobjects of the same class that can be attached to an Actor.*/
	UPROPERTY(EditAnywhere, config, Category = "Schema Generation",
			  meta = (DisplayName = "Maximum Dynamically Attached Subobjects Per Class"))
	uint32 MaxDynamicallyAttachedSubobjectsPerClass;

	/** The receptionist host to use if no 'receptionistHost' argument is passed to the command line. */
	UPROPERTY(EditAnywhere, config, Category = "Local Connection")
	FString DefaultReceptionistHost;

private:
	/** Will stop a non editor client auto connecting via command line args to a cloud deployment */
	UPROPERTY(EditAnywhere, config, Category = "Cloud Connection")
	bool bPreventClientCloudDeploymentAutoConnect;

public:
	bool GetPreventClientCloudDeploymentAutoConnect() const;

	UPROPERTY(EditAnywhere, Config, Category = "Region settings",
			  meta = (ConfigRestartRequired = true, DisplayName = "Region where services are located"))
	TEnumAsByte<EServicesRegion::Type> ServicesRegion;

	/** Deprecated!
	Upgraded into the two settings below for local/cloud configurations.
	Ticket for removal UNR-4348 */
	UPROPERTY(config, meta = (DeprecatedProperty, DeprecationMessage = "Use LocalWorkerLogLevel or CloudWorkerLogLevel"))
	TEnumAsByte<ESettingsWorkerLogVerbosity::Type> WorkerLogLevel;

	/** Controls the verbosity of worker logs which are sent to SpatialOS. These logs will appear in the Spatial Output and launch.log */
	UPROPERTY(EditAnywhere, config, Category = "Logging", meta = (DisplayName = "Local Worker Log Level"))
	TEnumAsByte<ESettingsWorkerLogVerbosity::Type> LocalWorkerLogLevel;

	/** Controls the verbosity of worker logs which are sent to SpatialOS. These logs will appear in the Spatial Output and launch.log */
	UPROPERTY(EditAnywhere, config, Category = "Logging", meta = (DisplayName = "Cloud Worker Log Level"))
	TEnumAsByte<ESettingsWorkerLogVerbosity::Type> CloudWorkerLogLevel;

	UPROPERTY(EditAnywhere, config, Category = "Debug", meta = (MetaClass = "SpatialDebugger"))
	TSubclassOf<ASpatialDebugger> SpatialDebugger;

	/** Enables multi-worker, if false uses single worker strategy in the editor.  */
	UPROPERTY(EditAnywhere, config, Category = "Load Balancing", meta = (DisplayName = "Enable multi-worker in editor"))
	bool bEnableMultiWorker;

	/** RPC ring buffers is enabled when either the matching setting is set, or load balancing is enabled */
	bool UseRPCRingBuffer() const;

#if WITH_EDITOR
	void SetMultiWorkerEditorEnabled(const bool bIsEnabled);
	FORCEINLINE bool IsMultiWorkerEditorEnabled() const { return bEnableMultiWorker; }
#endif // WITH_EDITOR

private:
#if WITH_EDITOR
	bool CanEditChange(const GDK_PROPERTY(Property) * InProperty) const override;

	void UpdateServicesRegionFile();
#endif

	UPROPERTY(EditAnywhere, Config, Category = "Replication", meta = (DisplayName = "Use RPC Ring Buffers"))
	bool bUseRPCRingBuffers;

	UPROPERTY(EditAnywhere, Config, Category = "Replication", meta = (DisplayName = "Default RPC Ring Buffer Size"))
	uint32 DefaultRPCRingBufferSize;

	/** Overrides default ring buffer size. */
	UPROPERTY(EditAnywhere, Config, Category = "Replication", meta = (DisplayName = "RPC Ring Buffer Size Map"))
	TMap<ERPCType, uint32> RPCRingBufferSizeMap;

public:
	uint32 GetRPCRingBufferSize(ERPCType RPCType) const;

	float GetSecondsBeforeWarning(const ERPCResult Result) const;

	bool ShouldRPCTypeAllowUnresolvedParameters(const ERPCType Type) const;

	/**
	 * The number of fields that the endpoint schema components are generated with. Changing this will require schema to be regenerated and
	 * break snapshot compatibility.
	 */
	UPROPERTY(EditAnywhere, Config, Category = "Replication", meta = (DisplayName = "Max RPC Ring Buffer Size"))
	uint32 MaxRPCRingBufferSize;

	/** Only valid on Tcp connections - indicates if we should enable TCP_NODELAY - see c_worker.h */
	UPROPERTY(Config)
	bool bTcpNoDelay;

	/** Only valid on Udp connections - specifies server downstream flush interval - see c_worker.h */
	UPROPERTY(Config)
	uint32 UdpServerDownstreamUpdateIntervalMS;

	/** Only valid on Udp connections - specifies client downstream flush interval - see c_worker.h */
	UPROPERTY(Config)
	uint32 UdpClientDownstreamUpdateIntervalMS;

	/** Will flush worker messages immediately after every RPC. Higher bandwidth but lower latency on RPC calls. */
	UPROPERTY(Config)
	bool bWorkerFlushAfterOutgoingNetworkOp;

	/** Do async loading for new classes when checking out entities. */
	UPROPERTY(Config)
	bool bAsyncLoadNewClassesOnEntityCheckout;

	UPROPERTY(EditAnywhere, config, Category = "Queued RPC Warning Timeouts", AdvancedDisplay,
			  meta = (DisplayName = "For a given RPC failure type, the time it will queue before reporting warnings to the logs."))
	TMap<ERPCResult, float> RPCQueueWarningTimeouts;

	UPROPERTY(EditAnywhere, config, Category = "Queued RPC Warning Timeouts", AdvancedDisplay,
			  meta = (DisplayName = "Default time before a queued RPC will start reporting warnings to the logs."))
	float RPCQueueWarningDefaultTimeout;

	FORCEINLINE bool IsRunningInChina() const { return ServicesRegion == EServicesRegion::CN; }

	void SetServicesRegion(EServicesRegion::Type NewRegion);

	/** Enable to use the new net cull distance component tagging form of interest */
	UPROPERTY(EditAnywhere, Config, Category = "Interest")
	bool bEnableNetCullDistanceInterest;

	/** Enable to use interest frequency with bEnableNetCullDistanceInterest*/
	UPROPERTY(EditAnywhere, Config, Category = "Interest", meta = (EditCondition = "bEnableNetCullDistanceInterest"))
	bool bEnableNetCullDistanceFrequency;

	/** Full update frequency ratio of actor's net cull distance */
	UPROPERTY(EditAnywhere, Config, Category = "Interest", meta = (EditCondition = "bEnableNetCullDistanceFrequency"))
	float FullFrequencyNetCullDistanceRatio;

	/** QBI pairs for ratio of - net cull distance : update frequency */
	UPROPERTY(EditAnywhere, Config, Category = "Interest", meta = (EditCondition = "bEnableNetCullDistanceFrequency"))
	TArray<FDistanceFrequencyPair> InterestRangeFrequencyPairs;

	/** Use TLS encryption for UnrealClient workers connection. May impact performance. Only works in non-editor builds. */
	UPROPERTY(EditAnywhere, Config, Category = "Connection", meta = (DisplayName = "Use Secure Client Connection In Packaged Builds"))
	bool bUseSecureClientConnection;

	/** Use TLS encryption for UnrealWorker (server) workers connection. May impact performance. Only works in non-editor builds. */
	UPROPERTY(EditAnywhere, Config, Category = "Connection", meta = (DisplayName = "Use Secure Server Connection In Packaged Builds"))
	bool bUseSecureServerConnection;

	/**
	 * Enable to ensure server workers always express interest such that any server is interested in a super set of
	 * client interest. This will cause servers to make most of the same queries as their delegated client queries.
	 * Intended to be used in development before interest due to the LB strategy ensures correct functionality.
	 */
	UPROPERTY(EditAnywhere, Config, Category = "Interest")
	bool bEnableClientQueriesOnServer;

	/**
	 * By default, load balancing config will be read from the WorldSettings, but this can be toggled to override
	 * the multi-worker settings class
	 */
	TOptional<FString> OverrideMultiWorkerSettingsClass;

	/**
	 * This will allow Actors to be spawned on a layer different to the intended authoritative layer.
	 */
	UPROPERTY(Config)
	bool bEnableCrossLayerActorSpawning;

	// clang-format off
	UPROPERTY(EditAnywhere, Config, Category = "Logging", AdvancedDisplay,
		meta = (DisplayName = "Whether or not to suppress a warning if an RPC of Type is being called with unresolved references. Default is false.  QueuedIncomingWaitRPC time is still respected."))
	// clang-format on
	TMap<ERPCType, bool> RPCTypeAllowUnresolvedParamMap;

	/**
	 * Time in seconds, controls at which frequency logs related to startup are emitted.
	 */
	UPROPERTY(EditAnywhere, Config, Category = "Logging", AdvancedDisplay)
	float StartupLogRate;

	/**
	 * Time in seconds, controls at which frequency the logs related to failed actor migration are emitted.
	 */
	UPROPERTY(EditAnywhere, Config, Category = "Logging", AdvancedDisplay)
	float ActorMigrationLogRate;
	/*
	 * -- EXPERIMENTAL --
	 * This will enable event tracing for the Unreal client/worker.
	 */
	UPROPERTY(EditAnywhere, Config, Category = "Event Tracing")
	bool bEventTracingEnabled;

	/*
	 * -- EXPERIMENTAL --
	 * The maximum size of the event tracing file, in bytes
	 */
	UPROPERTY(Config)
	uint64 MaxEventTracingFileSizeBytes;
};
