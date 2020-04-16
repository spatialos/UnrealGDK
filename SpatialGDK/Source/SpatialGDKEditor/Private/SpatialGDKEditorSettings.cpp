// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "SpatialGDKEditorSettings.h"

#include "Internationalization/Regex.h"
#include "ISettingsModule.h"
#include "Misc/FileHelper.h"
#include "Misc/MessageDialog.h"
#include "Modules/ModuleManager.h"
#include "Settings/LevelEditorPlaySettings.h"
#include "Templates/SharedPointer.h"
#include "SpatialConstants.h"
#include "SpatialGDKSettings.h"

#include "Serialization/JsonReader.h"
#include "Serialization/JsonSerializer.h"

DEFINE_LOG_CATEGORY(LogSpatialEditorSettings);
#define LOCTEXT_NAMESPACE "USpatialGDKEditorSettings"

void FSpatialLaunchConfigDescription::SetLevelEditorPlaySettingsWorkerTypes()
{
	ULevelEditorPlaySettings* PlayInSettings = GetMutableDefault<ULevelEditorPlaySettings>();

	PlayInSettings->WorkerTypesToLaunch.Empty(ServerWorkers.Num());
	for (const FWorkerTypeLaunchSection& WorkerLaunch : ServerWorkers)
	{
		PlayInSettings->WorkerTypesToLaunch.Add(WorkerLaunch.WorkerTypeName, WorkerLaunch.NumEditorInstances);
	}
}

USpatialGDKEditorSettings::USpatialGDKEditorSettings(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
	, bShowSpatialServiceButton(false)
	, bDeleteDynamicEntities(true)
	, bGenerateDefaultLaunchConfig(true)
	, bUseGDKPinnedRuntimeVersion(true)
	, bExposeRuntimeIP(false)
	, ExposedRuntimeIP(TEXT(""))
	, bStopSpatialOnExit(false)
	, bAutoStartLocalDeployment(true)
	, PrimaryDeploymentRegionCode(ERegionCode::US)
	, SimulatedPlayerLaunchConfigPath(FSpatialGDKServicesModule::GetSpatialGDKPluginDirectory(TEXT("SpatialGDK/Build/Programs/Improbable.Unreal.Scripts/WorkerCoordinator/SpatialConfig/cloud_launch_sim_player_deployment.json")))
	, bUseDevelopmentAuthenticationFlow(false)
	, SimulatedPlayerDeploymentRegionCode(ERegionCode::US)
{
	SpatialOSLaunchConfig.FilePath = GetSpatialOSLaunchConfig();
	SpatialOSSnapshotToSave = GetSpatialOSSnapshotToSave();
	SpatialOSSnapshotToLoad = GetSpatialOSSnapshotToLoad();
}

const FString& USpatialGDKEditorSettings::GetSpatialOSRuntimeVersionForLocal() const
{
	if (bUseGDKPinnedRuntimeVersion || LocalRuntimeVersion.IsEmpty())
	{
		return SpatialGDKServicesConstants::SpatialOSRuntimePinnedVersion;
	}
	return LocalRuntimeVersion;
}

const FString& USpatialGDKEditorSettings::GetSpatialOSRuntimeVersionForCloud() const
{
	if (bUseGDKPinnedRuntimeVersion || CloudRuntimeVersion.IsEmpty())
	{
		return SpatialGDKServicesConstants::SpatialOSRuntimePinnedVersion;
	}
	return CloudRuntimeVersion;
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
	else if (Name == GET_MEMBER_NAME_CHECKED(USpatialGDKEditorSettings, LaunchConfigDesc))
	{
		SetRuntimeWorkerTypes();
	}
	else if (Name == GET_MEMBER_NAME_CHECKED(USpatialGDKEditorSettings, bUseDevelopmentAuthenticationFlow))
	{
		SetRuntimeUseDevelopmentAuthenticationFlow();
	}
	else if (Name == GET_MEMBER_NAME_CHECKED(USpatialGDKEditorSettings, DevelopmentAuthenticationToken))
	{
		SetRuntimeDevelopmentAuthenticationToken();
	}
	else if (Name == GET_MEMBER_NAME_CHECKED(USpatialGDKEditorSettings, DevelopmentDeploymentToConnect))
	{
		SetRuntimeDevelopmentDeploymentToConnect();
	}
}

void USpatialGDKEditorSettings::PostInitProperties()
{
	Super::PostInitProperties();

	ULevelEditorPlaySettings* PlayInSettings = GetMutableDefault<ULevelEditorPlaySettings>();
	PlayInSettings->SetDeleteDynamicEntities(bDeleteDynamicEntities);
	PlayInSettings->PostEditChange();
	PlayInSettings->SaveConfig();

	SetRuntimeWorkerTypes();
	SetRuntimeUseDevelopmentAuthenticationFlow();
	SetRuntimeDevelopmentAuthenticationToken();
	SetRuntimeDevelopmentDeploymentToConnect();
}

void USpatialGDKEditorSettings::SetRuntimeWorkerTypes()
{
	TSet<FName> WorkerTypes;
	
	for (const FWorkerTypeLaunchSection& WorkerLaunch : LaunchConfigDesc.ServerWorkers)
	{
		if (WorkerLaunch.WorkerTypeName != NAME_None)
		{
			WorkerTypes.Add(WorkerLaunch.WorkerTypeName);
		}
	}

	USpatialGDKSettings* RuntimeSettings = GetMutableDefault<USpatialGDKSettings>();
	if (RuntimeSettings != nullptr)
	{
		RuntimeSettings->ServerWorkerTypes.Empty(WorkerTypes.Num());
		RuntimeSettings->ServerWorkerTypes.Append(WorkerTypes);
		RuntimeSettings->PostEditChange();
		RuntimeSettings->UpdateSinglePropertyInConfigFile(RuntimeSettings->GetClass()->FindPropertyByName(GET_MEMBER_NAME_CHECKED(USpatialGDKSettings, ServerWorkerTypes)), RuntimeSettings->GetDefaultConfigFilename());
	}
}

void USpatialGDKEditorSettings::SetRuntimeUseDevelopmentAuthenticationFlow()
{
	USpatialGDKSettings* RuntimeSettings = GetMutableDefault<USpatialGDKSettings>();
	RuntimeSettings->bUseDevelopmentAuthenticationFlow = bUseDevelopmentAuthenticationFlow;
}

void USpatialGDKEditorSettings::SetRuntimeDevelopmentAuthenticationToken()
{
	USpatialGDKSettings* RuntimeSettings = GetMutableDefault<USpatialGDKSettings>();
	RuntimeSettings->DevelopmentAuthenticationToken = DevelopmentAuthenticationToken;
}

void USpatialGDKEditorSettings::SetRuntimeDevelopmentDeploymentToConnect()
{
	USpatialGDKSettings* RuntimeSettings = GetMutableDefault<USpatialGDKSettings>();
	RuntimeSettings->DevelopmentDeploymentToConnect = DevelopmentDeploymentToConnect;
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
	SnapshotPath.FilePath = FPaths::ConvertRelativePathToFull(Path);
	SaveConfig();
}

void USpatialGDKEditorSettings::SetPrimaryRegionCode(const ERegionCode::Type RegionCode)
{
	PrimaryDeploymentRegionCode = RegionCode;
}

void USpatialGDKEditorSettings::SetAssemblyWindowsPlatform(const FString& Platform)
{
	AssemblyWindowsPlatform = Platform;
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
}

void USpatialGDKEditorSettings::SetSimulatedPlayersEnabledState(bool IsEnabled)
{
	bSimulatedPlayersIsEnabled = IsEnabled;
	SaveConfig();
}

void USpatialGDKEditorSettings::SetBuildClientWorker(bool bBuild)
{
	bBuildClientWorker = bBuild;
	SaveConfig();
}

void USpatialGDKEditorSettings::SetUseGDKPinnedRuntimeVersion(bool Use)
{
	bUseGDKPinnedRuntimeVersion = Use;
	SaveConfig();
}

void USpatialGDKEditorSettings::SetCustomCloudSpatialOSRuntimeVersion(const FString& Version)
{
	CloudRuntimeVersion = Version;
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
			UE_LOG(LogSpatialEditorSettings, Error, TEXT("Could not open configuration file %s"), *LaunchConfigPath);
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
	if (!(*LaunchConfigJsonRootObject)->TryGetObjectField("load_balancing", LoadBalancingField))
	{
		return false;
	}

	const TArray<TSharedPtr<FJsonValue>>* LayerConfigurations;
	if (!(*LoadBalancingField)->TryGetArrayField("layer_configurations", LayerConfigurations))
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
			 && (*OptionsField)->TryGetBoolField("manual_worker_connection_only", ManualWorkerConnectionFlag)
			 && ManualWorkerConnectionFlag)
			{
				FString WorkerName;
				if (LayerConfiguration->TryGetStringField("layer", WorkerName))
				{
					OutWorkersManuallyLaunched.Add(WorkerName);
				}
				else
				{
					UE_LOG(LogSpatialEditorSettings, Error, TEXT("Invalid configuration file %s, Layer configuration missing its layer field"), *LaunchConfigPath);
				}
			}
		}
	}

	return OutWorkersManuallyLaunched.Num() != 0;
}

