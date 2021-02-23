// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "SpatialGDKEditorSettings.h"

#include "ISettingsModule.h"
#include "Interfaces/ITargetPlatformManagerModule.h"
#include "Internationalization/Regex.h"
#include "Misc/FileHelper.h"
#include "Misc/MessageDialog.h"
#include "Modules/ModuleManager.h"
#include "Serialization/JsonReader.h"
#include "Serialization/JsonSerializer.h"
#include "Settings/LevelEditorPlaySettings.h"
#include "Templates/SharedPointer.h"

#include "SpatialConstants.h"
#include "SpatialGDKSettings.h"

DEFINE_LOG_CATEGORY(LogSpatialEditorSettings);
#define LOCTEXT_NAMESPACE "SpatialGDKEditorSettings"

const FString& FRuntimeVariantVersion::GetVersionForLocal() const
{
	if (bUseGDKPinnedRuntimeVersionForLocal || LocalRuntimeVersion.IsEmpty())
	{
		return PinnedVersion;
	}
	return LocalRuntimeVersion;
}

const FString& FRuntimeVariantVersion::GetVersionForCloud() const
{
	if (bUseGDKPinnedRuntimeVersionForCloud || CloudRuntimeVersion.IsEmpty())
	{
		return PinnedVersion;
	}
	return CloudRuntimeVersion;
}

