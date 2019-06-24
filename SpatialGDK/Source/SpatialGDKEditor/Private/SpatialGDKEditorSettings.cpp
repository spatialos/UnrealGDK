// Copyright (c) Improbable Worlds Ltd, All Rights Reserved
#include "SpatialGDKEditorSettings.h"
#include "Settings/LevelEditorPlaySettings.h"
#include "SpatialGDKSettings.h"

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
	else if (Name == GET_MEMBER_NAME_CHECKED(USpatialGDKEditorSettings, LaunchConfigDesc))
	{
		SetRuntimeWorkerTypes();
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

}

void USpatialGDKEditorSettings::SetRuntimeWorkerTypes()
{
	TSet<FString> WorkerTypes;

	for (const FWorkerTypeLaunchSection WorkerLaunch : LaunchConfigDesc.Workers)
	{
		WorkerTypes.Add(WorkerLaunch.WorkerTypeName);
	}

	USpatialGDKSettings* RuntimeSettings = GetMutableDefault<USpatialGDKSettings>();
	if (RuntimeSettings != nullptr)
	{
		RuntimeSettings->WorkerTypes.Empty();
		RuntimeSettings->WorkerTypes.Append(WorkerTypes);
		//RuntimeSettings->SaveConfig(CPF_Config, *RuntimeSettings->GetDefaultConfigFilename());
	}
}
