// Copyright (c) Improbable Worlds Ltd, All Rights Reserved
#include "SpatialGDKEditorSettings.h"
#include "Settings/LevelEditorPlaySettings.h"
#include "ISettingsModule.h"
#include "MessageDialog.h"
#include "Modules/ModuleManager.h"

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

	SafetyCheckSpatialOSDirectoryPaths();
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