USpatialGDKEditorSettings::USpatialGDKEditorSettings(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
	, bDeleteDynamicEntities(true)
	, bGenerateDefaultLaunchConfig(true)
	, StandardRuntimeVersion(SpatialGDKServicesConstants::SpatialOSRuntimePinnedStandardVersion)
	, bUseGDKPinnedInspectorVersion(true)
	, InspectorVersionOverride(TEXT(""))
	, ExposedRuntimeIP(TEXT(""))
	, bAutoStartLocalDeployment(true)
	, bSpatialDebuggerEditorEnabled(false)
	, AutoStopLocalDeployment(EAutoStopLocalDeploymentMode::OnEndPIE)
	, bStopPIEOnTestingCompleted(true)
	, CookAndGeneratePlatform("")
	, CookAndGenerateAdditionalArguments("-cookall -unversioned")
	, PrimaryDeploymentRegionCode(ERegionCode::US)
	, bIsAutoGenerateCloudConfigEnabled(true)
	, SimulatedPlayerLaunchConfigPath(FSpatialGDKServicesModule::GetSpatialGDKPluginDirectory(TEXT(
		  "SpatialGDK/Build/Programs/Improbable.Unreal.Scripts/WorkerCoordinator/SpatialConfig/cloud_launch_sim_player_deployment.json")))
	, bBuildAndUploadAssembly(true)
	, AssemblyBuildConfiguration(TEXT("Development"))
	, bConnectServerToCloud(false)
	, LocalReceptionistPort(SpatialConstants::DEFAULT_SERVER_RECEPTIONIST_PROXY_PORT)
	, ListeningAddress(SpatialConstants::LOCAL_HOST)
	, SimulatedPlayerDeploymentRegionCode(ERegionCode::US)
	, bPackageMobileCommandLineArgs(true)
	, bStartPIEClientsWithLocalLaunchOnDevice(false)
	, SpatialOSNetFlowType(ESpatialOSNetFlow::LocalDeployment)
{
	SpatialOSLaunchConfig.FilePath = GetSpatialOSLaunchConfig();
	SpatialOSSnapshotToSave = GetSpatialOSSnapshotToSave();
	SpatialOSSnapshotToLoad = GetSpatialOSSnapshotToLoad();
	SnapshotPath.FilePath = GetSpatialOSSnapshotToSavePath();

	// TODO: UNR-4472 - Remove this WorkerTypeName renaming when refactoring FLaunchConfigDescription.
	// Force update users settings in-case they have a bad server worker name saved.
	LaunchConfigDesc.ServerWorkerConfiguration.WorkerTypeName = SpatialConstants::DefaultServerWorkerType;
}

FRuntimeVariantVersion& USpatialGDKEditorSettings::GetRuntimeVariantVersion(ESpatialOSRuntimeVariant::Type Variant)
{
	return StandardRuntimeVersion;
}

void USpatialGDKEditorSettings::PostEditChangeProperty(struct FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);

	// Use MemberProperty here so we report the correct member name for nested changes
	const FName Name = (PropertyChangedEvent.MemberProperty != nullptr) ? PropertyChangedEvent.MemberProperty->GetFName() : NAME_None;

	if (Name == GET_MEMBER_NAME_CHECKED(USpatialGDKEditorSettings, bDeleteDynamicEntities))
	{
		ULevelEditorPlaySettings* PlayInSettings = GetMutableDefault<ULevelEditorPlaySettings>();
		PlayInSettings->SetDeleteDynamicEntities(bDeleteDynamicEntities);

		PlayInSettings->PostEditChange();
		PlayInSettings->SaveConfig();
	}
	else if (Name == GET_MEMBER_NAME_CHECKED(USpatialGDKEditorSettings, StandardRuntimeVersion))
	{
		FSpatialGDKServicesModule& GDKServices = FModuleManager::GetModuleChecked<FSpatialGDKServicesModule>("SpatialGDKServices");
		GDKServices.GetLocalDeploymentManager()->SetRedeployRequired();

		OnDefaultTemplateNameRequireUpdate.Broadcast();
	}
	else if (Name == GET_MEMBER_NAME_CHECKED(USpatialGDKEditorSettings, PrimaryDeploymentRegionCode))
	{
		OnDefaultTemplateNameRequireUpdate.Broadcast();
	}
	else if (Name == GET_MEMBER_NAME_CHECKED(USpatialGDKEditorSettings, ExposedRuntimeIP))
	{
		if (!USpatialGDKEditorSettings::IsValidIP(ExposedRuntimeIP))
		{
			FMessageDialog::Open(EAppMsgType::Ok, LOCTEXT("InputValidIP_Prompt", "Please input a valid IP address."));
			UE_LOG(LogSpatialEditorSettings, Error, TEXT("Invalid IP address: %s"), *ExposedRuntimeIP);
			// Reset IP to empty instead of keeping the invalid value.
			SetExposedRuntimeIP(TEXT(""));
			return;
		}
	}
	else if (Name == GET_MEMBER_NAME_CHECKED(USpatialGDKEditorSettings, LaunchConfigDesc))
	{
		// TODO: UNR-4472 - Remove this WorkerTypeName renaming when refactoring FLaunchConfigDescription.
		// Force override the server worker name as it MUST be UnrealWorker.
		LaunchConfigDesc.ServerWorkerConfiguration.WorkerTypeName = SpatialConstants::DefaultServerWorkerType;

		if (PropertyChangedEvent.Property->GetFName() == GET_MEMBER_NAME_CHECKED(FSpatialLaunchConfigDescription, RuntimeFlags))
		{
			USpatialGDKEditorSettings::TrimTMap(LaunchConfigDesc.RuntimeFlags);
		}
		if (PropertyChangedEvent.Property->GetFName() == GET_MEMBER_NAME_CHECKED(FWorkerTypeLaunchSection, Flags))
		{
			USpatialGDKEditorSettings::TrimTMap(LaunchConfigDesc.ServerWorkerConfiguration.Flags);

			for (auto& WorkerConfig : LaunchConfigDesc.AdditionalWorkerConfigs)
			{
				USpatialGDKEditorSettings::TrimTMap(WorkerConfig.Flags);
			}
		}
	}
}

