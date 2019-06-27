// Copyright (c) Improbable Worlds Ltd, All Rights Reserved
#include "SpatialGDKEditorSettings.h"
#include "ISettingsModule.h"
#include "Misc/MessageDialog.h"
#include "Modules/ModuleManager.h"
#include "Settings/LevelEditorPlaySettings.h"
#include "SpatialGDKSettings.h"


USpatialGDKEditorSettings::USpatialGDKEditorSettings(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
	, bDeleteDynamicEntities(true)
	, bGenerateDefaultLaunchConfig(true)
	, bStopSpatialOnExit(false)
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
	SafetyCheckSpatialOSDirectoryPaths();
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

void USpatialGDKEditorSettings::SafetyCheckSpatialOSDirectoryPaths()
{
	const FString Path = FPaths::ConvertRelativePathToFull(FPaths::Combine(FPaths::ProjectDir(), TEXT("/../spatial/")));

	FString DisplayMessage = TEXT("The following directory paths are invalid in SpatialOS GDK for Unreal - Editor Settings:\n\n");

	bool bFoundInvalidPath = false;

	if (!FPaths::DirectoryExists(SpatialOSDirectory.Path))
	{
		SpatialOSDirectory.Path = Path;
		DisplayMessage += TEXT("SpatialOS directory\n");
		bFoundInvalidPath = true;
	}

	if (!FPaths::DirectoryExists(SpatialOSSnapshotPath.Path))
	{
		SpatialOSSnapshotPath.Path = FPaths::ConvertRelativePathToFull(FPaths::Combine(Path, TEXT("snapshots/")));
		DisplayMessage += TEXT("Snapshot path\n");
		bFoundInvalidPath = true;
	}

	if (!FPaths::DirectoryExists(GeneratedSchemaOutputFolder.Path))
	{
		GeneratedSchemaOutputFolder.Path = FPaths::ConvertRelativePathToFull(FPaths::Combine(Path, TEXT("schema/unreal/generated/")));
		DisplayMessage += TEXT("Output path for the generated schemas\n");
		bFoundInvalidPath = true;
	}

	if (bFoundInvalidPath)
	{
		DisplayMessage += TEXT("\nDefaults for these will now be set\n");
		FMessageDialog::Open(EAppMsgType::Ok, FText::FromString(DisplayMessage));
		FModuleManager::LoadModuleChecked<ISettingsModule>("Settings").ShowViewer("Project", "SpatialGDKEditor", "Editor Settings");

		PostEditChange();
		SaveConfig(CPF_Config, *GetDefaultConfigFilename());
	}
}

void USpatialGDKEditorSettings::SetLevelEditorPlaySettingsWorkerTypes()
{
	ULevelEditorPlaySettings* PlayInSettings = GetMutableDefault<ULevelEditorPlaySettings>();

	for (const FWorkerTypeLaunchSection WorkerLaunch : LaunchConfigDesc.Workers)
	{
		PlayInSettings->WorkerTypesToLaunch.Add(WorkerLaunch.WorkerTypeName, WorkerLaunch.NumEditorInstances);
	}
}
