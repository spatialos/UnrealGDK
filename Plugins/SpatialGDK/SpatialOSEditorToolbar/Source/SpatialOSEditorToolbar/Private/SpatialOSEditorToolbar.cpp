// Copyright (c) Improbable Worlds Ltd, All Rights Reserved
#include "SpatialOSEditorToolbar.h"
#include "Async.h"
#include "EditorStyleSet.h"
#include "Framework/MultiBox/MultiBoxBuilder.h"
#include "ISettingsContainer.h"
#include "ISettingsModule.h"
#include "ISettingsSection.h"
#include "Misc/MessageDialog.h"
#include "NotificationManager.h"
#include "SNotificationList.h"
#include "SpatialOSEditorToolbarCommands.h"
#include "SpatialOSEditorToolbarSettings.h"
#include "SpatialOSEditorToolbarStyle.h"

#include "LevelEditor.h"

DEFINE_LOG_CATEGORY(LogSpatialOSEditor);

#define LOCTEXT_NAMESPACE "FSpatialOSEditorToolbarModule"

FSpatialOSEditorToolbarModule::FSpatialOSEditorToolbarModule() : SpatialOSStackProcessID(0), bStopSpatialOnExit(false)
{
}

void FSpatialOSEditorToolbarModule::StartupModule()
{
	FSpatialOSEditorToolbarStyle::Initialize();
	FSpatialOSEditorToolbarStyle::ReloadTextures();

	FSpatialOSEditorToolbarCommands::Register();

	PluginCommands = MakeShareable(new FUICommandList);
	MapActions(PluginCommands);
	SetupToolbar(PluginCommands);

	RegisterSettings();
	CheckForRunningStack();

	OnPropertyChangedDelegateHandle = FCoreUObjectDelegates::OnObjectPropertyChanged.AddRaw(this, &FSpatialOSEditorToolbarModule::OnPropertyChanged);

	auto SpatialOSToolbarSettings = GetDefault<USpatialOSEditorToolbarSettings>();
	bStopSpatialOnExit = SpatialOSToolbarSettings->bStopSpatialOnExit;
}

void FSpatialOSEditorToolbarModule::ShutdownModule()
{
	FCoreUObjectDelegates::OnObjectPropertyChanged.Remove(OnPropertyChangedDelegateHandle);

	if (UObjectInitialized())
	{
		UnregisterSettings();
	}

	FSpatialOSEditorToolbarStyle::Shutdown();
	FSpatialOSEditorToolbarCommands::Unregister();
}

void FSpatialOSEditorToolbarModule::PreUnloadCallback()
{
	if (bStopSpatialOnExit)
	{
		StopRunningStack();
	}
}

void FSpatialOSEditorToolbarModule::Tick(float DeltaTime)
{
	if (SpatialOSStackProcessID != 0 && !FPlatformProcess::IsApplicationRunning(SpatialOSStackProcessID))
	{
		FPlatformProcess::CloseProc(SpatialOSStackProcHandle);
		SpatialOSStackProcessID = 0;
	}
}

void FSpatialOSEditorToolbarModule::RegisterSettings()
{
	if (ISettingsModule* SettingsModule = FModuleManager::GetModulePtr<ISettingsModule>("Settings"))
	{
		ISettingsContainerPtr SettingsContainer = SettingsModule->GetContainer("Project");

		SettingsContainer->DescribeCategory("SpatialOSEditorToolbar", LOCTEXT("RuntimeWDCategoryName", "SpatialOS - Toolbar"), LOCTEXT("RuntimeWDCategoryDescription", "Configuration for the SpatialOS Editor toolbar plugin"));

		ISettingsSectionPtr SettingsSection = SettingsModule->RegisterSettings("Project", "SpatialOS", "Toolbar", LOCTEXT("RuntimeGeneralSettingsName", "Toolbar"), LOCTEXT("RuntimeGeneralSettingsDescription", "Configuration for SpatialOS Editor toolbar plugin."), GetMutableDefault<USpatialOSEditorToolbarSettings>());

		if (SettingsSection.IsValid())
		{
			SettingsSection->OnModified().BindRaw(this, &FSpatialOSEditorToolbarModule::HandleSettingsSaved);
		}
	}
}

