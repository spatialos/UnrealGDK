// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "Engine/EngineTypes.h"
#include "Misc/Paths.h"
#include "Utils/ActorGroupManager.h"

#include "SpatialGDKSettings.generated.h"

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
	UPROPERTY(EditAnywhere, config, Category = "Entity Pool", meta = (ConfigRestartRequired = false, DisplayName = "Initial Entity ID Reservation Count"))
	uint32 EntityPoolInitialReservationCount;

	/** 
	 * Specifies when the SpatialOS Runtime should reserve a new batch of entity IDs: the value is the number of un-used entity 
	 * IDs left in the entity pool which triggers the SpatialOS Runtime to reserve new entity IDs
	*/
	UPROPERTY(EditAnywhere, config, Category = "Entity Pool", meta = (ConfigRestartRequired = false, DisplayName = "Pool Refresh Threshold"))
	uint32 EntityPoolRefreshThreshold;

	/** 
	* Specifies the number of new entity IDs the SpatialOS Runtime reserves when `Pool refresh threshold` triggers a new batch.
	*/
	UPROPERTY(EditAnywhere, config, Category = "Entity Pool", meta = (ConfigRestartRequired = false, DisplayName = "Refresh Count"))
	uint32 EntityPoolRefreshCount;

	/** Specifies the amount of time, in seconds, between heartbeat events sent from a game client to notify the server-worker instances that it's connected. */
	UPROPERTY(EditAnywhere, config, Category = "Heartbeat", meta = (ConfigRestartRequired = false, DisplayName = "Heartbeat Interval (seconds)"))
	float HeartbeatIntervalSeconds;

	/** 
	* Specifies the maximum amount of time, in seconds, that the server-worker instances wait for a game client to send heartbeat events. 
	* (If the timeout expires, the game client has disconnected.) 
	*/
	UPROPERTY(EditAnywhere, config, Category = "Heartbeat", meta = (ConfigRestartRequired = false, DisplayName = "Heartbeat Timeout (seconds)"))
	float HeartbeatTimeoutSeconds;

	/**
	 * Specifies the maximum number of Actors replicated per tick.
	 * Default: `0` per tick  (no limit)
	 * (If you set the value to ` 0`, the SpatialOS Runtime replicates every Actor per tick; this forms a large SpatialOS  world, affecting the performance of both game clients and server-worker instances.)
	 * You can use the `stat Spatial` flag when you run project builds to find the number of calls to `ReplicateActor`, and then use this number for reference.
	 */
	UPROPERTY(EditAnywhere, config, Category = "Replication", meta = (ConfigRestartRequired = false, DisplayName = "Maximum Actors replicated per tick"))
	uint32 ActorReplicationRateLimit;

	/** 
	* Specifies the maximum number of entities created by the SpatialOS Runtime per tick. 
	* (The SpatialOS Runtime handles entity creation separately from Actor replication to ensure it can handle entity creation requests under load.)
	* Note: if you set the value to 0, there is no limit to the number of entities created per tick. However, too many entities created at the same time might overload the SpatialOS Runtime, which can negatively affect your game.
	* Default: `0` per tick  (no limit)
	*/
	UPROPERTY(EditAnywhere, config, Category = "Replication", meta = (ConfigRestartRequired = false, DisplayName = "Maximum entities created per tick"))
	uint32 EntityCreationRateLimit;

	/**
	* Specifies the rate, in number of times per second, at which server-worker instance updates are sent to and received from the SpatialOS Runtime.
	* Default:1000/s
	*/
	UPROPERTY(EditAnywhere, config, Category = "Replication", meta = (ConfigRestartRequired = false, DisplayName = "SpatialOS Network Update Rate"))
	float OpsUpdateRate;

	/** Replicate handover properties between servers, required for zoned worker deployments.*/
	UPROPERTY(EditAnywhere, config, Category = "Replication", meta = (ConfigRestartRequired = false))
	bool bEnableHandover;

	/** Maximum NetCullDistanceSquared value used in Spatial networking. Set to 0.0 to disable. This is temporary and will be removed when the runtime issue is resolved.*/
	UPROPERTY(EditAnywhere, config, Category = "Replication", meta = (ConfigRestartRequired = false))
	float MaxNetCullDistanceSquared;

	/** Query Based Interest is required for level streaming and the AlwaysInterested UPROPERTY specifier to be supported when using spatial networking, however comes at a performance cost for larger-scale projects.*/
	UPROPERTY(config, meta = (ConfigRestartRequired = false))
	bool bUsingQBI;

	/** Frequency for updating an Actor's SpatialOS Position. Updating position should have a low update rate since it is expensive.*/
	UPROPERTY(EditAnywhere, config, Category = "SpatialOS Position Updates", meta = (ConfigRestartRequired = false))
	float PositionUpdateFrequency;

	/** Threshold an Actor needs to move, in centimeters, before its SpatialOS Position is updated.*/
	UPROPERTY(EditAnywhere, config, Category = "SpatialOS Position Updates", meta = (ConfigRestartRequired = false))
	float PositionDistanceThreshold;

	/** Metrics about client and server performance can be reported to SpatialOS to monitor a deployments health.*/
	UPROPERTY(EditAnywhere, config, Category = "Metrics", meta = (ConfigRestartRequired = false))
	bool bEnableMetrics;

	/** Display server metrics on clients.*/
	UPROPERTY(EditAnywhere, config, Category = "Metrics", meta = (ConfigRestartRequired = false))
	bool bEnableMetricsDisplay;

	/** Frequency that metrics are reported to SpatialOS.*/
	UPROPERTY(EditAnywhere, config, Category = "Metrics", meta = (ConfigRestartRequired = false), DisplayName = "Metrics Report Rate (seconds)")
	float MetricsReportRate;

	/** 
	* By default the SpatialOS Runtime reports server-worker instance’s load in frames per second (FPS). 
	* Select this to switch so it reports as seconds per frame. 
	* This value is visible as 'Load' in the Inspector, next to each worker.
	*/
	UPROPERTY(EditAnywhere, config, Category = "Metrics", meta = (ConfigRestartRequired = false))
	bool bUseFrameTimeAsLoad;

	/** Include an order index with reliable RPCs and warn if they are executed out of order.*/
	UPROPERTY(config, meta = (ConfigRestartRequired = false))
	bool bCheckRPCOrder;

	/** Batch entity position updates to be processed on a single frame.*/
	UPROPERTY(config, meta = (ConfigRestartRequired = false))
	bool bBatchSpatialPositionUpdates;

	/** Maximum number of ActorComponents/Subobjects of the same class that can be attached to an Actor.*/
	UPROPERTY(EditAnywhere, config, Category = "Schema Generation", meta = (ConfigRestartRequired = false), DisplayName = "Maximum Dynamically Attached Subobjects Per Class")
	uint32 MaxDynamicallyAttachedSubobjectsPerClass;

	/** EXPERIMENTAL - This is a stop-gap until we can better define server interest on system entities.
	Disabling this is not supported in any type of multi-server environment*/
	UPROPERTY(config, meta = (ConfigRestartRequired = false))
	bool bEnableServerQBI;

	/** Pack RPCs sent during the same frame into a single update. */
	UPROPERTY(config, meta = (ConfigRestartRequired = false))
	bool bPackRPCs;

	/** The receptionist host to use if no 'receptionistHost' argument is passed to the command line. */
	UPROPERTY(EditAnywhere, config, Category = "Local Connection", meta = (ConfigRestartRequired = false))
	FString DefaultReceptionistHost;

	/** If the Development Authentication Flow is used, the client will try to connect to the cloud rather than local deployment. */
	UPROPERTY(EditAnywhere, config, Category = "Cloud Connection", meta = (ConfigRestartRequired = false))
	bool bUseDevelopmentAuthenticationFlow;

	/** The token created using 'spatial project auth dev-auth-token' */
	UPROPERTY(EditAnywhere, config, Category = "Cloud Connection", meta = (ConfigRestartRequired = false))
	FString DevelopmentAuthenticationToken;

	/** The deployment to connect to when using the Development Authentication Flow. If left empty, it uses the first available one (order not guaranteed when there are multiple items). The deployment needs to be tagged with 'dev_login'. */
	UPROPERTY(EditAnywhere, config, Category = "Cloud Connection", meta = (ConfigRestartRequired = false))
	FString DevelopmentDeploymentToConnect;

	/** Single server worker type to launch when offloading is disabled, fallback server worker type when offloading is enabled (owns all actor classes by default). */
	UPROPERTY(EditAnywhere, Config, Category = "Offloading")
	FWorkerType DefaultWorkerType;

	/** Enable running different server worker types to split the simulation by Actor Groups. */
	UPROPERTY(EditAnywhere, Config, Category = "Offloading")
	bool bEnableOffloading;

	/** Actor Group configuration. */
	UPROPERTY(EditAnywhere, Config, Category = "Offloading", meta = (EditCondition = "bEnableOffloading"))
	TMap<FName, FActorGroupInfo> ActorGroups;

	/** Available server worker types. */
	UPROPERTY(Config)
	TSet<FName> ServerWorkerTypes;
};
