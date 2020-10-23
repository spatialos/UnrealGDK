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
	 * You must reserve more entity IDs than Actors that spawn on deployment.
	 */
	UPROPERTY(EditAnywhere, config, Category = "Entity Pool", meta = (DisplayName = "Initial Entity ID Reservation Count"))
	uint32 EntityPoolInitialReservationCount;

	/**
	 * The number of remaining entity IDs in the pool that triggers a new batch.
	 */
	UPROPERTY(EditAnywhere, config, Category = "Entity Pool", meta = (DisplayName = "Pool Refresh Threshold"))
	uint32 EntityPoolRefreshThreshold;

	/**
	 * The number of entity IDs in a new batch.
	 */
	UPROPERTY(EditAnywhere, config, Category = "Entity Pool", meta = (DisplayName = "Refresh Count"))
	uint32 EntityPoolRefreshCount;

	/**
	 * From game clients to server-workers.
	 */
	UPROPERTY(EditAnywhere, config, Category = "Heartbeat", meta = (DisplayName = "Heartbeat Interval (Seconds)"))
	float HeartbeatIntervalSeconds;


	UPROPERTY(EditAnywhere, config, Category = "Heartbeat", meta = (DisplayName = "Heartbeat Timeout (Seconds)"))
	float HeartbeatTimeoutSeconds;

	/**
	 * Use when you are running the game server from the editor, or with the editor executable.
	 */
	UPROPERTY(EditAnywhere, config, Category = "Heartbeat", meta = (DisplayName = "Heartbeat Timeout With Editor (Seconds)"))
	float HeartbeatTimeoutWithEditorSeconds;

	/**
	 * Warning: default value 0 means that every Actor replicates. Ignored when using Replication Graph.
	 */
	UPROPERTY(EditAnywhere, config, Category = "Replication", meta = (DisplayName = "Maximum Actors Replicated Per Tick"))
	uint32 ActorReplicationRateLimit;

	/**
	 * Warning: default value 0 means there is no upper limit. Ignored when using Replication Graph.
	 */
	UPROPERTY(EditAnywhere, config, Category = "Replication", meta = (DisplayName = "Maximum Entities Created Per Tick"))
	uint32 EntityCreationRateLimit;

	/**
	 * Relevancy as defined by Unreal. Only use for single server configurations. Ignored when using Replication Graph.
	 */
	UPROPERTY(EditAnywhere, config, Category = "Replication", meta = (DisplayName = "Only Replicate Net Relevant Actors"))
	bool bUseIsActorRelevantForConnection;

	/**
	 * Between server-workers and the SpatialOS Runtime.
	 */
	UPROPERTY(EditAnywhere, config, Category = "Replication", meta = (DisplayName = "SpatialOS Network Update Rate"))
	float OpsUpdateRate;

	/**
	 * The upper limit for Net Cull Distance Squared. Applies to all Actors. 
	 */
	UPROPERTY(EditAnywhere, config, Category = "Replication")
	float MaxNetCullDistanceSquared;

	/** In seconds. Null pointers replace Unresolved UObjects. */
	UPROPERTY(EditAnywhere, config, Category = "Replication",
			  meta = (DisplayName = "Wait Time Before Processing Received RPC With Unresolved Refs"))
	float QueuedIncomingRPCWaitTime;

	/** In seconds. */
	UPROPERTY(EditAnywhere, config, Category = "Replication",
			  meta = (DisplayName = "Wait Time Before Attempting To Reprocess Queued Incoming RPCs"))
	float QueuedIncomingRPCRetryTime;

	UPROPERTY(EditAnywhere, config, Category = "Replication", meta = (DisplayName = "Wait Time Before Retrying Outoing RPC (Seconds)"))
	float QueuedOutgoingRPCRetryTime;

	/** 
	 * Minimum time interval before an Actor’s SpatialOS Position updates. 
	 * Actors must have traveled more than the Position Update Lower Threshold (Centimeters). 
	 */
	UPROPERTY(EditAnywhere, config, Category = "SpatialOS Position Updates", meta = (DisplayName = "Position Update Lower Threshold (Seconds)"))
	float PositionUpdateLowerThresholdSeconds;

	/**
	 * Minimum distance an Actor can move before its SpatialOS Position updates. 
	 * More than the Position Update Lower Threshold (Seconds) must have passed.
	 */
	UPROPERTY(EditAnywhere, config, Category = "SpatialOS Position Updates", meta = (DisplayName = "Position Update Lower Threshold (Centimeters)"))
	float PositionUpdateLowerThresholdCentimeters;

	/**
	 * Maximum time interval before an Actor’s SpatialOS Position updates, regardless of the distance threshold.
	 */
	UPROPERTY(EditAnywhere, config, Category = "SpatialOS Position Updates", meta = (DisplayName = "Position Update Threshold Max (Seconds)"))
	float PositionUpdateThresholdMaxSeconds;

	/* Maximum distance an Actor can move before its SpatialOS Position updates, regardless of the time threshold. */
	UPROPERTY(EditAnywhere, config, Category = "SpatialOS Position Updates", meta = (DisplayName = "Position Update Threshold Max (Centimeters)"))
	float PositionUpdateThresholdMaxCentimeters;

	/** Performance metrics only. */
	UPROPERTY(EditAnywhere, config, Category = "Metrics")
	bool bEnableMetrics;

	/* Clients display server performance metrics. */
	UPROPERTY(EditAnywhere, config, Category = "Metrics")
	bool bEnableMetricsDisplay;

	UPROPERTY(EditAnywhere, config, Category = "Metrics", meta = (DisplayName = "Metrics Report Rate (Seconds)"))
	float MetricsReportRate;

	/** Report server load in seconds per frame. Labeled “Load” in the Inspector. */
	UPROPERTY(EditAnywhere, config, Category = "Metrics")
	bool bUseFrameTimeAsLoad;

	/** Batch entity position updates to be processed on a single frame.*/
	UPROPERTY(config)
	bool bBatchSpatialPositionUpdates;

	/* Includes components. */
	UPROPERTY(EditAnywhere, config, Category = "Schema Generation",
			  meta = (DisplayName = "Maximum Dynamically Attached Subobjects Per Class"))
	uint32 MaxDynamicallyAttachedSubobjectsPerClass;

	UPROPERTY(EditAnywhere, config, Category = "Local Connection")
	FString DefaultReceptionistHost;