void FSpatialOSEditorToolbarModule::UnregisterSettings()
{
	if (ISettingsModule* SettingsModule = FModuleManager::GetModulePtr<ISettingsModule>("Settings"))
	{
		SettingsModule->UnregisterSettings("Project", "SpatialOS", "Toolbar");
	}
}

bool FSpatialOSEditorToolbarModule::HandleSettingsSaved()
{
	USpatialOSEditorToolbarSettings* Settings = GetMutableDefault<USpatialOSEditorToolbarSettings>();
	Settings->SaveConfig();

	return true;
}

void FSpatialOSEditorToolbarModule::MapActions(TSharedPtr<class FUICommandList> PluginCommands)
{
	PluginCommands->MapAction(FSpatialOSEditorToolbarCommands::Get().StartSpatialOSStackAction, FExecuteAction::CreateRaw(this, &FSpatialOSEditorToolbarModule::StartSpatialOSButtonClicked), FCanExecuteAction::CreateRaw(this, &FSpatialOSEditorToolbarModule::StartSpatialOSStackCanExecute));

	PluginCommands->MapAction(FSpatialOSEditorToolbarCommands::Get().StopSpatialOSStackAction, FExecuteAction::CreateRaw(this, &FSpatialOSEditorToolbarModule::StopSpatialOSButtonClicked), FCanExecuteAction::CreateRaw(this, &FSpatialOSEditorToolbarModule::StopSpatialOSStackCanExecute));

	PluginCommands->MapAction(FSpatialOSEditorToolbarCommands::Get().LaunchInspectorWebPageAction, FExecuteAction::CreateRaw(this, &FSpatialOSEditorToolbarModule::LaunchInspectorWebpageButtonClicked), FCanExecuteAction());
}

void FSpatialOSEditorToolbarModule::SetupToolbar(TSharedPtr<class FUICommandList> PluginCommands)
{
	FLevelEditorModule& LevelEditorModule = FModuleManager::LoadModuleChecked<FLevelEditorModule>("LevelEditor");

	{
		TSharedPtr<FExtender> MenuExtender = MakeShareable(new FExtender());
		MenuExtender->AddMenuExtension("General", EExtensionHook::After, PluginCommands, FMenuExtensionDelegate::CreateRaw(this, &FSpatialOSEditorToolbarModule::AddMenuExtension));

		LevelEditorModule.GetMenuExtensibilityManager()->AddExtender(MenuExtender);
	}

	{
		TSharedPtr<FExtender> ToolbarExtender = MakeShareable(new FExtender);
		ToolbarExtender->AddToolBarExtension("Game", EExtensionHook::After, PluginCommands, FToolBarExtensionDelegate::CreateRaw(this, &FSpatialOSEditorToolbarModule::AddToolbarExtension));

		LevelEditorModule.GetToolBarExtensibilityManager()->AddExtender(ToolbarExtender);
	}
}

void FSpatialOSEditorToolbarModule::AddMenuExtension(FMenuBuilder& Builder)
{
	Builder.BeginSection("SpatialOS", LOCTEXT("SpatialOS", "SpatialOS"));
	{
		Builder.AddMenuEntry(FSpatialOSEditorToolbarCommands::Get().LaunchInspectorWebPageAction);
	}
	Builder.EndSection();
}

void FSpatialOSEditorToolbarModule::AddToolbarExtension(FToolBarBuilder& Builder)
{
	Builder.AddSeparator(NAME_None);
	Builder.AddToolBarButton(FSpatialOSEditorToolbarCommands::Get().StartSpatialOSStackAction);
	Builder.AddToolBarButton(FSpatialOSEditorToolbarCommands::Get().StopSpatialOSStackAction);
	Builder.AddToolBarButton(FSpatialOSEditorToolbarCommands::Get().LaunchInspectorWebPageAction);
}

