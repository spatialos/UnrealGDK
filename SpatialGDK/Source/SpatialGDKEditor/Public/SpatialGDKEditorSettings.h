// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "Engine/EngineTypes.h"
#include "Misc/Paths.h"
#include "SpatialConstants.h"
#include "UObject/Package.h"
#include "SpatialGDKServicesConstants.h"
#include "SpatialGDKServicesModule.h"

#include "SpatialGDKEditorSettings.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(LogSpatialEditorSettings, Log, All);

USTRUCT()
struct FWorldLaunchSection
{
	GENERATED_BODY()

	FWorldLaunchSection()
		: Dimensions(2000, 2000)
		, ChunkEdgeLengthMeters(50)
		, SnapshotWritePeriodSeconds(0)
	{
		LegacyFlags.Add(TEXT("bridge_qos_max_timeout"), TEXT("0"));
		LegacyFlags.Add(TEXT("bridge_soft_handover_enabled"), TEXT("false"));
		LegacyFlags.Add(TEXT("bridge_single_port_max_heartbeat_timeout_ms"), TEXT("3600000"));
	}

	/** The size of the simulation, in meters, for the auto-generated launch configuration file. */
	UPROPERTY(Category = "SpatialGDK", EditAnywhere, config, meta = (DisplayName = "Simulation dimensions in meters"))
	FIntPoint Dimensions;

	/** The size of the grid squares that the world is divided into, in “world units” (an arbitrary unit that worker instances can interpret as they choose). */
	UPROPERTY(Category = "SpatialGDK", EditAnywhere, config, meta = (DisplayName = "Chunk edge length in meters"))
	int32 ChunkEdgeLengthMeters;

	/** The frequency in seconds to write snapshots of the simulated world. */
	UPROPERTY(Category = "SpatialGDK", EditAnywhere, config, meta = (DisplayName = "Snapshot write period in seconds"))
	int32 SnapshotWritePeriodSeconds;

	/** Legacy non-worker flag configurations. */
	UPROPERTY(Category = "SpatialGDK", EditAnywhere, config)
	TMap<FString, FString> LegacyFlags;

	/** Legacy JVM configurations. */
	UPROPERTY(Category = "SpatialGDK", EditAnywhere, config, meta = (DisplayName = "Legacy Java parameters"))
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
	UPROPERTY(Category = "SpatialGDK", EditAnywhere, config, meta = (DisplayName = "All"))
	bool bAllPermissions;

	/** Enables a worker instance to create new entities. */
	UPROPERTY(Category = "SpatialGDK", EditAnywhere, config, meta = (EditCondition = "!bAllPermissions", DisplayName = "Allow entity creation"))
	bool bAllowEntityCreation;

	/** Enables a worker instance to delete entities. */
	UPROPERTY(Category = "SpatialGDK", EditAnywhere, config, meta = (EditCondition = "!bAllPermissions", DisplayName = "Allow entity deletion"))
	bool bAllowEntityDeletion;

	/** Controls which components can be returned from entity queries that the worker instance performs. If an entity query specifies other components to be returned, the query will fail. */
	UPROPERTY(Category = "SpatialGDK", EditAnywhere, config, meta = (EditCondition = "!bAllPermissions", DisplayName = "Allow entity query"))
	bool bAllowEntityQuery;

	/** Specifies which components can be returned in the query result. */
	UPROPERTY(Category = "SpatialGDK", EditAnywhere, config, meta = (EditCondition = "!bAllPermissions", DisplayName = "Component queries"))
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
	UPROPERTY(Category = "SpatialGDK", EditAnywhere, config)
	FString Duration;

	/** The connection request limit for the duration. */
	UPROPERTY(Category = "SpatialGDK", EditAnywhere, config, meta = (ClampMin = "1", UIMin = "1"))
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
	UPROPERTY(Category = "SpatialGDK", EditAnywhere, config)
	FName WorkerTypeName;

	/** Defines the worker instance's permissions. */
	UPROPERTY(Category = "SpatialGDK", EditAnywhere, config)
	FWorkerPermissionsSection WorkerPermissions;

	/** Defines the maximum number of worker instances that can connect. */
	UPROPERTY(Category = "SpatialGDK", EditAnywhere, config, meta = (DisplayName = "Max connection capacity limit (0 = unlimited capacity)", ClampMin = "0", UIMin = "0"))
	int32 MaxConnectionCapacityLimit;

