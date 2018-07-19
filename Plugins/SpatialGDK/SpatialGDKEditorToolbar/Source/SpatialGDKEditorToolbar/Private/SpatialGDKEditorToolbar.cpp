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
#include "SpatialGDKEditorToolbarSettings.h"
#include "SpatialGDKEditorToolbarStyle.h"

#include "Editor/EditorEngine.h"
#include "HAL/FileManager.h"
#include "Sound/SoundBase.h"

#include "LevelEditor.h"

DEFINE_LOG_CATEGORY(LogSpatialGDKEditor);

#define LOCTEXT_NAMESPACE "FSpatialGDKEditorToolbarModule"

FSpatialGDKEditorToolbarModule::FSpatialGDKEditorToolbarModule()
: SpatialOSStackProcessID(0), bStopSpatialOnExit(false)
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
	CheckForRunningStack();

	// load sounds
	ExecutionStartSound = LoadObject<USoundBase>(nullptr, TEXT("/Engine/EditorSounds/Notifications/CompileStart_Cue.CompileStart_Cue"));
	ExecutionStartSound->AddToRoot();
	ExecutionSuccessSound = LoadObject<USoundBase>(nullptr, TEXT("/Engine/EditorSounds/Notifications/CompileSuccess_Cue.CompileSuccess_Cue"));
	ExecutionSuccessSound->AddToRoot();
	ExecutionFailSound = LoadObject<USoundBase>(nullptr, TEXT("/Engine/EditorSounds/Notifications/CompileFailed_Cue.CompileFailed_Cue"));
	ExecutionFailSound->AddToRoot();
	InteropCodeGenRunning = false;

	OnPropertyChangedDelegateHandle = FCoreUObjectDelegates::OnObjectPropertyChanged.AddRaw(
	  this, &FSpatialGDKEditorToolbarModule::OnPropertyChanged);

	auto SpatialGDKToolbarSettings = GetDefault<USpatialGDKEditorToolbarSettings>();
	bStopSpatialOnExit = SpatialGDKToolbarSettings->bStopSpatialOnExit;
}

