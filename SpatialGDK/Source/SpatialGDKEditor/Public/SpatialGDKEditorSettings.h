// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "Engine/EngineTypes.h"
#include "Misc/Paths.h"
#include "UObject/Package.h"

#include "SpatialConstants.h"
#include "SpatialGDKServicesConstants.h"
#include "SpatialGDKServicesModule.h"

#include "SpatialGDKEditorSettings.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(LogSpatialEditorSettings, Log, All);

DECLARE_MULTICAST_DELEGATE(FOnDefaultTemplateNameRequireUpdate)

class FSpatialRuntimeVersionCustomization;
class UAbstractRuntimeLoadBalancingStrategy;
class USpatialGDKEditorSettings;

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

	/** The size of the grid squares that the world is divided into, in “world units” (an arbitrary unit that worker instances can interpret
	 * as they choose). */
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
	UPROPERTY(Category = "SpatialGDK", EditAnywhere, config,
			  meta = (EditCondition = "!bAllPermissions", DisplayName = "Allow entity creation"))
	bool bAllowEntityCreation;

	/** Enables a worker instance to delete entities. */
	UPROPERTY(Category = "SpatialGDK", EditAnywhere, config,
			  meta = (EditCondition = "!bAllPermissions", DisplayName = "Allow entity deletion"))
	bool bAllowEntityDeletion;

	/** Controls which components can be returned from entity queries that the worker instance performs. If an entity query specifies other
	 * components to be returned, the query will fail. */
	UPROPERTY(Category = "SpatialGDK", EditAnywhere, config,
			  meta = (EditCondition = "!bAllPermissions", DisplayName = "Allow entity query"))
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
		: WorkerPermissions()
		, bAutoNumEditorInstances(true)
		, NumEditorInstances(1)
		, bManualWorkerConnectionOnly(false)
	{
	}

	/** Worker type name, deprecated in favor of defining them in the runtime settings.*/
	UPROPERTY(config)
	FName WorkerTypeName_DEPRECATED;

	/** Defines the worker instance's permissions. */
	UPROPERTY(Category = "SpatialGDK", EditAnywhere, config)
	FWorkerPermissionsSection WorkerPermissions;

	/** Automatically or manually specifies the number of worker instances to launch in editor. */
	UPROPERTY(Category = "SpatialGDK", EditAnywhere, config,
			  meta = (DisplayName = "Automatically compute number of instances to launch in Editor"))
	bool bAutoNumEditorInstances;

	/** Number of instances to launch when playing in editor. */
	UPROPERTY(Category = "SpatialGDK", EditAnywhere, config,
			  meta = (DisplayName = "Instances to launch in editor", ClampMin = "0", UIMin = "0",
					  EditCondition = "!bAutoNumEditorInstances"))
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
	friend class USpatialGDKEditorSettings;

	GENERATED_BODY()

	FSpatialLaunchConfigDescription()
		: bUseDefaultTemplateForRuntimeVariant(true)
		, Template()
		, World()
	{
	}

	const FString& GetTemplate() const;

	const FString& GetDefaultTemplateForRuntimeVariant() const;

	/** Use default template for deployments. */
	UPROPERTY(Category = "SpatialGDK", EditAnywhere, config)
	bool bUseDefaultTemplateForRuntimeVariant;

	/** Deployment template. */
	UPROPERTY(Category = "SpatialGDK", EditAnywhere, config, meta = (EditCondition = "!bUseDefaultTemplateForRuntimeVariant"))
	FString Template;

	/** Configuration for the simulated world. */
	UPROPERTY(Category = "SpatialGDK", EditAnywhere, config)
	FWorldLaunchSection World;

	/** Worker-specific configuration parameters. */
	UPROPERTY(config)
	TArray<FWorkerTypeLaunchSection> ServerWorkers_DEPRECATED;

	UPROPERTY(Category = "SpatialGDK", EditAnywhere, EditFixedSize, config)
	FWorkerTypeLaunchSection ServerWorkerConfig;
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
	CN UMETA(Hidden)
};
}

