// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "SpatialGDKEditorSettings.h"

#include "ISettingsModule.h"
#include "Misc/MessageDialog.h"
#include "Modules/ModuleManager.h"
#include "Settings/LevelEditorPlaySettings.h"
#include "SpatialGDKSettings.h"


USpatialGDKEditorSettings::USpatialGDKEditorSettings(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
	, bShowSpatialServiceButton(false)
	, bDeleteDynamicEntities(true)
	, bGenerateDefaultLaunchConfig(true)
	, bStopSpatialOnExit(false)
{
	SpatialOSLaunchConfig.FilePath = GetSpatialOSLaunchConfig();
	SpatialOSSnapshotPath.Path = GetSpatialOSSnapshotFolderPath();
	SpatialOSSnapshotFile = GetSpatialOSSnapshotFile();
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
