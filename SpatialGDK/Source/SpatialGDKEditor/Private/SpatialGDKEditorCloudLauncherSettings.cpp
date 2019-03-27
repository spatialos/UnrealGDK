// Copyright (c) Improbable Worlds Ltd, All Rights Reserved
#include "SpatialGDKEditorCloudLauncherSettings.h"

#include "Dom/JsonObject.h"
#include "Internationalization/Regex.h"
#include "Misc/FileHelper.h"
#include "Serialization/JsonReader.h"
#include "Serialization/JsonSerializer.h"
#include "Settings/LevelEditorPlaySettings.h"
#include "SpatialGDKEditorSettings.h";
#include "Templates/SharedPointer.h";
#include "SpatialConstants.h";

USpatialGDKEditorCloudLauncherSettings::USpatialGDKEditorCloudLauncherSettings(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	const USpatialGDKEditorSettings* SpatialEditorSettings = GetDefault<USpatialGDKEditorSettings>();
	SnapshotPath.FilePath = FPaths::Combine(SpatialEditorSettings->GetSpatialOSSnapshotFolderPath(), SpatialEditorSettings->GetSpatialOSSnapshotFile());
	ProjectName = GetProjectNameFromSpatial();
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

bool USpatialGDKEditorCloudLauncherSettings::IsProjectNameValid(const FString & Name)
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

void USpatialGDKEditorCloudLauncherSettings::SetPrimaryDeploymentName(const FString & Name)
{
	PrimaryDeploymentName = Name;
}

void USpatialGDKEditorCloudLauncherSettings::SetAssemblyName(const FString & Name)
{
	AssemblyName = Name;
}

void USpatialGDKEditorCloudLauncherSettings::SetProjectName(const FString & Name)
{
	ProjectName = Name;
}

void USpatialGDKEditorCloudLauncherSettings::SetPrimaryLaunchConfigPath(const FString & Path)
{
	PrimaryLaunchConfigPath.FilePath = FPaths::ConvertRelativePathToFull(Path);
}

void USpatialGDKEditorCloudLauncherSettings::SetSnapshotPath(const FString & Path)
{
	SnapshotPath.FilePath = FPaths::ConvertRelativePathToFull(Path);
}

void USpatialGDKEditorCloudLauncherSettings::SetSimulatedPlayersEnabledState(bool IsEnabled)
{
	bSimulatedPlayersIsEnabled = IsEnabled;
}

void USpatialGDKEditorCloudLauncherSettings::SetSimulatedPlayerDeploymentName(const FString & Name)
{
	SimulatedPlayerDeploymentName = Name;
}

void USpatialGDKEditorCloudLauncherSettings::SetSimulatedPlayerLaunchConfigPath(const FString & Path)
{
	SimulatedPlayerLaunchConfigPath.FilePath = FPaths::ConvertRelativePathToFull(Path);
}

void USpatialGDKEditorCloudLauncherSettings::SetNumberOfSimulatedPlayers(uint32 Number)
{
	NumberOfSimulatedPlayers = Number;
}

bool USpatialGDKEditorCloudLauncherSettings::IsDeploymentConfigurationValid() const
{
	return IsAssemblyNameValid(AssemblyName) &&
		IsDeploymentNameValid(PrimaryDeploymentName) &&
		IsProjectNameValid(ProjectName) &&
		!SnapshotPath.FilePath.IsEmpty() &&
		!PrimaryLaunchConfigPath.FilePath.IsEmpty();
}