UENUM()
namespace ESpatialOSNetFlow
{
enum Type
{
	LocalDeployment,
	CloudDeployment
};
}

UENUM()
namespace ESpatialOSRuntimeVariant
{
enum Type
{
	Standard,
	CompatibilityMode
};
}

USTRUCT()
struct SPATIALGDKEDITOR_API FRuntimeVariantVersion
{
	friend class USpatialGDKEditorSettings;
	friend class FSpatialRuntimeVersionCustomization;

	GENERATED_BODY()

	FRuntimeVariantVersion()
		: PinnedVersion(SpatialGDKServicesConstants::SpatialOSRuntimePinnedStandardVersion)
	{
	}

	FRuntimeVariantVersion(const FString& InPinnedVersion)
		: PinnedVersion(InPinnedVersion)
	{
	}

	/** Returns the Runtime version to use for cloud deployments, either the pinned one, or the user-specified one depending on the
	 * settings. */
	const FString& GetVersionForCloud() const;

	/** Returns the Runtime version to use for local deployments, either the pinned one, or the user-specified one depending on the
	 * settings. */
	const FString& GetVersionForLocal() const;

	bool GetUseGDKPinnedRuntimeVersionForLocal() const { return bUseGDKPinnedRuntimeVersionForLocal; }

	bool GetUseGDKPinnedRuntimeVersionForCloud() const { return bUseGDKPinnedRuntimeVersionForCloud; }

	const FString& GetPinnedVersion() const { return PinnedVersion; }

private:
	/** Whether to use the GDK-associated SpatialOS runtime version for local deployments, or to use the one specified in the RuntimeVersion
	 * field. */
	UPROPERTY(EditAnywhere, config, Category = "Runtime", meta = (DisplayName = "Use GDK pinned runtime version for local"))
	bool bUseGDKPinnedRuntimeVersionForLocal = true;

	/** Whether to use the GDK-associated SpatialOS runtime version for cloud deployments, or to use the one specified in the RuntimeVersion
	 * field. */
	UPROPERTY(EditAnywhere, config, Category = "Runtime", meta = (DisplayName = "Use GDK pinned runtime version for cloud"))
	bool bUseGDKPinnedRuntimeVersionForCloud = true;

	/** Runtime version to use for local deployments, if not using the GDK pinned version. */
	UPROPERTY(EditAnywhere, config, Category = "Runtime", meta = (EditCondition = "!bUseGDKPinnedRuntimeVersionForLocal"))
	FString LocalRuntimeVersion;

	/** Runtime version to use for cloud deployments, if not using the GDK pinned version. */
	UPROPERTY(EditAnywhere, config, Category = "Runtime", meta = (EditCondition = "!bUseGDKPinnedRuntimeVersionForCloud"))
	FString CloudRuntimeVersion;

private:
	/** Pinned version for this variant. */
	FString PinnedVersion;
};

UCLASS(config = SpatialGDKEditorSettings, defaultconfig, HideCategories = LoadBalancing)
class SPATIALGDKEDITOR_API USpatialGDKEditorSettings : public UObject
{
	GENERATED_BODY()

public:
	USpatialGDKEditorSettings(const FObjectInitializer& ObjectInitializer);

	virtual void PostEditChangeProperty(struct FPropertyChangedEvent& PropertyChangedEvent) override;
	virtual void PostInitProperties() override;

public:
	/** If checked, show the Spatial service button on the GDK toolbar which can be used to turn the Spatial service on and off. */
	UPROPERTY(EditAnywhere, config, Category = "General", meta = (DisplayName = "Show Spatial service button"))
	bool bShowSpatialServiceButton;

	/** Select to delete all a server-worker instance’s dynamically-spawned entities when the server-worker instance shuts down. If NOT
	 * selected, a new server-worker instance has all of these entities from the former server-worker instance’s session. */
	UPROPERTY(EditAnywhere, config, Category = "Play in editor settings", meta = (DisplayName = "Delete dynamically spawned entities"))
	bool bDeleteDynamicEntities;

