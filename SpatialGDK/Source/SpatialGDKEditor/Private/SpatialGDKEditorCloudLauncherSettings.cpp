// Copyright (c) Improbable Worlds Ltd, All Rights Reserved
#include "SpatialGDKEditorCloudLauncherSettings.h"

#include "Dom/JsonObject.h"
#include "Internationalization/Regex.h"
#include "Misc/FileHelper.h"
#include "Serialization/JsonReader.h"
#include "Serialization/JsonSerializer.h"
#include "Settings/LevelEditorPlaySettings.h"
#include "SpatialGDKEditorSettings.h";
#include "Templates/SharedPointer.h"

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

void USpatialGDKEditorCloudLauncherSettings::ValidateAssemblyName()
{
	const FRegexPattern AssemblyPattern(TEXT("^[a-zA-Z0-9_.-]{5,64}$"));
	FRegexMatcher RegMatcher(AssemblyPattern, AssemblyName);

	bAssemblyNameIsValid = RegMatcher.FindNext();
}

void USpatialGDKEditorCloudLauncherSettings::ValidateProjectName()
{
	const FRegexPattern ProjectPattern(TEXT("^[a-z0-9_]{3,32}$"));
	FRegexMatcher RegMatcher(ProjectPattern, ProjectName);

	bProjectNameIsValid = RegMatcher.FindNext();
}

void USpatialGDKEditorCloudLauncherSettings::ValidateDeploymentName()
{
	const FRegexPattern DeploymentPattern(TEXT("^[a-z0-9_]{2,32}$"));
	FRegexMatcher RegMatcher(DeploymentPattern, PrimaryDeploymentName);

	bPrimaryDeploymentNameIsValid = RegMatcher.FindNext();
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
	SimulatedPlayersIsEnabled = IsEnabled;
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

bool USpatialGDKEditorCloudLauncherSettings::IsDeploymentConfigurationValidSinceLastCheck() const
{
	return bProjectNameIsValid &&
		bAssemblyNameIsValid &&
		bPrimaryDeploymentNameIsValid &&
		!PrimaryLaunchConfigPath.FilePath.IsEmpty() &&
		!SnapshotPath.FilePath.IsEmpty();
}

bool USpatialGDKEditorCloudLauncherSettings::IsDeploymentConfigurationValidWithCheck()
{
	ValidateAssemblyName();
	ValidateDeploymentName();
	ValidateProjectName();

	return IsDeploymentConfigurationValidSinceLastCheck();
}