void USpatialGDKEditorSettings::PostInitProperties()
{
	Super::PostInitProperties();

	ULevelEditorPlaySettings* PlayInSettings = GetMutableDefault<ULevelEditorPlaySettings>();
	PlayInSettings->SetDeleteDynamicEntities(bDeleteDynamicEntities);
	PlayInSettings->PostEditChange();
	PlayInSettings->SaveConfig();

	const USpatialGDKSettings* GDKSettings = GetDefault<USpatialGDKSettings>();
}

bool USpatialGDKEditorSettings::IsAssemblyNameValid(const FString& Name)
{
	const FRegexPattern AssemblyPatternRegex(SpatialConstants::AssemblyPattern);
	FRegexMatcher RegMatcher(AssemblyPatternRegex, Name);

	return RegMatcher.FindNext();
}

bool USpatialGDKEditorSettings::IsProjectNameValid(const FString& Name)
{
	const FRegexPattern ProjectPatternRegex(SpatialConstants::ProjectPattern);
	FRegexMatcher RegMatcher(ProjectPatternRegex, Name);

	return RegMatcher.FindNext();
}

bool USpatialGDKEditorSettings::IsDeploymentNameValid(const FString& Name)
{
	const FRegexPattern DeploymentPatternRegex(SpatialConstants::DeploymentPattern);
	FRegexMatcher RegMatcher(DeploymentPatternRegex, Name);

	return RegMatcher.FindNext();
}

bool USpatialGDKEditorSettings::IsRegionCodeValid(const ERegionCode::Type RegionCode)
{
	// Selecting CN region code in the Cloud Deployment Configuration window has been deprecated.
	// It will now be automatically determined based on the services region.
	if (RegionCode == ERegionCode::CN)
	{
		return false;
	}

	UEnum* pEnum = FindObject<UEnum>(ANY_PACKAGE, TEXT("ERegionCode"), true);

	return pEnum != nullptr && pEnum->IsValidEnumValue(RegionCode);
}

void USpatialGDKEditorSettings::SetPrimaryDeploymentName(const FString& Name)
{
	PrimaryDeploymentName = Name;
	SaveConfig();
}

void USpatialGDKEditorSettings::SetAssemblyName(const FString& Name)
{
	AssemblyName = Name;
	SaveConfig();
}

void USpatialGDKEditorSettings::SetPrimaryLaunchConfigPath(const FString& Path)
{
	// If the path is empty don't try to convert it to a full path.
	if (Path.IsEmpty())
	{
		PrimaryLaunchConfigPath.FilePath = Path;
	}
	else
	{
		PrimaryLaunchConfigPath.FilePath = FPaths::ConvertRelativePathToFull(Path);
	}
	SaveConfig();
}

void USpatialGDKEditorSettings::SetSnapshotPath(const FString& Path)
{
	// If a non-empty path is specified, convert it to full, otherwise just empty the field.
	SnapshotPath.FilePath = Path.IsEmpty() ? TEXT("") : FPaths::ConvertRelativePathToFull(Path);
	SaveConfig();
}

void USpatialGDKEditorSettings::SetPrimaryRegionCode(const ERegionCode::Type RegionCode)
{
	PrimaryDeploymentRegionCode = RegionCode;
	SaveConfig();
}

void USpatialGDKEditorSettings::SetMainDeploymentCluster(const FString& NewCluster)
{
	MainDeploymentCluster = NewCluster;
	SaveConfig();
}

void USpatialGDKEditorSettings::SetDeploymentTags(const FString& Tags)
{
	DeploymentTags = Tags;
	SaveConfig();
}

void USpatialGDKEditorSettings::SetAssemblyBuildConfiguration(const FString& Configuration)
{
	AssemblyBuildConfiguration = Configuration;
	SaveConfig();
}

void USpatialGDKEditorSettings::SetSimulatedPlayerRegionCode(const ERegionCode::Type RegionCode)
{
	SimulatedPlayerDeploymentRegionCode = RegionCode;
	SaveConfig();
}