	/** Select the check box for the GDK to auto-generate a launch configuration file for your game when you launch a deployment session. If
	 * NOT selected, you must specify a launch configuration `.json` file. */
	UPROPERTY(EditAnywhere, config, Category = "Launch", meta = (DisplayName = "Auto-generate launch configuration file"))
	bool bGenerateDefaultLaunchConfig;

	/** Returns which runtime variant we should use. */
	TEnumAsByte<ESpatialOSRuntimeVariant::Type> GetSpatialOSRuntimeVariant() const { return RuntimeVariant; }

	/** Returns the version information for the currently set variant*/
	const FRuntimeVariantVersion& GetSelectedRuntimeVariantVersion() const
	{
		return const_cast<USpatialGDKEditorSettings*>(this)->GetRuntimeVariantVersion(RuntimeVariant);
	}

	UPROPERTY(EditAnywhere, config, Category = "Runtime")
	TEnumAsByte<ESpatialOSRuntimeVariant::Type> RuntimeVariant;

	UPROPERTY(EditAnywhere, config, Category = "Runtime", AdvancedDisplay)
	FRuntimeVariantVersion StandardRuntimeVersion;

	UPROPERTY(EditAnywhere, config, Category = "Runtime", AdvancedDisplay)
	FRuntimeVariantVersion CompatibilityModeRuntimeVersion;

	mutable FOnDefaultTemplateNameRequireUpdate OnDefaultTemplateNameRequireUpdate;

private:
	FRuntimeVariantVersion& GetRuntimeVariantVersion(ESpatialOSRuntimeVariant::Type);

	/** If you are not using auto-generate launch configuration file, specify a launch configuration `.json` file and location here.  */
	UPROPERTY(EditAnywhere, config, Category = "Launch",
			  meta = (EditCondition = "!bGenerateDefaultLaunchConfig", DisplayName = "Launch configuration file path"))
	FFilePath SpatialOSLaunchConfig;

public:
	/** Specify on which IP address the local runtime should be reachable. If empty, the local runtime will not be exposed. Changes are
	 * applied on next local deployment startup. */
	UPROPERTY(EditAnywhere, config, Category = "Launch", meta = (DisplayName = "Exposed local runtime IP address"))
	FString ExposedRuntimeIP;

	UPROPERTY(EditAnywhere, config, Category = "Launch", meta = (DisplayName = "Stop local deployment on stop play in editor"))
	bool bStopLocalDeploymentOnEndPIE;

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

	UPROPERTY(EditAnywhere, config, Category = "Schema Generation",
			  meta = (Tooltip = "Platform to target when using Cook And Generate Schema (if empty, defaults to Editor's platform)"))
	FString CookAndGeneratePlatform;

	UPROPERTY(EditAnywhere, config, Category = "Schema Generation",
			  meta = (Tooltip = "Additional arguments passed to Cook And Generate Schema"))
	FString CookAndGenerateAdditionalArguments;

	/** Add flags to the `spatial local launch` command; they alter the deployment’s behavior. Select the trash icon to remove all the
	 * flags.*/
	UPROPERTY(EditAnywhere, config, Category = "Launch", meta = (DisplayName = "Command line flags for local launch"))
	TArray<FString> SpatialOSCommandLineLaunchFlags;

private:
	UPROPERTY(config)
	FString AssemblyName;

	UPROPERTY(config)
	FString PrimaryDeploymentName;

	UPROPERTY(config)
	FFilePath PrimaryLaunchConfigPath;

	UPROPERTY(config)
	FFilePath SnapshotPath;

	UPROPERTY(config)
	TEnumAsByte<ERegionCode::Type> PrimaryDeploymentRegionCode;

	UPROPERTY(config)
	FString MainDeploymentCluster;

	/** Tags used when launching a deployment */
	UPROPERTY(config)
	FString DeploymentTags;

	UPROPERTY(config)
	bool bIsAutoGenerateCloudConfigEnabled;