void FSpatialOSEditorToolbarModule::StartSpatialOSButtonClicked()
{
	auto SpatialOSToolbarSettings = GetDefault<USpatialOSEditorToolbarSettings>();

	const FString ExecuteAbsolutePath = FPaths::ConvertRelativePathToFull(SpatialOSToolbarSettings->ProjectRootFolder.Path);
	const FString CmdExecutable = TEXT("cmd.exe");
	const FString SpatialCmdArgument = FString::Printf(TEXT("/c spatial.exe local launch %s"), *SpatialOSToolbarSettings->SpatialOSLaunchArgument);

	UE_LOG(LogSpatialOSEditor, Log, TEXT("Starting cmd.exe with `%s` arguments."), *SpatialCmdArgument);
	// Temporary workaround to get spatial.exe to properly show a window we have
	// to call cmd.exe to
	// execute it.
	// We currently can't use pipes to capture output as it doesn't work properly
	// with current
	// spatial.exe.
	SpatialOSStackProcHandle = FPlatformProcess::CreateProc(*(CmdExecutable), *SpatialCmdArgument, true, false, false, &SpatialOSStackProcessID, 0, *ExecuteAbsolutePath, nullptr, nullptr);

	FNotificationInfo Info(SpatialOSStackProcHandle.IsValid() == true ? FText::FromString(TEXT("SpatialOS Starting...")) : FText::FromString(TEXT("Failed to start SpatialOS")));
	Info.ExpireDuration = 3.0f;
	Info.bUseSuccessFailIcons = true;
	auto NotificationItem = FSlateNotificationManager::Get().AddNotification(Info);

	if (!SpatialOSStackProcHandle.IsValid())
	{
		NotificationItem->SetCompletionState(SNotificationItem::CS_Fail);
		const auto LogPath = SpatialOSToolbarSettings->ProjectRootFolder.Path + FString(TEXT("/logs/spatial.log"));
		UE_LOG(LogSpatialOSEditor, Error, TEXT("Failed to start SpatialOS, please refer to log file `%s` for "
											   "more information."),
			   *LogPath);
	}
	else
	{
		NotificationItem->SetCompletionState(SNotificationItem::CS_Success);
	}

	NotificationItem->ExpireAndFadeout();
}

void FSpatialOSEditorToolbarModule::StopSpatialOSButtonClicked()
{
	StopRunningStack();
}

void FSpatialOSEditorToolbarModule::StopRunningStack()
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

void FSpatialOSEditorToolbarModule::LaunchInspectorWebpageButtonClicked()
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

bool FSpatialOSEditorToolbarModule::StartSpatialOSStackCanExecute() const
{
	return !SpatialOSStackProcHandle.IsValid() && !FPlatformProcess::IsApplicationRunning(SpatialOSStackProcessID);
}

bool FSpatialOSEditorToolbarModule::StopSpatialOSStackCanExecute() const
{
	return SpatialOSStackProcHandle.IsValid();
}

void FSpatialOSEditorToolbarModule::CheckForRunningStack()
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

void FSpatialOSEditorToolbarModule::OnPropertyChanged(UObject* ObjectBeingModified, FPropertyChangedEvent& PropertyChangedEvent)
{
	if (USpatialOSEditorToolbarSettings* ToolbarSettings = Cast<USpatialOSEditorToolbarSettings>(ObjectBeingModified))
	{
		FName PropertyName = PropertyChangedEvent.Property != nullptr ? PropertyChangedEvent.Property->GetFName() : NAME_None;
		if (PropertyName.ToString() == TEXT("bStopSpatialOnExit"))
		{
			bStopSpatialOnExit = ToolbarSettings->bStopSpatialOnExit;
		}
	}
}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FSpatialOSEditorToolbarModule, SpatialOSEditorToolbar)