void USpatialGDKEditorSettings::SetSimulatedPlayerCluster(const FString& NewCluster)
{
	SimulatedPlayerCluster = NewCluster;
	SaveConfig();
}

void USpatialGDKEditorSettings::SetSimulatedPlayersEnabledState(bool IsEnabled)
{
	bSimulatedPlayersIsEnabled = IsEnabled;
	SaveConfig();
}

void USpatialGDKEditorSettings::SetSpatialDebuggerEditorEnabled(bool IsEnabled)
{
	bSpatialDebuggerEditorEnabled = IsEnabled;
}

void USpatialGDKEditorSettings::SetAutoGenerateCloudLaunchConfigEnabledState(bool IsEnabled)
{
	bIsAutoGenerateCloudConfigEnabled = IsEnabled;
	SaveConfig();
}

void USpatialGDKEditorSettings::SetBuildAndUploadAssembly(bool bBuildAndUpload)
{
	bBuildAndUploadAssembly = bBuildAndUpload;
	SaveConfig();
}

void USpatialGDKEditorSettings::SetForceAssemblyOverwrite(bool bForce)
{
	bForceAssemblyOverwrite = bForce;
	SaveConfig();
}

void USpatialGDKEditorSettings::SetBuildClientWorker(bool bBuild)
{
	bBuildClientWorker = bBuild;
	SaveConfig();
}

void USpatialGDKEditorSettings::SetConnectServerToCloud(bool bIsEnabled)
{
	bConnectServerToCloud = bIsEnabled;
	SaveConfig();
}

void USpatialGDKEditorSettings::SetGenerateSchema(bool bGenerate)
{
	bGenerateSchema = bGenerate;
	SaveConfig();
}

void USpatialGDKEditorSettings::SetGenerateSnapshot(bool bGenerate)
{
	bGenerateSnapshot = bGenerate;
	SaveConfig();
}

void USpatialGDKEditorSettings::SetUseGDKPinnedRuntimeVersionForLocal(ESpatialOSRuntimeVariant::Type Variant, bool bUse)
{
	GetRuntimeVariantVersion(Variant).bUseGDKPinnedRuntimeVersionForLocal = bUse;
	SaveConfig();
	FSpatialGDKServicesModule& GDKServices = FModuleManager::GetModuleChecked<FSpatialGDKServicesModule>("SpatialGDKServices");
	GDKServices.GetLocalDeploymentManager()->SetRedeployRequired();
}

void USpatialGDKEditorSettings::SetUseGDKPinnedRuntimeVersionForCloud(ESpatialOSRuntimeVariant::Type Variant, bool bUse)
{
	GetRuntimeVariantVersion(Variant).bUseGDKPinnedRuntimeVersionForCloud = bUse;
	SaveConfig();
}

void USpatialGDKEditorSettings::SetCustomCloudSpatialOSRuntimeVersion(ESpatialOSRuntimeVariant::Type Variant, const FString& Version)
{
	GetRuntimeVariantVersion(Variant).CloudRuntimeVersion = Version;
	SaveConfig();
}

void USpatialGDKEditorSettings::SetSimulatedPlayerDeploymentName(const FString& Name)
{
	SimulatedPlayerDeploymentName = Name;
	SaveConfig();
}

void USpatialGDKEditorSettings::SetNumberOfSimulatedPlayers(uint32 Number)
{
	NumberOfSimulatedPlayers = Number;
	SaveConfig();
}

