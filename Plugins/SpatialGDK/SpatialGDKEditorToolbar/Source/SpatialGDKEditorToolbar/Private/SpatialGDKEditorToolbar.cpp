// Copyright (c) Improbable Worlds Ltd, All Rights Reserved
#include "SpatialGDKEditorToolbar.h"
#include "Async.h"
#include "EditorStyleSet.h"
#include "Framework/MultiBox/MultiBoxBuilder.h"
#include "ISettingsContainer.h"
#include "ISettingsModule.h"
#include "ISettingsSection.h"
#include "Misc/MessageDialog.h"
#include "NotificationManager.h"
#include "SNotificationList.h"
#include "SpatialGDKEditorGenerateSnapshot.h"
#include "SpatialGDKEditorInteropCodeGenerator.h"
#include "SpatialGDKEditorToolbarCommands.h"
#include "SpatialGDKEditorToolbarStyle.h"

#include "Editor/EditorEngine.h"
#include "HAL/FileManager.h"
#include "Sound/SoundBase.h"

#include "LevelEditor.h"

DEFINE_LOG_CATEGORY(LogSpatialGDKEditor);

#define LOCTEXT_NAMESPACE "FSpatialGDKEditorToolbarModule"

FSpatialGDKEditorToolbarModule::FSpatialGDKEditorToolbarModule()
{
}

void FSpatialGDKEditorToolbarModule::StartupModule()
{
	FSpatialGDKEditorToolbarStyle::Initialize();
	FSpatialGDKEditorToolbarStyle::ReloadTextures();

	FSpatialGDKEditorToolbarCommands::Register();

	PluginCommands = MakeShareable(new FUICommandList);
	MapActions(PluginCommands);
	SetupToolbar(PluginCommands);

	RegisterSettings();

	// load sounds
	ExecutionStartSound = LoadObject<USoundBase>(NULL, TEXT("/Engine/EditorSounds/Notifications/CompileStart_Cue.CompileStart_Cue"));
	ExecutionStartSound->AddToRoot();
	ExecutionSuccessSound = LoadObject<USoundBase>(NULL, TEXT("/Engine/EditorSounds/Notifications/CompileSuccess_Cue.CompileSuccess_Cue"));
	ExecutionSuccessSound->AddToRoot();
	ExecutionFailSound = LoadObject<USoundBase>(NULL, TEXT("/Engine/EditorSounds/Notifications/CompileFailed_Cue.CompileFailed_Cue"));
	ExecutionFailSound->AddToRoot();
}

void FSpatialGDKEditorToolbarModule::ShutdownModule()
{

	if (ExecutionStartSound != nullptr)
	{
		if (!GExitPurge)
		{
			ExecutionStartSound->RemoveFromRoot();
		}
		ExecutionStartSound = nullptr;
	}

	if (ExecutionSuccessSound != nullptr)
	{
		if (!GExitPurge)
		{
			ExecutionSuccessSound->RemoveFromRoot();
		}
		ExecutionSuccessSound = nullptr;
	}

	if (ExecutionFailSound != NULL)
	{
		if (!GExitPurge)
		{
			ExecutionFailSound->RemoveFromRoot();
		}
		ExecutionFailSound = NULL;
	}

	if (UObjectInitialized())
	{
		UnregisterSettings();
	}

	FSpatialGDKEditorToolbarStyle::Shutdown();
	FSpatialGDKEditorToolbarCommands::Unregister();
}

void FSpatialGDKEditorToolbarModule::PreUnloadCallback()
{
}

void FSpatialGDKEditorToolbarModule::Tick(float DeltaTime)
{
}

void FSpatialGDKEditorToolbarModule::RegisterSettings()
{
}

void FSpatialGDKEditorToolbarModule::UnregisterSettings()
{
	if (ISettingsModule* SettingsModule = FModuleManager::GetModulePtr<ISettingsModule>("Settings"))
	{
		SettingsModule->UnregisterSettings("Project", "SpatialGDK", "Toolbar");
	}
}

bool FSpatialGDKEditorToolbarModule::HandleSettingsSaved()
{
	return true;
}

void FSpatialGDKEditorToolbarModule::MapActions(TSharedPtr<class FUICommandList> PluginCommands)
{
	PluginCommands->MapAction(
		FSpatialGDKEditorToolbarCommands::Get().CreateSpatialGDKSnapshot,
		FExecuteAction::CreateRaw(
			this, &FSpatialGDKEditorToolbarModule::CreateSnapshotButtonClicked),
		FCanExecuteAction());

	PluginCommands->MapAction(
		FSpatialGDKEditorToolbarCommands::Get().GenerateInteropCode,
		FExecuteAction::CreateRaw(
			this, &FSpatialGDKEditorToolbarModule::GenerateInteropCodeButtonClicked),
		FCanExecuteAction());
}