private:
	UPROPERTY(EditAnywhere, config, Category = "Cloud Connection")
	bool bPreventClientCloudDeploymentAutoConnect;

public:
	bool GetPreventClientCloudDeploymentAutoConnect() const;

	UPROPERTY(EditAnywhere, Config, Category = "Region settings",
			  meta = (ConfigRestartRequired = true, DisplayName = "Region Where Services Are Located"))
	TEnumAsByte<EServicesRegion::Type> ServicesRegion;

	/** Deprecated!
	Upgraded into the two settings below for local/cloud configurations.
	Ticket for removal UNR-4348 */
	UPROPERTY(config, meta = (DeprecatedProperty, DeprecationMessage = "Use LocalWorkerLogLevel or CloudWorkerLogLevel"))
	TEnumAsByte<ESettingsWorkerLogVerbosity::Type> WorkerLogLevel;

	/** Written to Spatial output, launch log, and cloud deployment logs. */
	UPROPERTY(EditAnywhere, config, Category = "Logging", meta = (DisplayName = "Local Worker Log Level"))
	TEnumAsByte<ESettingsWorkerLogVerbosity::Type> LocalWorkerLogLevel;

	/** Written to Spatial output, launch log, and cloud deployment logs. */
	UPROPERTY(EditAnywhere, config, Category = "Logging", meta = (DisplayName = "Cloud Worker Log Level"))
	TEnumAsByte<ESettingsWorkerLogVerbosity::Type> CloudWorkerLogLevel;

	/* Visualizes authority assignments and helps to debug multi-worker scenarios. */
	UPROPERTY(EditAnywhere, config, Category = "Debug", meta = (MetaClass = "SpatialDebugger"))
	TSubclassOf<ASpatialDebugger> SpatialDebugger;

	/** If false, uses single worker strategy. */
	UPROPERTY(EditAnywhere, config, Category = "Debug", meta = (DisplayName = "Enable Multi-Worker In Editor"))
	bool bEnableMultiWorker;

	/** RPC Ring Buffer is enabled when either the matching setting or load balancing is enabled. */
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

	/** Applies to all RPC types. */
	UPROPERTY(EditAnywhere, Config, Category = "Replication", meta = (DisplayName = "Default RPC Ring Buffer Size"))
	uint32 DefaultRPCRingBufferSize;

	/* Adjust the sizes for specific Ring Buffer RPC Types. */
	UPROPERTY(EditAnywhere, Config, Category = "Replication", meta = (DisplayName = "RPC Ring Buffer Size Map"))
	TMap<ERPCType, uint32> RPCRingBufferSizeMap;