bool USpatialGDKEditorSettings::IsManualWorkerConnectionSet(const FString& LaunchConfigPath, TArray<FString>& OutWorkersManuallyLaunched)
{
	TSharedPtr<FJsonValue> LaunchConfigJson;
	{
		TUniquePtr<FArchive> ConfigFile(IFileManager::Get().CreateFileReader(*LaunchConfigPath));

		if (!ConfigFile)
		{
			UE_LOG(LogSpatialEditorSettings, Error, TEXT("Configuration file is missing at path %s"), *LaunchConfigPath);
			return false;
		}

		TSharedRef<TJsonReader<char>> JsonReader = TJsonReader<char>::Create(ConfigFile.Get());

		FJsonSerializer::Deserialize(*JsonReader, LaunchConfigJson);
	}

	const TSharedPtr<FJsonObject>* LaunchConfigJsonRootObject;

	if (!LaunchConfigJson || !LaunchConfigJson->TryGetObject(LaunchConfigJsonRootObject))
	{
		UE_LOG(LogSpatialEditorSettings, Error, TEXT("Invalid configuration file %s"), *LaunchConfigPath);
		return false;
	}

	const TSharedPtr<FJsonObject>* LoadBalancingField;
	if (!(*LaunchConfigJsonRootObject)->TryGetObjectField("loadBalancing", LoadBalancingField))
	{
		return false;
	}

	const TArray<TSharedPtr<FJsonValue>>* LayerConfigurations;
	if (!(*LoadBalancingField)->TryGetArrayField("layerConfigurations", LayerConfigurations))
	{
		return false;
	}

	for (const auto& LayerConfigurationValue : *LayerConfigurations)
	{
		if (const TSharedPtr<FJsonObject> LayerConfiguration = LayerConfigurationValue->AsObject())
		{
			const TSharedPtr<FJsonObject>* OptionsField;
			bool ManualWorkerConnectionFlag;

			// Check manual_worker_connection flag, if it exists.
			if (LayerConfiguration->TryGetObjectField("options", OptionsField)
				&& (*OptionsField)->TryGetBoolField("manualWorkerConnectionOnly", ManualWorkerConnectionFlag) && ManualWorkerConnectionFlag)
			{
				FString WorkerName;
				if (LayerConfiguration->TryGetStringField("layer", WorkerName))
				{
					OutWorkersManuallyLaunched.Add(WorkerName);
				}
				else
				{
					UE_LOG(LogSpatialEditorSettings, Error,
						   TEXT("Invalid configuration file %s, Layer configuration missing its layer field"), *LaunchConfigPath);
				}
			}
		}
	}

	return OutWorkersManuallyLaunched.Num() != 0;
}

bool USpatialGDKEditorSettings::IsDeploymentConfigurationValid() const
{
	bool bValid = true;
	if (!IsProjectNameValid(FSpatialGDKServicesModule::GetProjectName()))
	{
		UE_LOG(LogSpatialEditorSettings, Error, TEXT("Project name is invalid. %s"), *SpatialConstants::ProjectPatternHint.ToString());
		bValid = false;
	}
	if (!IsAssemblyNameValid(AssemblyName))
	{
		UE_LOG(LogSpatialEditorSettings, Error, TEXT("Assembly name is invalid. %s"), *SpatialConstants::AssemblyPatternHint.ToString());
		bValid = false;
	}
	if (!IsDeploymentNameValid(PrimaryDeploymentName))
	{
		UE_LOG(LogSpatialEditorSettings, Error, TEXT("Deployment name is invalid. %s"),
			   *SpatialConstants::DeploymentPatternHint.ToString());
		bValid = false;
	}
	if (!IsRegionCodeValid(PrimaryDeploymentRegionCode))
	{
		UE_LOG(LogSpatialEditorSettings, Error, TEXT("Region code is invalid."));
		bValid = false;
	}
	if (GetSnapshotPath().IsEmpty())
	{
		UE_LOG(LogSpatialEditorSettings, Error, TEXT("Snapshot path cannot be empty."));
		bValid = false;
	}
	if (GetPrimaryLaunchConfigPath().IsEmpty() && !bIsAutoGenerateCloudConfigEnabled)
	{
		UE_LOG(LogSpatialEditorSettings, Error, TEXT("Launch config path cannot be empty."));
		bValid = false;
	}

	if (IsSimulatedPlayersEnabled())
	{
		if (!IsDeploymentNameValid(SimulatedPlayerDeploymentName))
		{
			UE_LOG(LogSpatialEditorSettings, Error, TEXT("Simulated player deployment name is invalid. %s"),
				   *SpatialConstants::DeploymentPatternHint.ToString());
			bValid = false;
		}
		if (!IsRegionCodeValid(SimulatedPlayerDeploymentRegionCode))
		{
			UE_LOG(LogSpatialEditorSettings, Error, TEXT("Simulated player region code is invalid."));
			bValid = false;
		}
		if (GetSimulatedPlayerLaunchConfigPath().IsEmpty())
		{
			UE_LOG(LogSpatialEditorSettings, Error, TEXT("Simulated player launch config path cannot be empty."));
			bValid = false;
		}
	}

	return bValid;
}