	/** Enable connection rate limiting. */
	UPROPERTY(Category = "SpatialGDK", EditAnywhere, config, meta = (DisplayName = "Login rate limit enabled"))
	bool bLoginRateLimitEnabled;

	/** Login rate limiting configuration. */
	UPROPERTY(Category = "SpatialGDK", EditAnywhere, config, meta = (EditCondition = "bLoginRateLimitEnabled"))
	FLoginRateLimitSection LoginRateLimit;

	/** Number of columns in the rectangle grid load balancing config. */
	UPROPERTY(Category = "SpatialGDK", EditAnywhere, config, meta = (DisplayName = "Rectangle grid column count", ClampMin = "1", UIMin = "1"))
	int32 Columns;

	/** Number of rows in the rectangle grid load balancing config. */
	UPROPERTY(Category = "SpatialGDK", EditAnywhere, config, meta = (DisplayName = "Rectangle grid row count", ClampMin = "1", UIMin = "1"))
	int32 Rows;

	/** Number of instances to launch when playing in editor. */
	UPROPERTY(Category = "SpatialGDK", EditAnywhere, config, meta = (DisplayName = "Instances to launch in editor", ClampMin = "0", UIMin = "0"))
	int32 NumEditorInstances;

	/** Flags defined for a worker instance. */
	UPROPERTY(Category = "SpatialGDK", EditAnywhere, config, meta = (DisplayName = "Flags"))
	TMap<FString, FString> Flags;

	/** Determines if the worker instance is launched manually or by SpatialOS. */
	UPROPERTY(Category = "SpatialGDK", EditAnywhere, config, meta = (DisplayName = "Manual worker connection only"))
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

	FSpatialLaunchConfigDescription(const FName& WorkerTypeName)
		: Template(TEXT("w2_r0500_e5"))
		, World()
	{
		FWorkerTypeLaunchSection UnrealWorkerDefaultSetting;
		UnrealWorkerDefaultSetting.WorkerTypeName = WorkerTypeName;
		UnrealWorkerDefaultSetting.Rows = 1;
		UnrealWorkerDefaultSetting.Columns = 1;
		UnrealWorkerDefaultSetting.bManualWorkerConnectionOnly = true;

		ServerWorkers.Add(UnrealWorkerDefaultSetting);
	}


	/** Set WorkerTypesToLaunch in level editor play settings. */
	SPATIALGDKEDITOR_API void SetLevelEditorPlaySettingsWorkerTypes();

	/** Deployment template. */
	UPROPERTY(Category = "SpatialGDK", EditAnywhere, config)
	FString Template;

	/** Configuration for the simulated world. */
	UPROPERTY(Category = "SpatialGDK", EditAnywhere, config)
	FWorldLaunchSection World;

	/** Worker-specific configuration parameters. */
	UPROPERTY(Category = "SpatialGDK", EditAnywhere, config, meta = (TitleProperty = "WorkerTypeName"))
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
		CN
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

	/** Set DAT in runtime settings. */
	void SetRuntimeUseDevelopmentAuthenticationFlow();
	void SetRuntimeDevelopmentDeploymentToConnect();

public:

	/** If checked, show the Spatial service button on the GDK toolbar which can be used to turn the Spatial service on and off. */
	UPROPERTY(EditAnywhere, config, Category = "General", meta = (DisplayName = "Show Spatial service button"))
	bool bShowSpatialServiceButton;

	/** Select to delete all a server-worker instance’s dynamically-spawned entities when the server-worker instance shuts down. If NOT selected, a new server-worker instance has all of these entities from the former server-worker instance’s session. */
	UPROPERTY(EditAnywhere, config, Category = "Play in editor settings", meta = (DisplayName = "Delete dynamically spawned entities"))
	bool bDeleteDynamicEntities;

	/** Select the check box for the GDK to auto-generate a launch configuration file for your game when you launch a deployment session. If NOT selected, you must specify a launch configuration `.json` file. */
	UPROPERTY(EditAnywhere, config, Category = "Launch", meta = (DisplayName = "Auto-generate launch configuration file"))
	bool bGenerateDefaultLaunchConfig;

	/** Returns the Runtime version to use for cloud deployments, either the pinned one, or the user-specified one depending of the settings. */
	const FString& GetSpatialOSRuntimeVersionForCloud() const;

	/** Returns the Runtime version to use for local deployments, either the pinned one, or the user-specified one depending of the settings. */
	const FString& GetSpatialOSRuntimeVersionForLocal() const;

