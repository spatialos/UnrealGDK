// Copyright (c) Improbable Worlds Ltd, All Rights Reserved
#include "NUFEditorToolbar.h"
#include "Async.h"
#include "EditorStyleSet.h"
#include "Framework/MultiBox/MultiBoxBuilder.h"
#include "ISettingsContainer.h"
#include "ISettingsModule.h"
#include "ISettingsSection.h"
#include "Misc/MessageDialog.h"
#include "NotificationManager.h"
#include "SNotificationList.h"
#include "NUFEditorToolbarCommands.h"
#include "NUFEditorToolbarStyle.h"
#include "NUFEditorGenerateSnapshot.h"

#include "HAL/FileManager.h"
#include "Editor/EditorEngine.h"

#include "LevelEditor.h"

DEFINE_LOG_CATEGORY(LogNUFEditor);

#define LOCTEXT_NAMESPACE "FNUFEditorToolbarModule"

FNUFEditorToolbarModule::FNUFEditorToolbarModule()
{
}

void FNUFEditorToolbarModule::StartupModule()
{
	FNUFEditorToolbarStyle::Initialize();
	FNUFEditorToolbarStyle::ReloadTextures();

	FNUFEditorToolbarCommands::Register();

	PluginCommands = MakeShareable(new FUICommandList);
	MapActions(PluginCommands);
	SetupToolbar(PluginCommands);

	RegisterSettings();
}

void FNUFEditorToolbarModule::ShutdownModule()
{
	if (UObjectInitialized())
	{
		UnregisterSettings();
	}

	FNUFEditorToolbarStyle::Shutdown();
	FNUFEditorToolbarCommands::Unregister();
}

void FNUFEditorToolbarModule::PreUnloadCallback()
{
}

void FNUFEditorToolbarModule::Tick(float DeltaTime)
{
}

void FNUFEditorToolbarModule::RegisterSettings()
{
}

void FNUFEditorToolbarModule::UnregisterSettings()
{
	if (ISettingsModule* SettingsModule = FModuleManager::GetModulePtr<ISettingsModule>("Settings"))
	{
		SettingsModule->UnregisterSettings("Project", "NUF", "Toolbar");
	}
}

bool FNUFEditorToolbarModule::HandleSettingsSaved()
{
	return true;
}

void FNUFEditorToolbarModule::MapActions(TSharedPtr<class FUICommandList> PluginCommands)
{
	PluginCommands->MapAction(
		FNUFEditorToolbarCommands::Get().CreateNUFSnapshot,
		FExecuteAction::CreateRaw(
			this, &FNUFEditorToolbarModule::CreateSnapshotButtonClicked),
		FCanExecuteAction());
}

void FNUFEditorToolbarModule::SetupToolbar(TSharedPtr<class FUICommandList> PluginCommands)
{
	FLevelEditorModule& LevelEditorModule =
		FModuleManager::LoadModuleChecked<FLevelEditorModule>("LevelEditor");

	{
		TSharedPtr<FExtender> MenuExtender = MakeShareable(new FExtender());
		MenuExtender->AddMenuExtension(
			"General", EExtensionHook::After, PluginCommands,
			FMenuExtensionDelegate::CreateRaw(this, &FNUFEditorToolbarModule::AddMenuExtension));

		LevelEditorModule.GetMenuExtensibilityManager()->AddExtender(MenuExtender);
	}

	{
		TSharedPtr<FExtender> ToolbarExtender = MakeShareable(new FExtender);
		ToolbarExtender->AddToolBarExtension(
			"Game", EExtensionHook::After, PluginCommands,
			FToolBarExtensionDelegate::CreateRaw(this,
				&FNUFEditorToolbarModule::AddToolbarExtension));

		LevelEditorModule.GetToolBarExtensibilityManager()->AddExtender(ToolbarExtender);
	}
}

void FNUFEditorToolbarModule::AddMenuExtension(FMenuBuilder& Builder)
{
	Builder.BeginSection("NUF", LOCTEXT("NUF", "NUF"));
	{
		Builder.AddMenuEntry(FNUFEditorToolbarCommands::Get().CreateNUFSnapshot);
	}
	Builder.EndSection();
}

void FNUFEditorToolbarModule::AddToolbarExtension(FToolBarBuilder& Builder)
{
	Builder.AddSeparator(NAME_None);
	Builder.AddToolBarButton(FNUFEditorToolbarCommands::Get().CreateNUFSnapshot);
}

void FNUFEditorToolbarModule::CreateSnapshotButtonClicked()
{
	FString ProjectFilePath = IFileManager::Get().ConvertToAbsolutePathForExternalAppForRead(*FPaths::GetPath(FPaths::GetProjectFilePath()));
	FString CombinedPath = FPaths::Combine(*ProjectFilePath, TEXT("../../../snapshots"));
	UE_LOG(LogTemp, Display, TEXT("Combined path %s"), *CombinedPath);
	if (FPaths::CollapseRelativeDirectories(CombinedPath))
	{
		NUFGenerateSnapshot(CombinedPath, GEditor->GetEditorWorldContext().World());
	}
	else
	{
		UE_LOG(LogTemp, Display, TEXT("Path was invalid - snapshot not generated"));
	}
}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FNUFEditorToolbarModule, NUFEditorToolbar)