public:
	uint32 GetRPCRingBufferSize(ERPCType RPCType) const;

	float GetSecondsBeforeWarning(const ERPCResult Result) const;

	bool ShouldRPCTypeAllowUnresolvedParameters(const ERPCType Type) const;

	/** Applies to all RPC types. If you change this value, you must regenerate your schema. */
	UPROPERTY(EditAnywhere, Config, Category = "Replication", meta = (DisplayName = "Max RPC Ring Buffer Size"))
	uint32 MaxRPCRingBufferSize;

	UPROPERTY(Config)
	bool bTcpNoDelay;

	UPROPERTY(Config)
	uint32 UdpServerDownstreamUpdateIntervalMS;

	UPROPERTY(Config)
	uint32 UdpClientDownstreamUpdateIntervalMS;

	/** Higher bandwidth/lower latency on RPC calls. */
	UPROPERTY(Config)
	bool bWorkerFlushAfterOutgoingNetworkOp;

	UPROPERTY(Config)
	bool bAsyncLoadNewClassesOnEntityCheckout;

	/* Map that designates the time delay for reporting RCP failures by type. */
	UPROPERTY(EditAnywhere, config, Category = "Queued RPC Warning Timeouts", AdvancedDisplay,
			  meta = (DisplayName = "RPCQueue Warning Timeouts (Seconds)"))
	TMap<ERPCResult, float> RPCQueueWarningTimeouts;

	/* Default time delay for all RCP failure types. */
	UPROPERTY(EditAnywhere, config, Category = "Queued RPC Warning Timeouts", AdvancedDisplay,
			  meta = (DisplayName = "RPCQueue Warning Default Timeout (Seconds)"))
	float RPCQueueWarningDefaultTimeout;

	FORCEINLINE bool IsRunningInChina() const { return ServicesRegion == EServicesRegion::CN; }

	void SetServicesRegion(EServicesRegion::Type NewRegion);

	UPROPERTY(EditAnywhere, Config, Category = "Interest")
	bool bEnableNetCullDistanceInterest;

	/* Use with Net Cull Distance Interest. */
	UPROPERTY(EditAnywhere, Config, Category = "Interest", meta = (EditCondition = "bEnableNetCullDistanceInterest"))
	bool bEnableNetCullDistanceFrequency;

	UPROPERTY(EditAnywhere, Config, Category = "Interest", meta = (EditCondition = "bEnableNetCullDistanceFrequency"))
	float FullFrequencyNetCullDistanceRatio;

	UPROPERTY(EditAnywhere, Config, Category = "Interest", meta = (EditCondition = "bEnableNetCullDistanceFrequency"))
	TArray<FDistanceFrequencyPair> InterestRangeFrequencyPairs;

	/* Only for non-editor builds. */
	UPROPERTY(EditAnywhere, Config, Category = "Connection", meta = (DisplayName = "Use Secure Client Connection In Packaged Builds"))
	bool bUseSecureClientConnection;

	/* Only for non-editor builds. */
	UPROPERTY(EditAnywhere, Config, Category = "Connection", meta = (DisplayName = "Use Secure Server Connection In Packaged Builds"))
	bool bUseSecureServerConnection;

	/* Use for development only. Allows server-workers to capture out-of-range client interest. */
	UPROPERTY(EditAnywhere, Config, Category = "Interest")
	bool bEnableClientQueriesOnServer;

	TOptional<FString> OverrideMultiWorkerSettingsClass;

	UPROPERTY(Config)
	bool bEnableCrossLayerActorSpawning;


	UPROPERTY(EditAnywhere, Config, Category = "Logging", AdvancedDisplay,
		meta = (DisplayName = "Suppress Unresolved Reference RPC Warning"))

	TMap<ERPCType, bool> RPCTypeAllowUnresolvedParamMap;

	/**
	 * In seconds.
	 */
	UPROPERTY(EditAnywhere, Config, Category = "Logging", AdvancedDisplay)
	float StartupLogRate;

	/*
	 * -- EXPERIMENTAL --
	 */
	UPROPERTY(Config)
	bool bEventTracingEnabled;

	/*
	 * -- EXPERIMENTAL --
	 */
	UPROPERTY(Config)
	uint64 MaxEventTracingFileSizeBytes;
};