	const FString SimulatedPlayerLaunchConfigPath;

public:
	/** Whether to build and upload the assembly when starting the cloud deployment. */
	UPROPERTY(EditAnywhere, config, Category = "Assembly", meta = (DisplayName = "Build and Upload Assembly"))
	bool bBuildAndUploadAssembly;

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

	/** Extra arguments to pass when building the server worker. */
	UPROPERTY(EditAnywhere, config, Category = "Assembly")
	FString BuildServerExtraArgs;

	/** Extra arguments to pass when building the client worker. */
	UPROPERTY(EditAnywhere, config, Category = "Assembly")
	FString BuildClientExtraArgs;

	/** Extra arguments to pass when building the simulated player worker. */
	UPROPERTY(EditAnywhere, config, Category = "Assembly")
	FString BuildSimulatedPlayerExtraArgs;

	/** The token created using 'spatial project auth dev-auth-token' */
	UPROPERTY(EditAnywhere, config, Category = "Cloud Connection")
	FString DevelopmentAuthenticationToken;

	/** Whether to start local server worker when connecting to cloud deployment. If selected, make sure that the cloud deployment you want
	 * to connect to is not automatically launching Server-workers. (That your workers have "manual_connection_only" enabled) */
	UPROPERTY(EditAnywhere, config, Category = "Cloud Connection",
			  meta = (DisplayName = "Connect local server worker to the cloud deployment"))
	bool bConnectServerToCloud;

	/** Port on which the receptionist proxy will be available. */
	UPROPERTY(EditAnywhere, config, Category = "Cloud Connection",
			  meta = (EditCondition = "bConnectServerToCloud", DisplayName = "Local Receptionist Port"))
	int32 LocalReceptionistPort;

	/** Network address to bind the receptionist proxy to. */
	UPROPERTY(EditAnywhere, config, Category = "Cloud Connection",
			  meta = (EditCondition = "bConnectServerToCloud", DisplayName = "Listening Address"))
	FString ListeningAddress;

private:
	UPROPERTY(config)
	TEnumAsByte<ERegionCode::Type> SimulatedPlayerDeploymentRegionCode;

	UPROPERTY(config)
	FString SimulatedPlayerCluster;

	UPROPERTY(config)
	bool bSimulatedPlayersIsEnabled;

	UPROPERTY(config)
	FString SimulatedPlayerDeploymentName;

	UPROPERTY(config)
	uint32 NumberOfSimulatedPlayers;

	static bool IsRegionCodeValid(const ERegionCode::Type RegionCode);
	static bool IsManualWorkerConnectionSet(const FString& LaunchConfigPath, TArray<FString>& OutWorkersManuallyLaunched);

public:
	/** If checked, use the connection flow override below instead of the one selected in the editor when building the command line for
	 * mobile. */
	UPROPERTY(EditAnywhere, config, Category = "Mobile",
			  meta = (DisplayName = "Override Mobile Connection Flow (only for Push settings to device)"))
	bool bMobileOverrideConnectionFlow;

	/** The connection flow that should be used when pushing command line to the mobile device. */
	UPROPERTY(EditAnywhere, config, Category = "Mobile",
			  meta = (EditCondition = "bMobileOverrideConnectionFlow", DisplayName = "Mobile Connection Flow"))
	TEnumAsByte<ESpatialOSNetFlow::Type> MobileConnectionFlow;

	/** If specified, use this IP instead of 'Exposed local runtime IP address' when building the command line to push to the mobile device.
	 */
	UPROPERTY(EditAnywhere, config, Category = "Mobile", meta = (DisplayName = "Local Runtime IP Override"))
	FString MobileRuntimeIPOverride;

	UPROPERTY(EditAnywhere, config, Category = "Mobile", meta = (DisplayName = "Mobile Client Worker Type"))
	FString MobileWorkerType = SpatialConstants::DefaultClientWorkerType.ToString();

	UPROPERTY(EditAnywhere, config, Category = "Mobile", meta = (DisplayName = "Extra Command Line Arguments"))
	FString MobileExtraCommandLineArgs;

	UPROPERTY(EditAnywhere, config, Category = "Mobile", meta = (DisplayName = "Include Command Line Arguments when Packaging"))
	bool bPackageMobileCommandLineArgs;