void FSpatialGDKEditorToolbarModule::SetupToolbar(TSharedPtr<class FUICommandList> PluginCommands)
{
	FLevelEditorModule& LevelEditorModule =
		FModuleManager::LoadModuleChecked<FLevelEditorModule>("LevelEditor");

	{
		TSharedPtr<FExtender> MenuExtender = MakeShareable(new FExtender());
		MenuExtender->AddMenuExtension(
			"General", EExtensionHook::After, PluginCommands,
			FMenuExtensionDelegate::CreateRaw(this, &FSpatialGDKEditorToolbarModule::AddMenuExtension));

		LevelEditorModule.GetMenuExtensibilityManager()->AddExtender(MenuExtender);
	}

	{
		TSharedPtr<FExtender> ToolbarExtender = MakeShareable(new FExtender);
		ToolbarExtender->AddToolBarExtension(
			"Game", EExtensionHook::After, PluginCommands,
			FToolBarExtensionDelegate::CreateRaw(this,
				&FSpatialGDKEditorToolbarModule::AddToolbarExtension));

		LevelEditorModule.GetToolBarExtensibilityManager()->AddExtender(ToolbarExtender);
	}
}

void FSpatialGDKEditorToolbarModule::AddMenuExtension(FMenuBuilder& Builder)
{
	Builder.BeginSection("SpatialGDK", LOCTEXT("SpatialGDK", "SpatialGDK"));
	{
		Builder.AddMenuEntry(FSpatialGDKEditorToolbarCommands::Get().CreateSpatialGDKSnapshot);
		Builder.AddMenuEntry(FSpatialGDKEditorToolbarCommands::Get().GenerateInteropCode);
	}
	Builder.EndSection();
}

void FSpatialGDKEditorToolbarModule::AddToolbarExtension(FToolBarBuilder& Builder)
{
	Builder.AddSeparator(NAME_None);
	Builder.AddToolBarButton(FSpatialGDKEditorToolbarCommands::Get().CreateSpatialGDKSnapshot);
	Builder.AddToolBarButton(FSpatialGDKEditorToolbarCommands::Get().GenerateInteropCode);
}

void FSpatialGDKEditorToolbarModule::CreateSnapshotButtonClicked()
{
	if (GEditor)
	{
		GEditor->PlayEditorSound(ExecutionStartSound);
	}

	FString ProjectFilePath = IFileManager::Get().ConvertToAbsolutePathForExternalAppForRead(*FPaths::GetPath(FPaths::GetProjectFilePath()));
	FString CombinedPath = FPaths::Combine(*ProjectFilePath, TEXT("../spatial/snapshots"));
	UE_LOG(LogTemp, Display, TEXT("Combined path %s"), *CombinedPath);
	if (FPaths::CollapseRelativeDirectories(CombinedPath))
	{
		SpatialGDKGenerateSnapshot(CombinedPath, GEditor->GetEditorWorldContext().World());
	}
	else
	{
		UE_LOG(LogTemp, Display, TEXT("Path was invalid - snapshot not generated"));
	}
}

void FSpatialGDKEditorToolbarModule::GenerateInteropCodeButtonClicked()
{
	if (TaskNotificationPtr.IsValid())
	{
		TaskNotificationPtr.Pin()->ExpireAndFadeout();
	}

	if (GEditor)
	{
		GEditor->PlayEditorSound(ExecutionStartSound);
	}

	FNotificationInfo Info(FText::AsCultureInvariant("Started interop codegen"));
	Info.Image = FEditorStyle::GetBrush(TEXT("LevelEditor.RecompileGameCode"));
	Info.ExpireDuration = 5.0f;
	Info.bFireAndForget = false;

	TaskNotificationPtr = FSlateNotificationManager::Get().AddNotification(Info);

	if (TaskNotificationPtr.IsValid())
	{
		TaskNotificationPtr.Pin()->SetCompletionState(SNotificationItem::CS_Pending);
	}

	SpatialGDKGenerateInteropCode();

	ShowSuccessNotification("Interop Codegen Complete!");
}

void FSpatialGDKEditorToolbarModule::ShowSuccessNotification(const FString& NotificationText)
{
	FNotificationInfo Info(FText::AsCultureInvariant(NotificationText));
	Info.Image = FEditorStyle::GetBrush(TEXT("LevelEditor.RecompileGameCode"));
	Info.FadeInDuration = 0.1f;
	Info.FadeOutDuration = 0.5f;
	Info.ExpireDuration = 1.5f;
	Info.bUseThrobber = false;
	Info.bUseSuccessFailIcons = true;
	Info.bUseLargeFont = true;
	Info.bFireAndForget = false;
	Info.bAllowThrottleWhenFrameRateIsLow = false;
	auto NotificationItem = FSlateNotificationManager::Get().AddNotification(Info);
	NotificationItem->SetCompletionState(SNotificationItem::CS_Success);
	NotificationItem->ExpireAndFadeout();

	if (GEditor)
	{
		GEditor->PlayEditorSound(ExecutionSuccessSound);
	}
}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FSpatialGDKEditorToolbarModule, SpatialGDKEditorToolbar)
