// Copyright (c) Improbable Worlds Ltd, All Rights Reserved
#pragma once

#include "CoreMinimal.h"
#include "Engine/EngineTypes.h"
#include "Misc/Paths.h"

#include "SpatialConstants.h"

#include "SpatialGDKEditorSettings.generated.h"

USTRUCT()
struct FWorldLaunchSection
{
	GENERATED_BODY()

	FWorldLaunchSection()
		: Dimensions(2000, 2000)
		, ChunkEdgeLengthMeters(50)
		, StreamingQueryIntervalSeconds(4)
		, SnapshotWritePeriodSeconds(0)
	{
		LegacyFlags.Add(TEXT("bridge_qos_max_timeout"), TEXT("0"));
		LegacyFlags.Add(TEXT("bridge_soft_handover_enabled"), TEXT("false"));
		LegacyFlags.Add(TEXT("enable_chunk_interest"), TEXT("false"));
	}

	/** The size of the simulation, in meters, for the auto-generated launch configuration file. */
	UPROPERTY(Category = "SpatialGDK", EditAnywhere, config, meta = (ConfigRestartRequired = false, DisplayName = "Simulation dimensions in meters"))
	FIntPoint Dimensions;

	/** The size of the grid squares that the world is divided into, in “world units” (an arbitrary unit that worker instances can interpret as they choose). */
	UPROPERTY(Category = "SpatialGDK", EditAnywhere, config, meta = (ConfigRestartRequired = false, DisplayName = "Chunk edge length in meters"))
	int32 ChunkEdgeLengthMeters;

	/** The time in seconds between streaming query updates. */
	UPROPERTY(Category = "SpatialGDK", EditAnywhere, config, meta = (ConfigRestartRequired = false, DisplayName = "Streaming query interval in seconds"))
	int32 StreamingQueryIntervalSeconds;

	/** The frequency in seconds to write snapshots of the simulated world. */
	UPROPERTY(Category = "SpatialGDK", EditAnywhere, config, meta = (ConfigRestartRequired = false, DisplayName = "Snapshot write period in seconds"))
	int32 SnapshotWritePeriodSeconds;

	/** Legacy non-worker flag configurations. */
	UPROPERTY(Category = "SpatialGDK", EditAnywhere, config, meta = (ConfigRestartRequired = false))
	TMap<FString, FString> LegacyFlags;

	/** Legacy JVM configurations. */
	UPROPERTY(Category = "SpatialGDK", EditAnywhere, config, meta = (ConfigRestartRequired = false, DisplayName = "Legacy Java parameters"))
	TMap<FString, FString> LegacyJavaParams;
};

USTRUCT()
struct FWorkerPermissionsSection
{
	GENERATED_BODY()

	FWorkerPermissionsSection()
		: bAllPermissions(true)
		, bAllowEntityCreation(true)
		, bAllowEntityDeletion(true)
		, bAllowEntityQuery(true)
		, Components()
	{
	}

	/** Gives all permissions to a worker instance. */
	UPROPERTY(Category = "SpatialGDK", EditAnywhere, config, meta = (ConfigRestartRequired = false, DisplayName = "All"))
	bool bAllPermissions;

	/** Enables a worker instance to create new entities. */
	UPROPERTY(Category = "SpatialGDK", EditAnywhere, config, meta = (EditCondition = "!bAllPermissions", ConfigRestartRequired = false, DisplayName = "Allow entity creation"))
	bool bAllowEntityCreation;

	/** Enables a worker instance to delete entities. */
	UPROPERTY(Category = "SpatialGDK", EditAnywhere, config, meta = (EditCondition = "!bAllPermissions", ConfigRestartRequired = false, DisplayName = "Allow entity deletion"))
	bool bAllowEntityDeletion;

	/** Controls which components can be returned from entity queries that the worker instance performs. If an entity query specifies other components to be returned, the query will fail. */
	UPROPERTY(Category = "SpatialGDK", EditAnywhere, config, meta = (EditCondition = "!bAllPermissions", ConfigRestartRequired = false, DisplayName = "Allow entity query"))
	bool bAllowEntityQuery;

	/** Specifies which components can be returned in the query result. */
	UPROPERTY(Category = "SpatialGDK", EditAnywhere, config, meta = (EditCondition = "!bAllPermissions", ConfigRestartRequired = false, DisplayName = "Component queries"))
	TArray<FString> Components;
};

USTRUCT()
struct FLoginRateLimitSection
{
	GENERATED_BODY()

	FLoginRateLimitSection()
		: Duration()
		, RequestsPerDuration(0)
	{
	}

	/** The duration for which worker connection requests will be limited. */
	UPROPERTY(Category = "SpatialGDK", EditAnywhere, config, meta = (ConfigRestartRequired = false))
	FString Duration;

	/** The connection request limit for the duration. */
	UPROPERTY(Category = "SpatialGDK", EditAnywhere, config, meta = (ConfigRestartRequired = false, ClampMin = "1", UIMin = "1"))
	int32 RequestsPerDuration;
};