bool USpatialGDKEditorSettings::IsDeploymentConfigurationValid() const
{
	bool bValid = true;
	if (!IsAssemblyNameValid(AssemblyName))
	{
		UE_LOG(LogSpatialEditorSettings, Error, TEXT("Assembly name is invalid. It should match the regex: %s"), *SpatialConstants::AssemblyPattern);
		bValid = false;
	}
	if (!IsDeploymentNameValid(PrimaryDeploymentName))
	{
		UE_LOG(LogSpatialEditorSettings, Error, TEXT("Deployment name is invalid. It should match the regex: %s"), *SpatialConstants::DeploymentPattern);
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
	if (GetPrimaryLaunchConfigPath().IsEmpty())
	{
		UE_LOG(LogSpatialEditorSettings, Error, TEXT("Launch config path cannot be empty."));
		bValid = false;
	}

	if (IsSimulatedPlayersEnabled())
	{
		if (!IsDeploymentNameValid(SimulatedPlayerDeploymentName))
		{
			UE_LOG(LogSpatialEditorSettings, Error, TEXT("Simulated player deployment name is invalid. It should match the regex: %s"), *SpatialConstants::DeploymentPattern);
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

	TArray<FString> WorkersManuallyLaunched;
	if (IsManualWorkerConnectionSet(GetPrimaryLaunchConfigPath(), WorkersManuallyLaunched))
	{
		FString WorkersReportString (LOCTEXT("AllowManualWorkerConnection", "Chosen launch configuration will not automatically launch the following worker types. Do you want to continue?\n").ToString());

		for (const FString& Worker : WorkersManuallyLaunched)
		{
			WorkersReportString.Append(FString::Printf(TEXT(" - %s\n"), *Worker));
		}

		if (FMessageDialog::Open(EAppMsgType::YesNo, FText::FromString(WorkersReportString)) != EAppReturnType::Yes)
		{
			return false;
		}
	}

	return bValid;
}