	/** If checked, PIE clients will be automatically started when launching on a device and connecting to local deployment. */
	UPROPERTY(EditAnywhere, config, Category = "Mobile",
			  meta = (DisplayName = "Start PIE Clients when launching on a device with local deployment flow"))
	bool bStartPIEClientsWithLocalLaunchOnDevice;

public:
	/** If you have selected **Auto-generate launch configuration file**, you can change the default options in the file from the drop-down
	 * menu. */
	UPROPERTY(EditAnywhere, config, Category = "Launch",
			  meta = (EditCondition = "bGenerateDefaultLaunchConfig", DisplayName = "Launch configuration file options"))
	FSpatialLaunchConfigDescription LaunchConfigDesc;

	/** Select the connection flow that should be used when starting the game with Spatial networking enabled. */
	UPROPERTY(EditAnywhere, config, Category = "Connection Flow", meta = (DisplayName = "SpatialOS Connection Flow Type"))
	TEnumAsByte<ESpatialOSNetFlow::Type> SpatialOSNetFlowType;

	FORCEINLINE FString GetSpatialOSLaunchConfig() const { return SpatialOSLaunchConfig.FilePath; }

	FORCEINLINE FString GetSpatialOSSnapshotToSave() const
	{
		return SpatialOSSnapshotToSave.IsEmpty() ? FString(TEXT("default.snapshot")) : SpatialOSSnapshotToSave;
	}

	FORCEINLINE FString GetSpatialOSSnapshotToSavePath() const
	{
		return FPaths::Combine(GetSpatialOSSnapshotFolderPath(), GetSpatialOSSnapshotToSave());
	}

	FORCEINLINE FString GetSpatialOSSnapshotToLoad() const
	{
		return SpatialOSSnapshotToLoad.IsEmpty() ? FString(TEXT("default.snapshot")) : SpatialOSSnapshotToLoad;
	}

	FString GetCookAndGenerateSchemaTargetPlatform() const;

	FORCEINLINE FString GetCookAndGenerateSchemaAdditionalArgs() const { return CookAndGenerateAdditionalArguments; }

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
	FORCEINLINE FString GetPrimaryDeploymentName() const { return PrimaryDeploymentName; }

	void SetAssemblyName(const FString& Name);
	FORCEINLINE FString GetAssemblyName() const { return AssemblyName; }

	void SetPrimaryLaunchConfigPath(const FString& Path);
	FORCEINLINE FString GetPrimaryLaunchConfigPath() const { return PrimaryLaunchConfigPath.FilePath; }

	void SetSnapshotPath(const FString& Path);
	FORCEINLINE FString GetSnapshotPath() const { return SnapshotPath.FilePath; }

	void SetPrimaryRegionCode(const ERegionCode::Type RegionCode);
	FORCEINLINE FText GetPrimaryRegionCodeText() const
	{
		if (!IsRegionCodeValid(PrimaryDeploymentRegionCode))
		{
			return NSLOCTEXT("SpatialGDKEditorSettings", "InvalidRegion", "Invalid");
		}

		UEnum* Region = FindObject<UEnum>(ANY_PACKAGE, TEXT("ERegionCode"), true);

		return Region->GetDisplayNameTextByValue(static_cast<int64>(PrimaryDeploymentRegionCode.GetValue()));
	}

	const ERegionCode::Type GetPrimaryRegionCode() const { return PrimaryDeploymentRegionCode; }

	void SetMainDeploymentCluster(const FString& NewCluster);
	FORCEINLINE FString GetMainDeploymentCluster() const { return MainDeploymentCluster; }

	void SetDeploymentTags(const FString& Tags);
	FORCEINLINE FString GetDeploymentTags() const { return DeploymentTags; }

	void SetAssemblyBuildConfiguration(const FString& Configuration);
	FORCEINLINE FText GetAssemblyBuildConfiguration() const { return FText::FromString(AssemblyBuildConfiguration); }

