// Copyright (c) Improbable Worlds Ltd, All Rights Reserved
#include "SpatialGDKEditorCloudLauncherSettings.h"

#include "Dom/JsonObject.h"
#include "Internationalization/Regex.h"
#include "Misc/FileHelper.h"
#include "Serialization/JsonReader.h"
#include "Serialization/JsonSerializer.h"
#include "Settings/LevelEditorPlaySettings.h"
#include "Templates/SharedPointer.h"
#include "SpatialConstants.h"

USpatialGDKEditorCloudLauncherSettings::USpatialGDKEditorCloudLauncherSettings(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
	, SimulatedPlayerDeploymentName(FPaths::ConvertRelativePathToFull(FPaths::Combine(FPaths::ProjectDir() /
		TEXT("Plugins/UnrealGDK/SpatialGDK/Build/Programs/Improbable.Unreal.Scripts/WorkerCoordinator/SpatialConfig/cloud_launch_sim_player_deployment.json"))))
{
	ProjectName = GetProjectNameFromSpatial();
	SnapshotPath.FilePath = GetSnapshotPath();
	PrimaryLaunchConfigPath.FilePath = GetPrimaryLanchConfigPath();
}

FString USpatialGDKEditorCloudLauncherSettings::GetProjectNameFromSpatial() const
{
	FString FileContents;
	const FString SpatialOSFile = GetDefault<USpatialGDKEditorSettings>()->GetSpatialOSDirectory().Append(TEXT("/spatialos.json"));

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

	return TEXT("");
}

bool USpatialGDKEditorCloudLauncherSettings::IsAssemblyNameValid(const FString& Name)
{
	const FRegexPattern AssemblyPatternRegex(SpatialConstants::AssemblyPattern);
	FRegexMatcher RegMatcher(AssemblyPatternRegex, Name);

	return RegMatcher.FindNext();
}

bool USpatialGDKEditorCloudLauncherSettings::IsProjectNameValid(const FString& Name)
{
	const FRegexPattern ProjectPatternRegex(SpatialConstants::ProjectPattern);
	FRegexMatcher RegMatcher(ProjectPatternRegex, Name);

	return RegMatcher.FindNext();
}

bool USpatialGDKEditorCloudLauncherSettings::IsDeploymentNameValid(const FString& Name)
{
	const FRegexPattern DeploymentPatternRegex(SpatialConstants::DeploymentPattern);
	FRegexMatcher RegMatcher(DeploymentPatternRegex, Name);

	return RegMatcher.FindNext();
}

bool USpatialGDKEditorCloudLauncherSettings::IsRegionCodeValid(const ERegionCode::Type RegionCode)
{
	UEnum* pEnum = FindObject<UEnum>(ANY_PACKAGE, TEXT("ERegionCode"), true);

	return pEnum && pEnum->IsValidEnumValue(RegionCode);
}

void USpatialGDKEditorCloudLauncherSettings::SetPrimaryDeploymentName(const FString& Name)
{
	PrimaryDeploymentName = Name;
	SaveConfig();
}

void USpatialGDKEditorCloudLauncherSettings::SetAssemblyName(const FString& Name)
{
	AssemblyName = Name;
	SaveConfig();
}

void USpatialGDKEditorCloudLauncherSettings::SetProjectName(const FString& Name)
{
	ProjectName = Name;
	SaveConfig();
}

void USpatialGDKEditorCloudLauncherSettings::SetPrimaryLaunchConfigPath(const FString& Path)
{
	PrimaryLaunchConfigPath.FilePath = FPaths::ConvertRelativePathToFull(Path);
	SaveConfig();
}

void USpatialGDKEditorCloudLauncherSettings::SetSnapshotPath(const FString& Path)
{
	SnapshotPath.FilePath = FPaths::ConvertRelativePathToFull(Path);
	SaveConfig();
}

void USpatialGDKEditorCloudLauncherSettings::SetPrimaryRegionCode(const ERegionCode::Type RegionCode)
{
	PrimaryDeploymentRegionCode = RegionCode;
}

void USpatialGDKEditorCloudLauncherSettings::SetSimulatedPlayerRegionCode(const ERegionCode::Type RegionCode)
{
	SimulatedPlayerDeploymentRegionCode = RegionCode;
}

void USpatialGDKEditorCloudLauncherSettings::SetSimulatedPlayersEnabledState(bool IsEnabled)
{
	bSimulatedPlayersIsEnabled = IsEnabled;
	SaveConfig();
}

void USpatialGDKEditorCloudLauncherSettings::SetSimulatedPlayerDeploymentName(const FString& Name)
{
	SimulatedPlayerDeploymentName = Name;
	SaveConfig();
}

void USpatialGDKEditorCloudLauncherSettings::SetNumberOfSimulatedPlayers(uint32 Number)
{
	NumberOfSimulatedPlayers = Number;
	SaveConfig();
}

bool USpatialGDKEditorCloudLauncherSettings::IsDeploymentConfigurationValid() const
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
