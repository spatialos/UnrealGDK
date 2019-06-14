// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "Engine/EngineTypes.h"
#include "Misc/Paths.h"

#include "SpatialGDKSettings.generated.h"

UCLASS(config = SpatialGDKSettings, defaultconfig)
class SPATIALGDK_API USpatialGDKSettings : public UObject
{
	GENERATED_BODY()

public:
	USpatialGDKSettings(const FObjectInitializer& ObjectInitializer);

#if WITH_EDITOR
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif

	virtual void PostInitProperties() override;

	/** The number of entity IDs to be reserved when the entity pool is first created */
	UPROPERTY(EditAnywhere, config, Category = "Entity Pool", meta = (ConfigRestartRequired = false, DisplayName = "Initial Entity ID Reservation Count"))
	uint32 EntityPoolInitialReservationCount;

	/** The minimum number of entity IDs available in the pool before a new batch is reserved */
	UPROPERTY(EditAnywhere, config, Category = "Entity Pool", meta = (ConfigRestartRequired = false, DisplayName = "Pool Refresh Minimum Threshold"))
	uint32 EntityPoolRefreshThreshold;

	/** The number of entity IDs reserved when the minimum threshold is reached */
	UPROPERTY(EditAnywhere, config, Category = "Entity Pool", meta = (ConfigRestartRequired = false, DisplayName = "Refresh Count"))
	uint32 EntityPoolRefreshCount;

	/** Time between heartbeat events sent from clients to notify the servers they are still connected. */
	UPROPERTY(EditAnywhere, config, Category = "Heartbeat", meta = (ConfigRestartRequired = false, DisplayName = "Heartbeat Interval (seconds)"))
	float HeartbeatIntervalSeconds;

	/** Time that should pass since the last heartbeat event received to decide a client has disconnected. */
	UPROPERTY(EditAnywhere, config, Category = "Heartbeat", meta = (ConfigRestartRequired = false, DisplayName = "Heartbeat Timeout (seconds)"))
	float HeartbeatTimeoutSeconds;

	/**
	 * Limit the number of actors which are replicated per tick to the number specified.
	 * This acts as a hard limit to the number of actors per frame but nothing else. It's recommended to set this value to around 100~ (experimentation recommended).
	 * If set to 0, SpatialOS will replicate every actor per frame (unbounded) and so large worlds will experience slowdown server-side and client-side.
	 * Use `stat SpatialNet` in editor builds to find the number of calls to 'ReplicateActor' and use this to inform the rate limit setting.
	 */
	UPROPERTY(EditAnywhere, config, Category = "Replication", meta = (ConfigRestartRequired = false, DisplayName = "Actor Replication Rate Limit"))
	uint32 ActorReplicationRateLimit;

	/** Limits the number of entities which can be created in a game tick. Entity creation is handled seperately to actor replication to ensure creation requests are always handled when under load. **/
	UPROPERTY(EditAnywhere, config, Category = "Replication", meta = (ConfigRestartRequired = false, DisplayName = "Entity Creation Rate Limit"))
	uint32 EntityCreationRateLimit;

	/** Rate at which updates are sent to SpatialOS and processed from SpatialOS.*/
	UPROPERTY(EditAnywhere, config, Category = "Replication", meta = (ConfigRestartRequired = false, DisplayName = "SpatialOS Network Update Rate"))
	float OpsUpdateRate;

	/** Replicate handover properties between servers, required for zoned worker deployments.*/
	UPROPERTY(EditAnywhere, config, Category = "Replication", meta = (ConfigRestartRequired = false))
	bool bEnableHandover;

	/** Query Based Interest is required for level streaming and the AlwaysInterested UPROPERTY specifier to be supported when using spatial networking, however comes at a performance cost for larger-scale projects.*/
	UPROPERTY(config, meta = (ConfigRestartRequired = false))
	bool bUsingQBI;

	/** Frequency for updating an Actor's SpatialOS Position. Updating position should have a low update rate since it is expensive.*/
	UPROPERTY(EditAnywhere, config, Category = "SpatialOS Position Updates", meta = (ConfigRestartRequired = false))
	float PositionUpdateFrequency;

	/** Threshold an Actor needs to move before its SpatialOS Position is updated.*/
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

	/** Change 'Load' value in inspector to represent worker Frame Time instead of a fraction of target FPS.*/
	UPROPERTY(EditAnywhere, config, Category = "Metrics", meta = (ConfigRestartRequired = false))
	bool bUseFrameTimeAsLoad;

	/** Include an order index with reliable RPCs and warn if they are executed out of order.*/
	UPROPERTY(config, meta = (ConfigRestartRequired = false))
	bool bCheckRPCOrder;

	/** Batch entity position updates to be processed on a single frame.*/
	UPROPERTY(config, meta = (ConfigRestartRequired = false))
	bool bBatchSpatialPositionUpdates;

	/** EXPERIMENTAL - This is a stop-gap until we can better define server interest on system entities.
	Disabling this is not supported in any type of multi-server environment*/
	UPROPERTY(config, meta = (ConfigRestartRequired = false))
	bool bEnableServerQBI;
};

