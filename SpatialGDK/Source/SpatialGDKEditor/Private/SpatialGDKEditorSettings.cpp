// Copyright (c) Improbable Worlds Ltd, All Rights Reserved
#include "SpatialGDKEditorSettings.h"
#include "Settings/LevelEditorPlaySettings.h"

USpatialGDKEditorSettings::USpatialGDKEditorSettings(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
	, bDeleteDynamicEntities(true)
	, bGenerateDefaultLaunchConfig(true)
	, bStopSpatialOnExit(false)
	, bGeneratePlaceholderEntitiesInSnapshot(true)
{
	SpatialOSDirectory.Path = GetSpatialOSDirectory();
	SpatialOSLaunchConfig.FilePath = GetSpatialOSLaunchConfig();
	SpatialOSSnapshotPath.Path = GetSpatialOSSnapshotFolderPath();
	SpatialOSSnapshotFile = GetSpatialOSSnapshotFile();
	GeneratedSchemaOutputFolder.Path = GetGeneratedSchemaOutputFolder();
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
}

void USpatialGDKEditorSettings::PostInitProperties()
{
	Super::PostInitProperties();

	ULevelEditorPlaySettings* PlayInSettings = GetMutableDefault<ULevelEditorPlaySettings>();
	PlayInSettings->SetDeleteDynamicEntities(bDeleteDynamicEntities);

	PlayInSettings->PostEditChange();
	PlayInSettings->SaveConfig();
}

FString USpatialGDKEditorSettings::ToString()
{
	TArray<FStringFormatArg> Args;
	Args.Add(SpatialOSDirectory.Path);
	Args.Add(bDeleteDynamicEntities);
	Args.Add(bGenerateDefaultLaunchConfig);
	Args.Add(SpatialOSLaunchConfig.FilePath);
	Args.Add(bStopSpatialOnExit);
	Args.Add(SpatialOSSnapshotPath.Path);
	Args.Add(SpatialOSSnapshotFile);
	Args.Add(GeneratedSchemaOutputFolder.Path);
	Args.Add(bGeneratePlaceholderEntitiesInSnapshot);

	return FString::Format(TEXT(
		"ProjectRootFolder={0}, "
		"bDeleteDynamicEntities={1}"
		"bGenerateDefaultLaunchConfig={2}"
		"SpatialOSLaunchArgument={3}, "
		"bStopSpatialOnExit={4}, "
		"SpatialOSSnapshotPath={5}, "
		"SpatialOSSnapshotFile={6}, "
		"GeneratedSchemaOutputFolder={7}"
		"bGeneratePlaceholderEntitiesInSnapshot={8}")
		, Args);
}