USTRUCT()
struct FWorkerTypeLaunchSection
{
	GENERATED_BODY()

	FWorkerTypeLaunchSection()
		: WorkerTypeName()
		, WorkerPermissions()
		, MaxConnectionCapacityLimit(0)
		, bLoginRateLimitEnabled(false)
		, LoginRateLimit()
		, Columns(1)
		, Rows(1)
		, bManualWorkerConnectionOnly(true)
	{
	}

	/** The name of the worker type, defined in the filename of its spatialos.<worker_type>.worker.json file. */
	UPROPERTY(Category = "SpatialGDK", EditAnywhere, config, meta = (ConfigRestartRequired = false))
	FString WorkerTypeName;

	/** Defines the worker instance's permissions. */
	UPROPERTY(Category = "SpatialGDK", EditAnywhere, config, meta = (ConfigRestartRequired = false))
	FWorkerPermissionsSection WorkerPermissions;

	/** Defines the maximum number of worker instances that can connect. */
	UPROPERTY(Category = "SpatialGDK", EditAnywhere, config, meta = (ConfigRestartRequired = false, DisplayName = "Max connection capacity limit (0 = unlimited capacity)", ClampMin = "0", UIMin = "0"))
	int32 MaxConnectionCapacityLimit;

	/** Enable connection rate limiting. */
	UPROPERTY(Category = "SpatialGDK", EditAnywhere, config, meta = (ConfigRestartRequired = false, DisplayName = "Login rate limit enabled"))
	bool bLoginRateLimitEnabled;

	/** Login rate limiting configuration. */
	UPROPERTY(Category = "SpatialGDK", EditAnywhere, config, meta = (EditCondition = "bLoginRateLimitEnabled", ConfigRestartRequired = false))
	FLoginRateLimitSection LoginRateLimit;

	/** Number of columns in the rectangle grid load balancing config. */
	UPROPERTY(Category = "SpatialGDK", EditAnywhere, config, meta = (ConfigRestartRequired = false, DisplayName = "Rectangle grid column count", ClampMin = "1", UIMin = "1"))
	int32 Columns;

	/** Number of rows in the rectangle grid load balancing config. */
	UPROPERTY(Category = "SpatialGDK", EditAnywhere, config, meta = (ConfigRestartRequired = false, DisplayName = "Rectangle grid row count", ClampMin = "1", UIMin = "1"))
	int32 Rows;

	/** Flags defined for a worker instance. */
	UPROPERTY(Category = "SpatialGDK", EditAnywhere, config, meta = (ConfigRestartRequired = false, DisplayName = "Flags"))
	TMap<FString, FString> Flags;

	/** Determines if the worker instance is launched manually or by SpatialOS. */
	UPROPERTY(Category = "SpatialGDK", EditAnywhere, config, meta = (ConfigRestartRequired = false, DisplayName = "Manual worker connection only"))
	bool bManualWorkerConnectionOnly;
};

USTRUCT()
struct FSpatialLaunchConfigDescription
{
	GENERATED_BODY()

	FSpatialLaunchConfigDescription()
		: Template(TEXT("small"))
		, World()
	{
		FWorkerTypeLaunchSection UnrealWorkerDefaultSetting;
		UnrealWorkerDefaultSetting.WorkerTypeName = SpatialConstants::ServerWorkerType;
		UnrealWorkerDefaultSetting.Rows = 1;
		UnrealWorkerDefaultSetting.Columns = 1;
		UnrealWorkerDefaultSetting.bManualWorkerConnectionOnly = true;

		Workers.Add(UnrealWorkerDefaultSetting);
	}

	/** Deployment template. */
	UPROPERTY(Category = "SpatialGDK", EditAnywhere, config, meta = (ConfigRestartRequired = false))
	FString Template;

	/** Configuration for the simulated world. */
	UPROPERTY(Category = "SpatialGDK", EditAnywhere, config, meta = (ConfigRestartRequired = false))
	FWorldLaunchSection World;

	/** Worker-specific configuration parameters. */
	UPROPERTY(Category = "SpatialGDK", EditAnywhere, config, meta = (ConfigRestartRequired = false))
	TArray<FWorkerTypeLaunchSection> Workers;
};

UCLASS(config = SpatialGDKEditorSettings, defaultconfig)
class SPATIALGDKEDITOR_API USpatialGDKEditorSettings : public UObject
{
	GENERATED_BODY()

public:
	USpatialGDKEditorSettings(const FObjectInitializer& ObjectInitializer);

	virtual void PostEditChangeProperty(struct FPropertyChangedEvent& PropertyChangedEvent) override;
	virtual void PostInitProperties() override;

private:
	/** Path to the directory containing the SpatialOS-related files. */
	UPROPERTY(EditAnywhere, config, Category = "General", meta = (ConfigRestartRequired = false, DisplayName = "SpatialOS directory"))
	FDirectoryPath SpatialOSDirectory;

public:
	/** If checked, all dynamically spawned entities will be deleted when server workers disconnect. */
	UPROPERTY(EditAnywhere, config, Category = "Play in editor settings", meta = (ConfigRestartRequired = false, DisplayName = "Delete dynamically spawned entities"))
	bool bDeleteDynamicEntities;