	/** Whether to use the GDK-associated SpatialOS runtime version, or to use the one specified in the RuntimeVersion field. */
	UPROPERTY(EditAnywhere, config, Category = "Runtime", meta = (DisplayName = "Use GDK pinned runtime version"))
	bool bUseGDKPinnedRuntimeVersion;

	/** Runtime version to use for local deployments, if not using the GDK pinned version. */
	UPROPERTY(EditAnywhere, config, Category = "Runtime", meta = (EditCondition = "!bUseGDKPinnedRuntimeVersion"))
	FString LocalRuntimeVersion;

	/** Runtime version to use for cloud deployments, if not using the GDK pinned version. */
	UPROPERTY(EditAnywhere, config, Category = "Runtime", meta = (EditCondition = "!bUseGDKPinnedRuntimeVersion"))
	FString CloudRuntimeVersion;

private:

	/** If you are not using auto-generate launch configuration file, specify a launch configuration `.json` file and location here.  */
	UPROPERTY(EditAnywhere, config, Category = "Launch", meta = (EditCondition = "!bGenerateDefaultLaunchConfig", DisplayName = "Launch configuration file path"))
	FFilePath SpatialOSLaunchConfig;

public:
	/** Expose the runtime on a particular IP address when it is running on this machine. Changes are applied on next local deployment startup. */
	UPROPERTY(EditAnywhere, config, Category = "Launch", meta = (DisplayName = "Expose local runtime"))
	bool bExposeRuntimeIP;

	/** If the runtime is set to be exposed, specify on which IP address it should be reachable. Changes are applied on next local deployment startup. */
	UPROPERTY(EditAnywhere, config, Category = "Launch", meta = (EditCondition = "bExposeRuntimeIP", DisplayName = "Exposed local runtime IP address"))
	FString ExposedRuntimeIP;

	/** Select the check box to stop your game’s local deployment when you shut down Unreal Editor. */
	UPROPERTY(EditAnywhere, config, Category = "Launch", meta = (DisplayName = "Stop local deployment on exit"))
	bool bStopSpatialOnExit;

	/** Start a local SpatialOS deployment when clicking 'Play'. */
	UPROPERTY(EditAnywhere, config, Category = "Launch", meta = (DisplayName = "Auto-start local deployment"))
	bool bAutoStartLocalDeployment;

private:
	/** Name of your SpatialOS snapshot file that will be generated. */
	UPROPERTY(EditAnywhere, config, Category = "Snapshots", meta = (DisplayName = "Snapshot to save"))
	FString SpatialOSSnapshotToSave;

	/** Name of your SpatialOS snapshot file that will be loaded during deployment. */
	UPROPERTY(EditAnywhere, config, Category = "Snapshots", meta = (DisplayName = "Snapshot to load"))
	FString SpatialOSSnapshotToLoad;

	/** Add flags to the `spatial local launch` command; they alter the deployment’s behavior. Select the trash icon to remove all the flags.*/
	UPROPERTY(EditAnywhere, config, Category = "Launch", meta = (DisplayName = "Command line flags for local launch"))
	TArray<FString> SpatialOSCommandLineLaunchFlags;

private:
	UPROPERTY(EditAnywhere, config, Category = "Cloud", meta = (DisplayName = "Assembly name"))
		FString AssemblyName;

	UPROPERTY(EditAnywhere, config, Category = "Cloud", meta = (DisplayName = "Deployment name"))
		FString PrimaryDeploymentName;

	UPROPERTY(EditAnywhere, config, Category = "Cloud", meta = (DisplayName = "Cloud launch configuration path"))
		FFilePath PrimaryLaunchConfigPath;

	UPROPERTY(EditAnywhere, config, Category = "Cloud", meta = (DisplayName = "Snapshot path"))
		FFilePath SnapshotPath;

	UPROPERTY(EditAnywhere, config, Category = "Cloud", meta = (DisplayName = "Region"))
		TEnumAsByte<ERegionCode::Type> PrimaryDeploymentRegionCode;

	/** Tags used when launching a deployment */
	UPROPERTY(EditAnywhere, config, Category = "Cloud", meta = (DisplayName = "Deployment tags"))
		FString DeploymentTags;

	const FString SimulatedPlayerLaunchConfigPath;

public:
	/** Windows build architecture for which to build the client assembly */
	UPROPERTY(EditAnywhere, config, Category = "Assembly", meta = (DisplayName = "Windows Build Type"))
		FString AssemblyWindowsPlatform;

