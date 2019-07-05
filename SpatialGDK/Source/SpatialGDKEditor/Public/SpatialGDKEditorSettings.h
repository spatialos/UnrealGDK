// Copyright (c) Improbable Worlds Ltd, All Rights Reserved
#pragma once

#include "CoreMinimal.h"
#include "Engine/EngineTypes.h"
#include "Misc/Paths.h"
#include "SpatialConstants.h"
#include "UObject/Package.h"
#include "SpatialGDKServicesModule.h"

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
		, NumEditorInstances(1)
		, bManualWorkerConnectionOnly(true)
	{
	}

	/** The name of the worker type, defined in the filename of its spatialos.<worker_type>.worker.json file. */
	UPROPERTY(Category = "SpatialGDK", EditAnywhere, config, meta = (ConfigRestartRequired = false))
	FName WorkerTypeName;

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

	/** Number of instances to launch when playing in editor. */
	UPROPERTY(Category = "SpatialGDK", EditAnywhere, config, meta = (ConfigRestartRequired = false, DisplayName = "Instances to launch in editor", ClampMin = "0", UIMin = "0"))
	int32 NumEditorInstances;

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
		: Template(TEXT("w2_r0500_e5"))
		, World()
	{
		FWorkerTypeLaunchSection UnrealWorkerDefaultSetting;
		UnrealWorkerDefaultSetting.WorkerTypeName = SpatialConstants::DefaultServerWorkerType;
		UnrealWorkerDefaultSetting.Rows = 1;
		UnrealWorkerDefaultSetting.Columns = 1;
		UnrealWorkerDefaultSetting.bManualWorkerConnectionOnly = true;

		ServerWorkers.Add(UnrealWorkerDefaultSetting);
	}

	/** Deployment template. */
	UPROPERTY(Category = "SpatialGDK", EditAnywhere, config, meta = (ConfigRestartRequired = false))
	FString Template;

	/** Configuration for the simulated world. */
	UPROPERTY(Category = "SpatialGDK", EditAnywhere, config, meta = (ConfigRestartRequired = false))
	FWorldLaunchSection World;

	/** Worker-specific configuration parameters. */
	UPROPERTY(Category = "SpatialGDK", EditAnywhere, config, meta = (ConfigRestartRequired = false))
	TArray<FWorkerTypeLaunchSection> ServerWorkers;
};

/**
* Enumerates available Region Codes
*/
UENUM()
namespace ERegionCode
{
	enum Type
	{
		US = 1,
		EU,
		AP,
	};
}

UCLASS(config = SpatialGDKEditorSettings, defaultconfig)
class SPATIALGDKEDITOR_API USpatialGDKEditorSettings : public UObject
{
	GENERATED_BODY()

public:
	USpatialGDKEditorSettings(const FObjectInitializer& ObjectInitializer);

	virtual void PostEditChangeProperty(struct FPropertyChangedEvent& PropertyChangedEvent) override;
	virtual void PostInitProperties() override;

private:

	/** Set WorkerTypes in runtime settings. */
	void SetRuntimeWorkerTypes();

	/** Set WorkerTypesToLaunch in level editor play settings. */
	void SetLevelEditorPlaySettingsWorkerTypes();

public:
	/** If checked, show the Spatial service button on the GDK toolbar which can be used to turn the Spatial service on and off. */
	UPROPERTY(EditAnywhere, config, Category = "General", meta = (ConfigRestartRequired = false, DisplayName = "Show Spatial service button"))
	bool bShowSpatialServiceButton;

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

	/** Command line flags passed in to `spatial local launch`.*/
	UPROPERTY(EditAnywhere, config, Category = "Launch", meta = (ConfigRestartRequired = false, DisplayName = "Command line flags for local launch"))
	TArray<FString> SpatialOSCommandLineLaunchFlags;

private:
	UPROPERTY(EditAnywhere, config, Category = "Cloud", meta = (ConfigRestartRequired = false, DisplayName = "SpatialOS project"))
		FString ProjectName;

	UPROPERTY(EditAnywhere, config, Category = "Cloud", meta = (ConfigRestartRequired = false, DisplayName = "Assembly name"))
		FString AssemblyName;