	/** If checked, a launch configuration will be generated by default when launching spatial through the toolbar. */
	UPROPERTY(EditAnywhere, config, Category = "Launch", meta = (ConfigRestartRequired = false, DisplayName = "Generate default launch config"))
	bool bGenerateDefaultLaunchConfig;

private:
	/** Launch configuration file used for `spatial local launch`. */
	UPROPERTY(EditAnywhere, config, Category = "Launch", meta = (EditCondition = "!bGenerateDefaultLaunchConfig", ConfigRestartRequired = false, DisplayName = "Launch configuration"))
	FFilePath SpatialOSLaunchConfig;

public:
	/** Stop `spatial local launch` when shutting down editor. */
	UPROPERTY(EditAnywhere, config, Category = "Launch", meta = (ConfigRestartRequired = false, DisplayName = "Stop on exit"))
	bool bStopSpatialOnExit;

private:
	/** Path to your SpatialOS snapshot. */
	UPROPERTY(EditAnywhere, config, Category = "Snapshots", meta = (ConfigRestartRequired = false, DisplayName = "Snapshot path"))
	FDirectoryPath SpatialOSSnapshotPath;

	/** Name of your SpatialOS snapshot file. */
	UPROPERTY(EditAnywhere, config, Category = "Snapshots", meta = (ConfigRestartRequired = false, DisplayName = "Snapshot file name"))
	FString SpatialOSSnapshotFile;

	/** If checked, the GDK creates a launch configuration file by default when you launch a local deployment through the toolbar. */
	UPROPERTY(EditAnywhere, config, Category = "Schema", meta = (ConfigRestartRequired = false, DisplayName = "Output path for the generated schemas"))
	FDirectoryPath GeneratedSchemaOutputFolder;

	/** Command line flags passed in to `spatial local launch`.*/
	UPROPERTY(EditAnywhere, config, Category = "Launch", meta = (ConfigRestartRequired = false, DisplayName = "Command line flags for local launch"))
	TArray<FString> SpatialOSCommandLineLaunchFlags;

public:
	/** Auto-generated launch configuration file description. */
	UPROPERTY(EditAnywhere, config, Category = "Launch", meta = (EditCondition = "bGenerateDefaultLaunchConfig", ConfigRestartRequired = false, DisplayName = "Launch configuration file description"))
	FSpatialLaunchConfigDescription LaunchConfigDesc;

	/** If checked, placeholder entities are added to the snapshot on generation. */
	UPROPERTY(EditAnywhere, config, Category = "Snapshots", meta = (ConfigRestartRequired = false, DisplayName = "Generate placeholder entities in snapshot"))
	bool bGeneratePlaceholderEntitiesInSnapshot;

	FORCEINLINE FString GetSpatialOSDirectory() const
	{
		return SpatialOSDirectory.Path.IsEmpty()
			? FPaths::ConvertRelativePathToFull(FPaths::Combine(FPaths::ProjectDir(), TEXT("/../spatial/")))
			: SpatialOSDirectory.Path;
	}

	FORCEINLINE FString GetSpatialOSLaunchConfig() const
	{
		return SpatialOSLaunchConfig.FilePath.IsEmpty()
			? FPaths::ConvertRelativePathToFull(FPaths::Combine(FPaths::ProjectDir(), TEXT("/../spatial/default_launch.json")))
			: SpatialOSLaunchConfig.FilePath;
	}

	FORCEINLINE FString GetSpatialOSSnapshotFile() const
	{
		return SpatialOSSnapshotFile.IsEmpty()
			? FString(TEXT("default.snapshot"))
			: SpatialOSSnapshotFile;
	}

	FORCEINLINE FString GetSpatialOSSnapshotFolderPath() const
	{
		return SpatialOSSnapshotPath.Path.IsEmpty()
			? FPaths::ConvertRelativePathToFull(FPaths::Combine(GetSpatialOSDirectory(), TEXT("../spatial/snapshots/")))
			: SpatialOSSnapshotPath.Path;
	}

	FORCEINLINE FString GetGeneratedSchemaOutputFolder() const
	{
		return GeneratedSchemaOutputFolder.Path.IsEmpty()
			? FPaths::ConvertRelativePathToFull(FPaths::Combine(GetSpatialOSDirectory(), FString(TEXT("schema/unreal/generated/"))))
			: GeneratedSchemaOutputFolder.Path;
	}

	FORCEINLINE FString GetSpatialOSCommandLineLaunchFlags() const
	{
		FString CommandLineLaunchFlags = TEXT("");

		for (FString Flag : SpatialOSCommandLineLaunchFlags)
		{
			Flag = Flag.StartsWith(TEXT("--")) ? Flag : TEXT("--") + Flag;
			CommandLineLaunchFlags += Flag + TEXT(" ");
		}

		return CommandLineLaunchFlags;
	}
};