	void SetSimulatedPlayerRegionCode(const ERegionCode::Type RegionCode);
	FORCEINLINE FText GetSimulatedPlayerRegionCode() const
	{
		if (!IsRegionCodeValid(SimulatedPlayerDeploymentRegionCode))
		{
			return NSLOCTEXT("SpatialGDKEditorSettings", "InvalidRegion", "Invalid");
		}

		UEnum* Region = FindObject<UEnum>(ANY_PACKAGE, TEXT("ERegionCode"), true);

		return Region->GetDisplayNameTextByValue(static_cast<int64>(SimulatedPlayerDeploymentRegionCode.GetValue()));
	}

	void SetSimulatedPlayersEnabledState(bool IsEnabled);
	FORCEINLINE bool IsSimulatedPlayersEnabled() const { return bSimulatedPlayersIsEnabled; }

	void SetAutoGenerateCloudLaunchConfigEnabledState(bool IsEnabled);
	FORCEINLINE bool ShouldAutoGenerateCloudLaunchConfig() const { return bIsAutoGenerateCloudConfigEnabled; }

	void SetBuildAndUploadAssembly(bool bBuildAndUpload);
	FORCEINLINE bool ShouldBuildAndUploadAssembly() const { return bBuildAndUploadAssembly; }

	void SetForceAssemblyOverwrite(bool bForce);
	FORCEINLINE bool IsForceAssemblyOverwriteEnabled() const { return bForceAssemblyOverwrite; }

	void SetBuildClientWorker(bool bBuild);
	FORCEINLINE bool IsBuildClientWorkerEnabled() const { return bBuildClientWorker; }

	void SetGenerateSchema(bool bGenerate);
	FORCEINLINE bool IsGenerateSchemaEnabled() const { return bGenerateSchema; }

	void SetGenerateSnapshot(bool bGenerate);
	FORCEINLINE bool IsGenerateSnapshotEnabled() const { return bGenerateSnapshot; }

	void SetUseGDKPinnedRuntimeVersionForLocal(ESpatialOSRuntimeVariant::Type Variant, bool IsEnabled);
	void SetUseGDKPinnedRuntimeVersionForCloud(ESpatialOSRuntimeVariant::Type Variant, bool IsEnabled);
	void SetCustomCloudSpatialOSRuntimeVersion(ESpatialOSRuntimeVariant::Type Variant, const FString& Version);

	void SetSimulatedPlayerDeploymentName(const FString& Name);
	FORCEINLINE FString GetSimulatedPlayerDeploymentName() const { return SimulatedPlayerDeploymentName; }

	void SetConnectServerToCloud(bool bIsEnabled);
	FORCEINLINE bool IsConnectServerToCloudEnabled() const { return bConnectServerToCloud; }

	void SetSimulatedPlayerCluster(const FString& NewCluster);
	FORCEINLINE FString GetSimulatedPlayerCluster() const { return SimulatedPlayerCluster; }

	FORCEINLINE FString GetSimulatedPlayerLaunchConfigPath() const { return SimulatedPlayerLaunchConfigPath; }

	void SetNumberOfSimulatedPlayers(uint32 Number);
	FORCEINLINE uint32 GetNumberOfSimulatedPlayers() const { return NumberOfSimulatedPlayers; }

	FORCEINLINE FString GetDeploymentLauncherPath() const
	{
		return FSpatialGDKServicesModule::GetSpatialGDKPluginDirectory(
			TEXT("SpatialGDK/Binaries/ThirdParty/Improbable/Programs/DeploymentLauncher"));
	}

	bool IsDeploymentConfigurationValid() const;

	void SetDevelopmentAuthenticationToken(const FString& Token);

	static bool IsValidIP(const FString& IP);
	void SetExposedRuntimeIP(const FString& RuntimeIP);

	void SetSpatialOSNetFlowType(ESpatialOSNetFlow::Type NetFlowType);

	static bool IsProjectNameValid(const FString& Name);
	static bool IsAssemblyNameValid(const FString& Name);
	static bool IsDeploymentNameValid(const FString& Name);
};
