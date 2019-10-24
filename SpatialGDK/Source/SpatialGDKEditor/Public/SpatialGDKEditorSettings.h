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

	/** Select to delete all a server-worker instance’s dynamically-spawned entities when the server-worker instance shuts down. If NOT selected, a new server-worker instance has all of these entities from the former server-worker instance’s session. */
	UPROPERTY(EditAnywhere, config, Category = "Play in editor settings", meta = (ConfigRestartRequired = false, DisplayName = "Delete dynamically spawned entities"))
	bool bDeleteDynamicEntities;

	/** Select the check box for the GDK to auto-generate a launch configuration file for your game when you launch a deployment session. If NOT selected, you must specify a launch configuration `.json` file. */
	UPROPERTY(EditAnywhere, config, Category = "Launch", meta = (ConfigRestartRequired = false, DisplayName = "Auto-generate launch configuration file"))
	bool bGenerateDefaultLaunchConfig;

private:
	/** If you are not using auto-generate launch configuration file, specify a launch configuration `.json` file and location here.  */
	UPROPERTY(EditAnywhere, config, Category = "Launch", meta = (EditCondition = "!bGenerateDefaultLaunchConfig", ConfigRestartRequired = false, DisplayName = "Launch configuration file path"))
	FFilePath SpatialOSLaunchConfig;

public:
	/** Select the check box to stop your game’s local deployment when you shut down Unreal Editor. */
	UPROPERTY(EditAnywhere, config, Category = "Launch", meta = (ConfigRestartRequired = false, DisplayName = "Stop local deployment on exit"))
	bool bStopSpatialOnExit;

	/** Start a local SpatialOS deployment when clicking 'Play'. */
	UPROPERTY(EditAnywhere, config, Category = "Launch", meta = (ConfigRestartRequired = false, DisplayName = "Auto-start local deployment"))
	bool bAutoStartLocalDeployment;

private:
	/** Name of your SpatialOS snapshot file. */
	UPROPERTY(EditAnywhere, config, Category = "Snapshots", meta = (ConfigRestartRequired = false, DisplayName = "Snapshot file name"))
	FString SpatialOSSnapshotFile;

	/** Add flags to the `spatial local launch` command; they alter the deployment’s behavior. Select the trash icon to remove all the flags.*/
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

	const FString SimulatedPlayerLaunchConfigPath;

	UPROPERTY(EditAnywhere, config, Category = "Simulated Players", meta = (EditCondition = "bSimulatedPlayersIsEnabled", ConfigRestartRequired = false, DisplayName = "Region"))
		TEnumAsByte<ERegionCode::Type> SimulatedPlayerDeploymentRegionCode;

	UPROPERTY(EditAnywhere, config, Category = "Simulated Players", meta = (ConfigRestartRequired = false, DisplayName = "Include simulated players"))
		bool bSimulatedPlayersIsEnabled;

	UPROPERTY(EditAnywhere, config, Category = "Simulated Players", meta = (EditCondition = "bSimulatedPlayersIsEnabled", ConfigRestartRequired = false, DisplayName = "Deployment name"))
		FString SimulatedPlayerDeploymentName;

	UPROPERTY(EditAnywhere, config, Category = "Simulated Players", meta = (EditCondition = "bSimulatedPlayersIsEnabled", ConfigRestartRequired = false, DisplayName = "Number of simulated players"))
		uint32 NumberOfSimulatedPlayers;
	
	static bool IsAssemblyNameValid(const FString& Name);
	static bool IsProjectNameValid(const FString& Name);
	static bool IsDeploymentNameValid(const FString& Name);
	static bool IsRegionCodeValid(const ERegionCode::Type RegionCode);

public:
	/** If you have selected **Auto-generate launch configuration file**, you can change the default options in the file from the drop-down menu. */
	UPROPERTY(EditAnywhere, config, Category = "Launch", meta = (EditCondition = "bGenerateDefaultLaunchConfig", ConfigRestartRequired = false, DisplayName = "Launch configuration file options"))
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

	FORCEINLINE FString GetSpatialOSSnapshotFile() const
	{
		return SpatialOSSnapshotFile.IsEmpty()
			? FString(TEXT("default.snapshot"))
			: SpatialOSSnapshotFile;
	}

	FORCEINLINE FString GetSpatialOSSnapshotFolderPath() const
	{
		return FPaths::ConvertRelativePathToFull(FPaths::Combine(FSpatialGDKServicesModule::GetSpatialOSDirectory(), TEXT("snapshots")));
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
		return FPaths::ConvertRelativePathToFull(FPaths::Combine(GetGDKPluginDirectory() / TEXT("SpatialGDK/Binaries/ThirdParty/Improbable/Programs/DeploymentLauncher")));
	}

	bool IsDeploymentConfigurationValid() const;
};
