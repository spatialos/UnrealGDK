// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "SpatialGDKEditorSettings.h"

#include "Dom/JsonObject.h"
#include "Internationalization/Regex.h"
#include "ISettingsModule.h"
#include "Misc/FileHelper.h"
#include "Misc/MessageDialog.h"
#include "Modules/ModuleManager.h"
#include "Serialization/JsonReader.h"
#include "Serialization/JsonSerializer.h"
#include "Settings/LevelEditorPlaySettings.h"
#include "Templates/SharedPointer.h"
#include "SpatialConstants.h"
#include "SpatialGDKSettings.h"


USpatialGDKEditorSettings::USpatialGDKEditorSettings(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
	, bShowSpatialServiceButton(false)
	, bDeleteDynamicEntities(true)
	, bGenerateDefaultLaunchConfig(true)
	, bStopSpatialOnExit(false)
	, bAutoStartLocalDeployment(true)
	, PrimaryDeploymentRegionCode(ERegionCode::US)
	, SimulatedPlayerLaunchConfigPath(FPaths::ConvertRelativePathToFull(FPaths::Combine(FPaths::ProjectDir() /
		TEXT("Plugins/UnrealGDK/SpatialGDK/Build/Programs/Improbable.Unreal.Scripts/WorkerCoordinator/SpatialConfig/cloud_launch_sim_player_deployment.json"))))
	, SimulatedPlayerDeploymentRegionCode(ERegionCode::US)
{
	SpatialOSLaunchConfig.FilePath = GetSpatialOSLaunchConfig();
	SpatialOSSnapshotFile = GetSpatialOSSnapshotFile();
	ProjectName = GetProjectNameFromSpatial();
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
		SetLevelEditorPlaySettingsWorkerTypes();
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
	SetLevelEditorPlaySettingsWorkerTypes();
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
		RuntimeSettings->SaveConfig(CPF_Config, *RuntimeSettings->GetDefaultConfigFilename());
	}
}

void USpatialGDKEditorSettings::SetLevelEditorPlaySettingsWorkerTypes()
{
	ULevelEditorPlaySettings* PlayInSettings = GetMutableDefault<ULevelEditorPlaySettings>();

	PlayInSettings->WorkerTypesToLaunch.Empty(LaunchConfigDesc.ServerWorkers.Num());
	for (const FWorkerTypeLaunchSection& WorkerLaunch : LaunchConfigDesc.ServerWorkers)
	{
		PlayInSettings->WorkerTypesToLaunch.Add(WorkerLaunch.WorkerTypeName, WorkerLaunch.NumEditorInstances);
	}
}

FString USpatialGDKEditorSettings::GetProjectNameFromSpatial() const
{
	FString FileContents;
	const FString SpatialOSFile = FSpatialGDKServicesModule::GetSpatialOSDirectory().Append(TEXT("/spatialos.json"));

	if (!FFileHelper::LoadFileToString(FileContents, *SpatialOSFile))
	{
		return TEXT("");
	}

	TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(FileContents);
	TSharedPtr<FJsonObject> JsonObject;

	if (FJsonSerializer::Deserialize(Reader, JsonObject))
	{
		return JsonObject->GetStringField("name");
	}

	return FString();
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

void USpatialGDKEditorSettings::SetProjectName(const FString& Name)
{
	ProjectName = Name;
	SaveConfig();
}

void USpatialGDKEditorSettings::SetPrimaryLaunchConfigPath(const FString& Path)
{
	PrimaryLaunchConfigPath.FilePath = FPaths::ConvertRelativePathToFull(Path);
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

void USpatialGDKEditorSettings::SetSimulatedPlayerRegionCode(const ERegionCode::Type RegionCode)
{
	SimulatedPlayerDeploymentRegionCode = RegionCode;
}

void USpatialGDKEditorSettings::SetSimulatedPlayersEnabledState(bool IsEnabled)
{
	bSimulatedPlayersIsEnabled = IsEnabled;
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

bool USpatialGDKEditorSettings::IsDeploymentConfigurationValid() const
{
	bool result = IsAssemblyNameValid(AssemblyName) &&
		IsDeploymentNameValid(PrimaryDeploymentName) &&
		IsProjectNameValid(ProjectName) &&
		!SnapshotPath.FilePath.IsEmpty() &&
		!PrimaryLaunchConfigPath.FilePath.IsEmpty() &&
		IsRegionCodeValid(PrimaryDeploymentRegionCode);

	if (IsSimulatedPlayersEnabled())
	{
		result = result &&
			IsDeploymentNameValid(SimulatedPlayerDeploymentName) &&
			!SimulatedPlayerLaunchConfigPath.IsEmpty() &&
			IsRegionCodeValid(SimulatedPlayerDeploymentRegionCode);
	}

	return result;
}