	/** The build configuration to use when creating workers for the assembly, e.g. Development */
	UPROPERTY(EditAnywhere, config, Category = "Assembly", meta = (DisplayName = "Build Configuration"))
		FString AssemblyBuildConfiguration;

	/** Allow overwriting an assembly of the same name */
	UPROPERTY(EditAnywhere, config, Category = "Assembly", meta = (DisplayName = "Force Assembly Overwrite"))
		bool bForceAssemblyOverwrite;

	/** Whether to build client worker as part of the assembly */
	UPROPERTY(EditAnywhere, config, Category = "Assembly", meta = (DisplayName = "Build Client Worker"))
		bool bBuildClientWorker;

	/** Whether to generate schema automatically before building an assembly */
	UPROPERTY(EditAnywhere, config, Category = "Assembly", meta = (DisplayName = "Generate Schema"))
		bool bGenerateSchema;

	/** Whether to generate a snapshot automatically before building an assembly */
	UPROPERTY(EditAnywhere, config, Category = "Assembly", meta = (DisplayName = "Generate Snapshot"))
		bool bGenerateSnapshot;

public:
	/** If the Development Authentication Flow is used, the client will try to connect to the cloud rather than local deployment. */
	UPROPERTY(EditAnywhere, config, Category = "Cloud Connection")
		bool bUseDevelopmentAuthenticationFlow;

	/** The token created using 'spatial project auth dev-auth-token' */
	UPROPERTY(EditAnywhere, config, Category = "Cloud Connection")
		FString DevelopmentAuthenticationToken;

	/** The deployment to connect to when using the Development Authentication Flow. If left empty, it uses the first available one (order not guaranteed when there are multiple items). The deployment needs to be tagged with 'dev_login'. */
	UPROPERTY(EditAnywhere, config, Category = "Cloud Connection")
		FString DevelopmentDeploymentToConnect;

private:
	UPROPERTY(EditAnywhere, config, Category = "Simulated Players", meta = (EditCondition = "bSimulatedPlayersIsEnabled", DisplayName = "Region"))
		TEnumAsByte<ERegionCode::Type> SimulatedPlayerDeploymentRegionCode;

	UPROPERTY(EditAnywhere, config, Category = "Simulated Players", meta = (DisplayName = "Include simulated players"))
		bool bSimulatedPlayersIsEnabled;

	UPROPERTY(EditAnywhere, config, Category = "Simulated Players", meta = (EditCondition = "bSimulatedPlayersIsEnabled", DisplayName = "Deployment name"))
		FString SimulatedPlayerDeploymentName;

	UPROPERTY(EditAnywhere, config, Category = "Simulated Players", meta = (EditCondition = "bSimulatedPlayersIsEnabled", DisplayName = "Number of simulated players"))
		uint32 NumberOfSimulatedPlayers;

	static bool IsAssemblyNameValid(const FString& Name);
	static bool IsProjectNameValid(const FString& Name);
	static bool IsDeploymentNameValid(const FString& Name);
	static bool IsRegionCodeValid(const ERegionCode::Type RegionCode);
	static bool IsManualWorkerConnectionSet(const FString& LaunchConfigPath, TArray<FString>& OutWorkersManuallyLaunched);

public:
	UPROPERTY(EditAnywhere, config, Category = "Mobile", meta = (DisplayName = "Connect to a local deployment"))
	bool bMobileConnectToLocalDeployment;

	UPROPERTY(EditAnywhere, config, Category = "Mobile", meta = (EditCondition = "bMobileConnectToLocalDeployment", DisplayName = "Runtime IP to local deployment"))
	FString MobileRuntimeIP;

	UPROPERTY(EditAnywhere, config, Category = "Mobile", meta = (DisplayName = "Mobile Client Worker Type"))
	FString MobileWorkerType = SpatialConstants::DefaultClientWorkerType.ToString();

	UPROPERTY(EditAnywhere, config, Category = "Mobile", meta = (DisplayName = "Extra Command Line Arguments"))
	FString MobileExtraCommandLineArgs;

public:
	/** If you have selected **Auto-generate launch configuration file**, you can change the default options in the file from the drop-down menu. */
	UPROPERTY(EditAnywhere, config, Category = "Launch", meta = (EditCondition = "bGenerateDefaultLaunchConfig", DisplayName = "Launch configuration file options"))
	FSpatialLaunchConfigDescription LaunchConfigDesc;