	UPROPERTY(EditAnywhere, config, Category = "Cloud", meta = (ConfigRestartRequired = false, DisplayName = "Deployment name"))
		FString PrimaryDeploymentName;

	UPROPERTY(EditAnywhere, config, Category = "Cloud", meta = (ConfigRestartRequired = false, DisplayName = "Cloud launch configuration path"))
		FFilePath PrimaryLaunchConfigPath;

	UPROPERTY(EditAnywhere, config, Category = "Cloud", meta = (ConfigRestartRequired = false, DisplayName = "Snapshot path"))
		FFilePath SnapshotPath;

	UPROPERTY(EditAnywhere, config, Category = "Cloud", meta = (ConfigRestartRequired = false, DisplayName = "Region"))
		TEnumAsByte<ERegionCode::Type> PrimaryDeploymentRegionCode;

	UPROPERTY(EditAnywhere, config, Category = "Simulated Players", meta = (EditCondition = "bSimulatedPlayersIsEnabled", ConfigRestartRequired = false, DisplayName = "Region"))
		TEnumAsByte<ERegionCode::Type> SimulatedPlayerDeploymentRegionCode;

	UPROPERTY(EditAnywhere, config, Category = "Simulated Players", meta = (ConfigRestartRequired = false, DisplayName = "Include simulated players"))
		bool bSimulatedPlayersIsEnabled;

	UPROPERTY(EditAnywhere, config, Category = "Simulated Players", meta = (EditCondition = "bSimulatedPlayersIsEnabled", ConfigRestartRequired = false, DisplayName = "Deployment mame"))
		FString SimulatedPlayerDeploymentName;

	const FString SimulatedPlayerLaunchConfigPath;

	UPROPERTY(EditAnywhere, config, Category = "Simulated Players", meta = (EditCondition = "bSimulatedPlayersIsEnabled", ConfigRestartRequired = false, DisplayName = "Number of simulated players"))
		uint32 NumberOfSimulatedPlayers;
	
	static bool IsAssemblyNameValid(const FString& Name);
	static bool IsProjectNameValid(const FString& Name);
	static bool IsDeploymentNameValid(const FString& Name);
	static bool IsRegionCodeValid(const ERegionCode::Type RegionCode);

public:
	/** Auto-generated launch configuration file description. */
	UPROPERTY(EditAnywhere, config, Category = "Launch", meta = (EditCondition = "bGenerateDefaultLaunchConfig", ConfigRestartRequired = false, DisplayName = "Launch configuration file description"))
	FSpatialLaunchConfigDescription LaunchConfigDesc;

	FORCEINLINE FString GetGDKPluginDirectory() const
	{
		// Get the correct plugin directory.
		FString PluginDir = FPaths::ConvertRelativePathToFull(FPaths::Combine(FPaths::ProjectPluginsDir(), TEXT("UnrealGDK")));

		if (!FPaths::DirectoryExists(PluginDir))
		{
			// If the Project Plugin doesn't exist then use the Engine Plugin.
			PluginDir = FPaths::ConvertRelativePathToFull(FPaths::Combine(FPaths::EnginePluginsDir(), TEXT("UnrealGDK")));
		}

		return PluginDir;
	}

	FORCEINLINE FString GetSpatialOSLaunchConfig() const
	{
		return SpatialOSLaunchConfig.FilePath.IsEmpty()
			? FPaths::ConvertRelativePathToFull(FPaths::Combine(FPaths::ProjectDir(), TEXT("/../spatial/default_launch.json")))
			: SpatialOSLaunchConfig.FilePath;
	}

	FORCEINLINE FString GetSpatialOSConfig()
	{
		return SpatialOSLaunchConfig.FilePath.IsEmpty()
			? FPaths::ConvertRelativePathToFull(FPaths::Combine(FPaths::ProjectDir(), TEXT("/../spatial/spatialos.json")))
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
			? FPaths::ConvertRelativePathToFull(FPaths::Combine(FSpatialGDKServicesModule::GetSpatialOSDirectory(), TEXT("snapshots")))
			: SpatialOSSnapshotPath.Path;
	}