void FSpatialGDKEditorToolbarModule::ShutdownModule()
{
	FCoreUObjectDelegates::OnObjectPropertyChanged.Remove(OnPropertyChangedDelegateHandle);

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

	if (ExecutionFailSound != nullptr)
	{
		if (!GExitPurge)
		{
			ExecutionFailSound->RemoveFromRoot();
		}
		ExecutionFailSound = nullptr;
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
	if (bStopSpatialOnExit)
	{
		StopRunningStack();
	}
}

void FSpatialGDKEditorToolbarModule::Tick(float DeltaTime)
{
	if (SpatialOSStackProcessID != 0 &&
		!FPlatformProcess::IsApplicationRunning(SpatialOSStackProcessID))
	{
		FPlatformProcess::CloseProc(SpatialOSStackProcHandle);
		SpatialOSStackProcessID = 0;
	}	
}

void FSpatialGDKEditorToolbarModule::RegisterSettings()
{
	if (ISettingsModule* SettingsModule = FModuleManager::GetModulePtr<ISettingsModule>("Settings"))
	{
		ISettingsContainerPtr SettingsContainer = SettingsModule->GetContainer("Project");

		SettingsContainer->DescribeCategory(
			"SpatialOSEditorToolbar", LOCTEXT("RuntimeWDCategoryName", "SpatialOS - Toolbar"),
			LOCTEXT("RuntimeWDCategoryDescription",
					"Configuration for the SpatialOS Editor toolbar plugin"));

		ISettingsSectionPtr SettingsSection = SettingsModule->RegisterSettings(
			"Project", "SpatialOS", "Toolbar", LOCTEXT("RuntimeGeneralSettingsName", "Toolbar"),
			LOCTEXT("RuntimeGeneralSettingsDescription",
					"Configuration for SpatialOS Editor toolbar plugin."),
			GetMutableDefault<USpatialGDKEditorToolbarSettings>());

		if (SettingsSection.IsValid())
		{
			SettingsSection->OnModified().BindRaw(this, &FSpatialGDKEditorToolbarModule::HandleSettingsSaved);
		}
	}
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
	USpatialGDKEditorToolbarSettings* Settings = GetMutableDefault<USpatialGDKEditorToolbarSettings>();
	Settings->SaveConfig();

	return true;
}

bool FSpatialGDKEditorToolbarModule::CanExecuteInteropCodeGen()
{
	return !InteropCodeGenRunning;
}

void FSpatialGDKEditorToolbarModule::MapActions(TSharedPtr<class FUICommandList> PluginCommands)
{
	PluginCommands->MapAction(
		FSpatialGDKEditorToolbarCommands::Get().CreateSpatialGDKSnapshot,
		FExecuteAction::CreateRaw(this, &FSpatialGDKEditorToolbarModule::CreateSnapshotButtonClicked),
		FCanExecuteAction());

	PluginCommands->MapAction(
		FSpatialGDKEditorToolbarCommands::Get().GenerateInteropCode,
		FExecuteAction::CreateRaw(this, &FSpatialGDKEditorToolbarModule::GenerateInteropCodeButtonClicked),
		FCanExecuteAction::CreateRaw(this, &FSpatialGDKEditorToolbarModule::CanExecuteInteropCodeGen));
	
	PluginCommands->MapAction(
		FSpatialGDKEditorToolbarCommands::Get().StartSpatialOSStackAction,
		FExecuteAction::CreateRaw(this, &FSpatialGDKEditorToolbarModule::StartSpatialOSButtonClicked),
		FCanExecuteAction::CreateRaw(this, &FSpatialGDKEditorToolbarModule::StartSpatialOSStackCanExecute));

	PluginCommands->MapAction(
		FSpatialGDKEditorToolbarCommands::Get().StopSpatialOSStackAction,
		FExecuteAction::CreateRaw(this, &FSpatialGDKEditorToolbarModule::StopSpatialOSButtonClicked),
		FCanExecuteAction::CreateRaw(this, &FSpatialGDKEditorToolbarModule::StopSpatialOSStackCanExecute));

	PluginCommands->MapAction(
		FSpatialGDKEditorToolbarCommands::Get().LaunchInspectorWebPageAction,
		FExecuteAction::CreateRaw(this, &FSpatialGDKEditorToolbarModule::LaunchInspectorWebpageButtonClicked),
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
	Builder.BeginSection("SpatialOS", LOCTEXT("SpatialOS", "SpatialOS"));
	{
		Builder.AddMenuEntry(FSpatialGDKEditorToolbarCommands::Get().CreateSpatialGDKSnapshot);
		Builder.AddMenuEntry(FSpatialGDKEditorToolbarCommands::Get().GenerateInteropCode);
		Builder.AddMenuEntry(FSpatialGDKEditorToolbarCommands::Get().LaunchInspectorWebPageAction);
	}
	Builder.EndSection();
}

void FSpatialGDKEditorToolbarModule::AddToolbarExtension(FToolBarBuilder& Builder)
{
	Builder.AddSeparator(NAME_None);
	Builder.AddToolBarButton(FSpatialGDKEditorToolbarCommands::Get().CreateSpatialGDKSnapshot);
	Builder.AddToolBarButton(FSpatialGDKEditorToolbarCommands::Get().GenerateInteropCode);
	Builder.AddToolBarButton(FSpatialGDKEditorToolbarCommands::Get().StartSpatialOSStackAction);
	Builder.AddToolBarButton(FSpatialGDKEditorToolbarCommands::Get().StopSpatialOSStackAction);
	Builder.AddToolBarButton(FSpatialGDKEditorToolbarCommands::Get().LaunchInspectorWebPageAction);
}

void FSpatialGDKEditorToolbarModule::CreateSnapshotButtonClicked()
{
	ShowTaskStartNotification("Started snapshot generation");

	FString ProjectFilePath = IFileManager::Get().ConvertToAbsolutePathForExternalAppForRead(*FPaths::GetPath(FPaths::GetProjectFilePath()));
	FString CombinedPath = FPaths::Combine(*ProjectFilePath, TEXT("../spatial/snapshots"));
	const bool bSuccess = SpatialGDKGenerateSnapshot(CombinedPath, GEditor->GetEditorWorldContext().World());

	if(bSuccess)
	{
		ShowSuccessNotification("Snapshot successfully generated!");
	}
	else
	{
		ShowFailedNotification("Snapshot generation failed!");
	}
}

void FSpatialGDKEditorToolbarModule::GenerateInteropCodeButtonClicked()
{
	ShowTaskStartNotification("Generating Interop Code");
	InteropCodeGenRunning = true;

	AsyncTask(ENamedThreads::AnyHiPriThreadHiPriTask, [this] {
		bool bSuccess = SpatialGDKGenerateInteropCode();

		if (bSuccess)
		{
			ShowSuccessNotification("Interop Codegen Completed!");
		}
		else
		{
			ShowFailedNotification("Interop Codegen Failed");
		}
	});
}

void FSpatialGDKEditorToolbarModule::ShowTaskStartNotification(const FString& NotificationText)
{
	if (TaskNotificationPtr.IsValid())
	{
		TaskNotificationPtr.Pin()->ExpireAndFadeout();
	}

	if (GEditor && ExecutionStartSound)
	{
		GEditor->PlayEditorSound(ExecutionStartSound);
	}

	FNotificationInfo Info(FText::AsCultureInvariant(NotificationText));
	Info.Image = FEditorStyle::GetBrush(TEXT("LevelEditor.RecompileGameCode"));
	Info.ExpireDuration = 5.0f;
	Info.bFireAndForget = false;

	TaskNotificationPtr = FSlateNotificationManager::Get().AddNotification(Info);

	if (TaskNotificationPtr.IsValid())
	{
		TaskNotificationPtr.Pin()->SetCompletionState(SNotificationItem::CS_Pending);
	}
}

void FSpatialGDKEditorToolbarModule::ShowSuccessNotification(const FString& NotificationText)
{
	AsyncTask(ENamedThreads::GameThread, [this, NotificationText]{
		TSharedPtr<SNotificationItem> Notification = TaskNotificationPtr.Pin();
		Notification->SetFadeInDuration(0.1f);
		Notification->SetFadeOutDuration(0.5f);
		Notification->SetExpireDuration(7.5f);
		Notification->SetText(FText::AsCultureInvariant(NotificationText));
		Notification->SetCompletionState(SNotificationItem::CS_Success);
		Notification->ExpireAndFadeout();
		TaskNotificationPtr.Reset();

		if (GEditor && ExecutionSuccessSound)
		{
			GEditor->PlayEditorSound(ExecutionSuccessSound);
		}

		InteropCodeGenRunning = false;
	});
}

void FSpatialGDKEditorToolbarModule::ShowFailedNotification(const FString& NotificationText)
{
	AsyncTask(ENamedThreads::GameThread, [this, NotificationText]{
		TSharedPtr<SNotificationItem> Notification = TaskNotificationPtr.Pin();
		Notification->SetText(FText::AsCultureInvariant(NotificationText));
		Notification->SetCompletionState(SNotificationItem::CS_Fail);
		Notification->SetExpireDuration(5.0f);
		
		Notification->ExpireAndFadeout();

		if (GEditor && ExecutionFailSound)
		{
			GEditor->PlayEditorSound(ExecutionFailSound);
		}
	});
}

void FSpatialGDKEditorToolbarModule::StartSpatialOSButtonClicked()
{
	auto SpatialGDKToolbarSettings = GetDefault<USpatialGDKEditorToolbarSettings>();

	const FString ExecuteAbsolutePath =
		FPaths::ConvertRelativePathToFull(SpatialGDKToolbarSettings->ProjectRootFolder.Path);
	const FString CmdExecutable = TEXT("cmd.exe");
	const FString SpatialCmdArgument = FString::Printf(
		TEXT("/c spatial.exe local launch %s"), *SpatialGDKToolbarSettings->SpatialOSLaunchArgument);

	UE_LOG(LogSpatialGDKEditor, Log, TEXT("Starting cmd.exe with `%s` arguments."),
		 *SpatialCmdArgument);
	// Temporary workaround to get spatial.exe to properly show a window we have to call cmd.exe to
	// execute it.
	// We currently can't use pipes to capture output as it doesn't work properly with current
	// spatial.exe.
	SpatialOSStackProcHandle = FPlatformProcess::CreateProc(
		*(CmdExecutable), *SpatialCmdArgument, true, false, false, &SpatialOSStackProcessID, 0,
		*ExecuteAbsolutePath, nullptr, nullptr);

	FNotificationInfo Info(SpatialOSStackProcHandle.IsValid() == true
							 ? FText::FromString(TEXT("SpatialOS Starting..."))
							 : FText::FromString(TEXT("Failed to start SpatialOS")));
	Info.ExpireDuration = 3.0f;
	Info.bUseSuccessFailIcons = true;
	auto NotificationItem = FSlateNotificationManager::Get().AddNotification(Info);

	if (!SpatialOSStackProcHandle.IsValid())
	{
	NotificationItem->SetCompletionState(SNotificationItem::CS_Fail);
	const auto LogPath =
		SpatialGDKToolbarSettings->ProjectRootFolder.Path + FString(TEXT("/logs/spatial.log"));
	UE_LOG(LogSpatialGDKEditor, Error,
			 TEXT("Failed to start SpatialOS, please refer to log file `%s` for more information."),
			 *LogPath);
	}
	else
	{
	NotificationItem->SetCompletionState(SNotificationItem::CS_Success);
	}

	NotificationItem->ExpireAndFadeout();
}

void FSpatialGDKEditorToolbarModule::StopSpatialOSButtonClicked()
{
	StopRunningStack();
}

void FSpatialGDKEditorToolbarModule::StopRunningStack()
{
	if (SpatialOSStackProcHandle.IsValid())
	{
	if (FPlatformProcess::IsProcRunning(SpatialOSStackProcHandle))
	{
		FPlatformProcess::TerminateProc(SpatialOSStackProcHandle, true);
	}
	FPlatformProcess::CloseProc(SpatialOSStackProcHandle);
	SpatialOSStackProcessID = 0;
	}
}

void FSpatialGDKEditorToolbarModule::LaunchInspectorWebpageButtonClicked()
{
	FString WebError;
	FPlatformProcess::LaunchURL(TEXT("http://localhost:21000/inspector"), TEXT(""), &WebError);
	if (!WebError.IsEmpty())
	{
	FNotificationInfo Info(FText::FromString(WebError));
	Info.ExpireDuration = 3.0f;
	Info.bUseSuccessFailIcons = true;
	auto NotificationItem = FSlateNotificationManager::Get().AddNotification(Info);
	NotificationItem->SetCompletionState(SNotificationItem::CS_Fail);
	NotificationItem->ExpireAndFadeout();
	}
}

bool FSpatialGDKEditorToolbarModule::StartSpatialOSStackCanExecute() const
{
	return !SpatialOSStackProcHandle.IsValid() &&
		!FPlatformProcess::IsApplicationRunning(SpatialOSStackProcessID);
}

bool FSpatialGDKEditorToolbarModule::StopSpatialOSStackCanExecute() const
{
	return SpatialOSStackProcHandle.IsValid();
}

void FSpatialGDKEditorToolbarModule::CheckForRunningStack()
{
	FPlatformProcess::FProcEnumerator ProcEnumerator;
	do
	{
	auto Proc = ProcEnumerator.GetCurrent();
	const auto ProcName = Proc.GetName();
	if (ProcName.Compare(TEXT("spatial.exe"), ESearchCase::IgnoreCase) == 0)
	{
		auto ProcPID = Proc.GetPID();
		SpatialOSStackProcHandle = FPlatformProcess::OpenProcess(ProcPID);
		if (SpatialOSStackProcHandle.IsValid())
		{
		SpatialOSStackProcessID = ProcPID;
		}
	}
	} while (ProcEnumerator.MoveNext() && !SpatialOSStackProcHandle.IsValid());
}

void FSpatialGDKEditorToolbarModule::OnPropertyChanged(UObject* ObjectBeingModified,
														FPropertyChangedEvent& PropertyChangedEvent)
{
	if (USpatialGDKEditorToolbarSettings* ToolbarSettings =
			Cast<USpatialGDKEditorToolbarSettings>(ObjectBeingModified))
	{
	FName PropertyName = PropertyChangedEvent.Property != nullptr
		? PropertyChangedEvent.Property->GetFName()
		: NAME_None;
	if (PropertyName.ToString() == TEXT("bStopSpatialOnExit"))
	{
		bStopSpatialOnExit = ToolbarSettings->bStopSpatialOnExit;
	}
	}
}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FSpatialGDKEditorToolbarModule, SpatialGDKEditorToolbar)