	FORCEINLINE FString GetSpatialOSLaunchConfig() const
	{
		return SpatialOSLaunchConfig.FilePath.IsEmpty()
			? FPaths::Combine(SpatialGDKServicesConstants::SpatialOSDirectory, TEXT("default_launch.json"))
			: SpatialOSLaunchConfig.FilePath;
	}

	FORCEINLINE FString GetSpatialOSSnapshotToSave() const
	{
		return SpatialOSSnapshotToSave.IsEmpty()
			? FString(TEXT("default.snapshot"))
			: SpatialOSSnapshotToSave;
	}

	FORCEINLINE FString GetSpatialOSSnapshotToSavePath() const
	{
		return FPaths::Combine(GetSpatialOSSnapshotFolderPath(), GetSpatialOSSnapshotToSave());
	}

	FORCEINLINE FString GetSpatialOSSnapshotToLoad() const
	{
		return SpatialOSSnapshotToLoad.IsEmpty()
			? FString(TEXT("default.snapshot"))
			: SpatialOSSnapshotToLoad;
	}

	FORCEINLINE FString GetSpatialOSSnapshotToLoadPath() const
	{
		return FPaths::Combine(GetSpatialOSSnapshotFolderPath(), GetSpatialOSSnapshotToLoad());
	}

	FORCEINLINE FString GetSpatialOSSnapshotFolderPath() const
	{
		return FPaths::Combine(SpatialGDKServicesConstants::SpatialOSDirectory, TEXT("snapshots"));
	}

	FORCEINLINE FString GetGeneratedSchemaOutputFolder() const
	{
		return FPaths::Combine(SpatialGDKServicesConstants::SpatialOSDirectory, TEXT("schema/unreal/generated/"));
	}

	FORCEINLINE FString GetBuiltWorkerFolder() const
	{
		return FPaths::Combine(SpatialGDKServicesConstants::SpatialOSDirectory, TEXT("build/assembly/worker/"));
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

	void SetPrimaryLaunchConfigPath(const FString& Path);
	FORCEINLINE FString GetPrimaryLaunchConfigPath() const
	{

		return PrimaryLaunchConfigPath.FilePath;
	}

	void SetSnapshotPath(const FString& Path);
	FORCEINLINE FString GetSnapshotPath() const
	{
		const USpatialGDKEditorSettings* SpatialEditorSettings = GetDefault<USpatialGDKEditorSettings>();
		return SnapshotPath.FilePath.IsEmpty()
			? SpatialEditorSettings->GetSpatialOSSnapshotToSavePath()
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

	void SetDeploymentTags(const FString& Tags);
	FORCEINLINE FString GetDeploymentTags() const
	{
		return DeploymentTags;
	}

	void SetAssemblyWindowsPlatform(const FString& Platform);
	FORCEINLINE FText GetAssemblyWindowsPlatform() const
	{
		return FText::FromString(AssemblyWindowsPlatform);
	}

	void SetAssemblyBuildConfiguration(const FString& Configuration);
	FORCEINLINE FText GetAssemblyBuildConfiguration() const
	{
		return FText::FromString(AssemblyBuildConfiguration);
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

	void SetBuildClientWorker(bool bBuild);
	FORCEINLINE bool IsBuildClientWorkerEnabled() const
	{
		return bBuildClientWorker;
	}

	void SetGenerateSchema(bool bGenerate);
	FORCEINLINE bool IsGenerateSchemaEnabled() const
	{
		return bGenerateSchema;
	}

	void SetGenerateSnapshot(bool bGenerate);
	FORCEINLINE bool IsGenerateSnapshotEnabled() const
	{
		return bGenerateSnapshot;
	}

	void SetUseGDKPinnedRuntimeVersion(bool IsEnabled);
	FORCEINLINE bool GetUseGDKPinnedRuntimeVersion() const
	{
		return bUseGDKPinnedRuntimeVersion;
	}

	void SetCustomCloudSpatialOSRuntimeVersion(const FString& Version);
	FORCEINLINE const FString& GetCustomCloudSpatialOSRuntimeVersion() const
	{
		return CloudRuntimeVersion;
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
		return FSpatialGDKServicesModule::GetSpatialGDKPluginDirectory(TEXT("SpatialGDK/Binaries/ThirdParty/Improbable/Programs/DeploymentLauncher"));
	}

	bool IsDeploymentConfigurationValid() const;

	void SetRuntimeDevelopmentAuthenticationToken();
};