	FORCEINLINE FString GetGeneratedSchemaOutputFolder() const
	{
		return FPaths::ConvertRelativePathToFull(FPaths::Combine(FSpatialGDKServicesModule::GetSpatialOSDirectory(), FString(TEXT("schema/unreal/generated/"))));
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

	FString GetProjectNameFromSpatial() const;

	void SetPrimaryDeploymentName(const FString& Name);
	FORCEINLINE FString GetPrimaryDeploymentName() const
	{
		return PrimaryDeploymentName;
	}

	void SetAssemblyName(const FString& Name);
	FORCEINLINE FString GetAssemblyName() const
	{
		return AssemblyName;
	}

	void SetProjectName(const FString& Name);
	FORCEINLINE FString GetProjectName() const
	{
		return ProjectName;
	}

	void SetPrimaryLaunchConfigPath(const FString& Path);
	FORCEINLINE FString GetPrimaryLanchConfigPath() const
	{
		const USpatialGDKEditorSettings* SpatialEditorSettings = GetDefault<USpatialGDKEditorSettings>();
		return PrimaryLaunchConfigPath.FilePath.IsEmpty()
			? SpatialEditorSettings->GetSpatialOSLaunchConfig()
			: PrimaryLaunchConfigPath.FilePath;
	}

	void SetSnapshotPath(const FString& Path);
	FORCEINLINE FString GetSnapshotPath() const
	{
		const USpatialGDKEditorSettings* SpatialEditorSettings = GetDefault<USpatialGDKEditorSettings>();
		return SnapshotPath.FilePath.IsEmpty()
			? FPaths::Combine(SpatialEditorSettings->GetSpatialOSSnapshotFolderPath(), SpatialEditorSettings->GetSpatialOSSnapshotFile())
			: SnapshotPath.FilePath;
	}

	void SetPrimaryRegionCode(const ERegionCode::Type RegionCode);
	FORCEINLINE FText GetPrimaryRegionCode() const
	{
		if (!IsRegionCodeValid(PrimaryDeploymentRegionCode))
		{
			return FText::FromString(TEXT("Invalid"));
		}

		UEnum* Region = FindObject<UEnum>(ANY_PACKAGE, TEXT("ERegionCode"), true);

		return Region->GetDisplayNameTextByValue(static_cast<int64>(PrimaryDeploymentRegionCode.GetValue()));
	}

	void SetSimulatedPlayerRegionCode(const ERegionCode::Type RegionCode);
	FORCEINLINE FText GetSimulatedPlayerRegionCode() const
	{
		if (!IsRegionCodeValid(SimulatedPlayerDeploymentRegionCode))
		{
			return FText::FromString(TEXT("Invalid"));
		}

		UEnum* Region = FindObject<UEnum>(ANY_PACKAGE, TEXT("ERegionCode"), true);

		return Region->GetDisplayNameTextByValue(static_cast<int64>(SimulatedPlayerDeploymentRegionCode.GetValue()));
	}

	void SetSimulatedPlayersEnabledState(bool IsEnabled);
	FORCEINLINE bool IsSimulatedPlayersEnabled() const
	{
		return bSimulatedPlayersIsEnabled;
	}

	void SetSimulatedPlayerDeploymentName(const FString& Name);
	FORCEINLINE FString GetSimulatedPlayerDeploymentName() const
	{
		return SimulatedPlayerDeploymentName;
	}

	FORCEINLINE FString GetSimulatedPlayerLaunchConfigPath() const
	{
		return SimulatedPlayerLaunchConfigPath;
	}

	void SetNumberOfSimulatedPlayers(uint32 Number);
	FORCEINLINE uint32 GetNumberOfSimulatedPlayer() const
	{
		return NumberOfSimulatedPlayers;
	}

	FORCEINLINE FString GetDeploymentLauncherPath() const
	{
		return FPaths::ConvertRelativePathToFull(FPaths::Combine(FPaths::ProjectDir() / TEXT("Plugins/UnrealGDK/SpatialGDK/Binaries/ThirdParty/Improbable/Programs/DeploymentLauncher")));
	}

	bool IsDeploymentConfigurationValid() const;
};