bool USpatialGDKEditorSettings::CheckManualWorkerConnectionOnLaunch() const
{
	TArray<FString> WorkersManuallyLaunched;
	if (IsManualWorkerConnectionSet(GetPrimaryLaunchConfigPath(), WorkersManuallyLaunched))
	{
		FString WorkersReportString(
			LOCTEXT("AllowManualWorkerConnection",
					"Chosen launch configuration will not automatically launch the following worker types. Do you want to continue?\n")
				.ToString());

		for (const FString& Worker : WorkersManuallyLaunched)
		{
			WorkersReportString.Append(FString::Printf(TEXT(" - %s\n"), *Worker));
		}

		if (FMessageDialog::Open(EAppMsgType::YesNo, FText::FromString(WorkersReportString)) != EAppReturnType::Yes)
		{
			return false;
		}
	}

	return true;
}

void USpatialGDKEditorSettings::SetDevelopmentAuthenticationToken(const FString& Token)
{
	DevelopmentAuthenticationToken = Token;
	SaveConfig();
}

bool USpatialGDKEditorSettings::IsValidIP(const FString& IP)
{
	const FRegexPattern IpV4PatternRegex(SpatialConstants::Ipv4Pattern);
	FRegexMatcher IpV4RegexMatcher(IpV4PatternRegex, IP);
	return IP.IsEmpty() || IpV4RegexMatcher.FindNext();
}

void USpatialGDKEditorSettings::SetExposedRuntimeIP(const FString& RuntimeIP)
{
	ExposedRuntimeIP = RuntimeIP;
	SaveConfig();
}

void USpatialGDKEditorSettings::SetSpatialOSNetFlowType(ESpatialOSNetFlow::Type NetFlowType)
{
	SpatialOSNetFlowType = NetFlowType;
	SaveConfig();
}

FString USpatialGDKEditorSettings::GetCookAndGenerateSchemaTargetPlatform() const
{
	if (!CookAndGeneratePlatform.IsEmpty())
	{
		return CookAndGeneratePlatform;
	}

	// Return current Editor's Build variant as default.
	return FPlatformProcess::GetBinariesSubdirectory();
}

void USpatialGDKEditorSettings::TrimTMap(TMap<FString, FString>& Map)
{
	for (auto& Flag : Map)
	{
		Flag.Key.TrimStartAndEndInline();
		Flag.Value.TrimStartAndEndInline();
	}
}

const FString& FSpatialLaunchConfigDescription::GetTemplate() const
{
	if (bUseDefaultTemplateForRuntimeVariant)
	{
		return GetDefaultTemplateForRuntimeVariant();
	}

	return Template;
}

const FString& FSpatialLaunchConfigDescription::GetDefaultTemplateForRuntimeVariant() const
{
	if (GetDefault<USpatialGDKSettings>()->IsRunningInChina())
	{
		return SpatialGDKServicesConstants::PinnedChinaStandardRuntimeTemplate;
	}
	else
	{
		return SpatialGDKServicesConstants::PinnedStandardRuntimeTemplate;
	}
}

#undef LOCTEXT_NAMESPACE
