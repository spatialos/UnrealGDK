// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "SpatialGDKEditorToolbar.h"
#include "Async/Async.h"
#include "Editor.h"
#include "EditorStyleSet.h"
#include "Framework/MultiBox/MultiBoxBuilder.h"
#include "ISettingsContainer.h"
#include "ISettingsModule.h"
#include "ISettingsSection.h"
#include "Misc/MessageDialog.h"
#include "Framework/Notifications/NotificationManager.h"
#include "Widgets/Notifications/SNotificationList.h"
#include "SpatialGDKEditorToolbarCommands.h"
#include "SpatialGDKEditorToolbarStyle.h"

#include "SpatialGDKEditor.h"
#include "SpatialGDKEditorSettings.h"

#include "Editor/EditorEngine.h"
#include "HAL/FileManager.h"
#include "Sound/SoundBase.h"

#include "AssetRegistryModule.h"
#include "GeneralProjectSettings.h"
#include "LevelEditor.h"
#include "Misc/FileHelper.h"
#include "Serialization/JsonWriter.h"

DEFINE_LOG_CATEGORY(LogSpatialGDKEditorToolbar);

#define LOCTEXT_NAMESPACE "FSpatialGDKEditorToolbarModule"

FSpatialGDKEditorToolbarModule::FSpatialGDKEditorToolbarModule()
: bStopSpatialOnExit(false),
SpatialOSStackProcessID(0)
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

	CheckForRunningStack();

	// load sounds
	ExecutionStartSound = LoadObject<USoundBase>(nullptr, TEXT("/Engine/EditorSounds/Notifications/CompileStart_Cue.CompileStart_Cue"));
	ExecutionStartSound->AddToRoot();
	ExecutionSuccessSound = LoadObject<USoundBase>(nullptr, TEXT("/Engine/EditorSounds/Notifications/CompileSuccess_Cue.CompileSuccess_Cue"));
	ExecutionSuccessSound->AddToRoot();
	ExecutionFailSound = LoadObject<USoundBase>(nullptr, TEXT("/Engine/EditorSounds/Notifications/CompileFailed_Cue.CompileFailed_Cue"));
	ExecutionFailSound->AddToRoot();
	SpatialGDKEditorInstance = MakeShareable(new FSpatialGDKEditor());

	OnPropertyChangedDelegateHandle = FCoreUObjectDelegates::OnObjectPropertyChanged.AddRaw(this, &FSpatialGDKEditorToolbarModule::OnPropertyChanged);
	bStopSpatialOnExit = GetDefault<USpatialGDKEditorSettings>()->bStopSpatialOnExit;
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
	if (SpatialOSStackProcessID != 0 && !FPlatformProcess::IsApplicationRunning(SpatialOSStackProcessID))
	{
		FPlatformProcess::CloseProc(SpatialOSStackProcHandle);
		SpatialOSStackProcessID = 0;
	}
}

bool FSpatialGDKEditorToolbarModule::CanExecuteSchemaGenerator() const
{
	return SpatialGDKEditorInstance.IsValid() && !SpatialGDKEditorInstance.Get()->IsSchemaGeneratorRunning();
}

bool FSpatialGDKEditorToolbarModule::CanExecuteSnapshotGenerator() const
{
	return SpatialGDKEditorInstance.IsValid() && !SpatialGDKEditorInstance.Get()->IsSchemaGeneratorRunning();
}

void FSpatialGDKEditorToolbarModule::MapActions(TSharedPtr<class FUICommandList> InPluginCommands)
{
	InPluginCommands->MapAction(
		FSpatialGDKEditorToolbarCommands::Get().CreateSpatialGDKSchema,
		FExecuteAction::CreateRaw(this, &FSpatialGDKEditorToolbarModule::SchemaGenerateButtonClicked),
		FCanExecuteAction::CreateRaw(this, &FSpatialGDKEditorToolbarModule::CanExecuteSchemaGenerator));

	InPluginCommands->MapAction(
		FSpatialGDKEditorToolbarCommands::Get().CreateSpatialGDKSnapshot,
		FExecuteAction::CreateRaw(this, &FSpatialGDKEditorToolbarModule::CreateSnapshotButtonClicked),
		FCanExecuteAction::CreateRaw(this, &FSpatialGDKEditorToolbarModule::CanExecuteSnapshotGenerator));

	InPluginCommands->MapAction(
		FSpatialGDKEditorToolbarCommands::Get().StartSpatialOSStackAction,
		FExecuteAction::CreateRaw(this, &FSpatialGDKEditorToolbarModule::StartSpatialOSButtonClicked),
		FCanExecuteAction::CreateRaw(this, &FSpatialGDKEditorToolbarModule::StartSpatialOSStackCanExecute));

	InPluginCommands->MapAction(
		FSpatialGDKEditorToolbarCommands::Get().StopSpatialOSStackAction,
		FExecuteAction::CreateRaw(this, &FSpatialGDKEditorToolbarModule::StopSpatialOSButtonClicked),
		FCanExecuteAction::CreateRaw(this, &FSpatialGDKEditorToolbarModule::StopSpatialOSStackCanExecute));

	InPluginCommands->MapAction(
		FSpatialGDKEditorToolbarCommands::Get().LaunchInspectorWebPageAction,
		FExecuteAction::CreateRaw(this, &FSpatialGDKEditorToolbarModule::LaunchInspectorWebpageButtonClicked),
		FCanExecuteAction());
}

void FSpatialGDKEditorToolbarModule::SetupToolbar(TSharedPtr<class FUICommandList> InPluginCommands)
{
	FLevelEditorModule& LevelEditorModule =
		FModuleManager::LoadModuleChecked<FLevelEditorModule>("LevelEditor");

	{
		TSharedPtr<FExtender> MenuExtender = MakeShareable(new FExtender());
		MenuExtender->AddMenuExtension(
			"General", EExtensionHook::After, InPluginCommands,
			FMenuExtensionDelegate::CreateRaw(this, &FSpatialGDKEditorToolbarModule::AddMenuExtension));

		LevelEditorModule.GetMenuExtensibilityManager()->AddExtender(MenuExtender);
	}

	{
		TSharedPtr<FExtender> ToolbarExtender = MakeShareable(new FExtender);
		ToolbarExtender->AddToolBarExtension(
			"Game", EExtensionHook::After, InPluginCommands,
			FToolBarExtensionDelegate::CreateRaw(this,
				&FSpatialGDKEditorToolbarModule::AddToolbarExtension));

		LevelEditorModule.GetToolBarExtensibilityManager()->AddExtender(ToolbarExtender);
	}
}

void FSpatialGDKEditorToolbarModule::AddMenuExtension(FMenuBuilder& Builder)
{
	Builder.BeginSection("SpatialOS Unreal GDK", LOCTEXT("SpatialOS Unreal GDK", "SpatialOS Unreal GDK"));
	{
		Builder.AddMenuEntry(FSpatialGDKEditorToolbarCommands::Get().CreateSpatialGDKSchema);
		Builder.AddMenuEntry(FSpatialGDKEditorToolbarCommands::Get().CreateSpatialGDKSnapshot);
		Builder.AddMenuEntry(FSpatialGDKEditorToolbarCommands::Get().StartSpatialOSStackAction);
		Builder.AddMenuEntry(FSpatialGDKEditorToolbarCommands::Get().StopSpatialOSStackAction);
		Builder.AddMenuEntry(FSpatialGDKEditorToolbarCommands::Get().LaunchInspectorWebPageAction);
	}
	Builder.EndSection();
}

void FSpatialGDKEditorToolbarModule::AddToolbarExtension(FToolBarBuilder& Builder)
{
	Builder.AddSeparator(NAME_None);
	Builder.AddToolBarButton(FSpatialGDKEditorToolbarCommands::Get().CreateSpatialGDKSchema);
	Builder.AddToolBarButton(FSpatialGDKEditorToolbarCommands::Get().CreateSpatialGDKSnapshot);
	Builder.AddToolBarButton(FSpatialGDKEditorToolbarCommands::Get().StartSpatialOSStackAction);
	Builder.AddToolBarButton(FSpatialGDKEditorToolbarCommands::Get().StopSpatialOSStackAction);
	Builder.AddToolBarButton(FSpatialGDKEditorToolbarCommands::Get().LaunchInspectorWebPageAction);
}

void FSpatialGDKEditorToolbarModule::CreateSnapshotButtonClicked()
{
	ShowTaskStartNotification("Started snapshot generation");

	const USpatialGDKEditorSettings* Settings = GetDefault<USpatialGDKEditorSettings>();

	SpatialGDKEditorInstance->GenerateSnapshot(
		GEditor->GetEditorWorldContext().World(), Settings->GetSpatialOSSnapshotFile(),
		FSimpleDelegate::CreateLambda([this]() { ShowSuccessNotification("Snapshot successfully generated!"); }),
		FSimpleDelegate::CreateLambda([this]() { ShowFailedNotification("Snapshot generation failed!"); }),
		FSpatialGDKEditorErrorHandler::CreateLambda([](FString ErrorText) { FMessageDialog::Debugf(FText::FromString(ErrorText)); }));
}

void FSpatialGDKEditorToolbarModule::SchemaGenerateButtonClicked()
{
	ShowTaskStartNotification("Generating Schema");
	SpatialGDKEditorInstance->GenerateSchema(
		FSimpleDelegate::CreateLambda([this]() { ShowSuccessNotification("Schema Generation Completed!"); }),
		FSimpleDelegate::CreateLambda([this]() { ShowFailedNotification("Schema Generation Failed"); }),
		FSpatialGDKEditorErrorHandler::CreateLambda([](FString ErrorText) { FMessageDialog::Debugf(FText::FromString(ErrorText)); }));
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
	const USpatialGDKEditorSettings* SpatialGDKSettings = GetDefault<USpatialGDKEditorSettings>();

	FString LaunchConfig;
	if (SpatialGDKSettings->bGenerateDefaultLaunchConfig)
	{
		LaunchConfig = FPaths::Combine(FPaths::ConvertRelativePathToFull(FPaths::ProjectIntermediateDir()), TEXT("Improbable/DefaultLaunchConfig.json"));
		GenerateDefaultLaunchConfig(LaunchConfig);
	}
	else
	{
		LaunchConfig = SpatialGDKSettings->GetSpatialOSLaunchConfig();
	}

	const FString ExecuteAbsolutePath = SpatialGDKSettings->GetSpatialOSDirectory();
	const FString CmdExecutable = TEXT("cmd.exe");

	const FString SpatialCmdArgument = FString::Printf(
		TEXT("/c cmd.exe /c spatial.exe worker build build-config ^& spatial.exe local launch %s ^& pause"), *LaunchConfig);

	UE_LOG(LogSpatialGDKEditorToolbar, Log, TEXT("Starting cmd.exe with `%s` arguments."), *SpatialCmdArgument);
	// Temporary workaround: To get spatial.exe to properly show a window we have to call cmd.exe to
	// execute it. We currently can't use pipes to capture output as it doesn't work properly with current
	// spatial.exe.
	SpatialOSStackProcHandle = FPlatformProcess::CreateProc(
		*(CmdExecutable), *SpatialCmdArgument, true, false, false, &SpatialOSStackProcessID, 0,
		*ExecuteAbsolutePath, nullptr, nullptr);

	FNotificationInfo Info(SpatialOSStackProcHandle.IsValid() == true
							 ? FText::FromString(TEXT("SpatialOS Starting..."))
							 : FText::FromString(TEXT("Failed to start SpatialOS")));
	Info.ExpireDuration = 3.0f;
	Info.bUseSuccessFailIcons = true;
	TSharedPtr<SNotificationItem> NotificationItem = FSlateNotificationManager::Get().AddNotification(Info);

	if (!SpatialOSStackProcHandle.IsValid())
	{
		NotificationItem->SetCompletionState(SNotificationItem::CS_Fail);
		const FString SpatialLogPath =
			SpatialGDKSettings->GetSpatialOSDirectory() + FString(TEXT("/logs/spatial.log"));
		UE_LOG(LogSpatialGDKEditorToolbar, Error,
				TEXT("Failed to start SpatialOS, please refer to log file `%s` for more information."),
				*SpatialLogPath);
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
		TSharedPtr<SNotificationItem> NotificationItem = FSlateNotificationManager::Get().AddNotification(Info);
		NotificationItem->SetCompletionState(SNotificationItem::CS_Fail);
		NotificationItem->ExpireAndFadeout();
	}
}

bool FSpatialGDKEditorToolbarModule::StartSpatialOSStackCanExecute() const
{
	return !SpatialOSStackProcHandle.IsValid() && !FPlatformProcess::IsApplicationRunning(SpatialOSStackProcessID);
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
		FPlatformProcess::FProcEnumInfo Proc = ProcEnumerator.GetCurrent();
		const FString ProcName = Proc.GetName();
		if (ProcName.Compare(TEXT("spatial.exe"), ESearchCase::IgnoreCase) == 0)
		{
			uint32 ProcPID = Proc.GetPID();
			SpatialOSStackProcHandle = FPlatformProcess::OpenProcess(ProcPID);
			if (SpatialOSStackProcHandle.IsValid())
			{
				SpatialOSStackProcessID = ProcPID;
			}
		}
	} while (ProcEnumerator.MoveNext() && !SpatialOSStackProcHandle.IsValid());
}

/**
* This function is used to update our own local copy of bStopSpatialOnExit as Settings change.
* We keep the copy of the variable as all the USpatialGDKEditorSettings references get
* cleaned before all the available callbacks that IModuleInterface exposes. This means that we can't access
* this variable through its references after the engine is closed.
*/
void FSpatialGDKEditorToolbarModule::OnPropertyChanged(UObject* ObjectBeingModified, FPropertyChangedEvent& PropertyChangedEvent)
{
	if (USpatialGDKEditorSettings* Settings = Cast<USpatialGDKEditorSettings>(ObjectBeingModified))
	{
		FName PropertyName = PropertyChangedEvent.Property != nullptr
				? PropertyChangedEvent.Property->GetFName()
				: NAME_None;
		if (PropertyName.ToString() == TEXT("bStopSpatialOnExit"))
		{
			bStopSpatialOnExit = Settings->bStopSpatialOnExit;
		}
	}
}

bool FSpatialGDKEditorToolbarModule::GenerateDefaultLaunchConfig(const FString& LaunchConfigPath) const
{
	FString Text;
	TSharedRef< TJsonWriter<> > Writer = TJsonWriterFactory<>::Create(&Text);

	// Populate json file for launch config
	Writer->WriteObjectStart(); // Start of json
		Writer->WriteValue(TEXT("template"), TEXT("small")); // Template section
		Writer->WriteObjectStart(TEXT("world")); // World section begin
			Writer->WriteObjectStart(TEXT("dimensions"));
				Writer->WriteValue(TEXT("x_meters"), 2000);
				Writer->WriteValue(TEXT("z_meters"), 2000);
			Writer->WriteObjectEnd();
			Writer->WriteValue(TEXT("chunk_edge_length_meters"), 50);
			Writer->WriteValue(TEXT("streaming_query_interval"), 4);
			Writer->WriteArrayStart(TEXT("legacy_flags"));
				Writer->WriteObjectStart();
					Writer->WriteValue(TEXT("name"), TEXT("streaming_query_diff"));
					Writer->WriteValue(TEXT("value"), TEXT("true"));
				Writer->WriteObjectEnd();
				Writer->WriteObjectStart();
					Writer->WriteValue(TEXT("name"), TEXT("bridge_qos_max_timeout"));
					Writer->WriteValue(TEXT("value"), TEXT("0"));
				Writer->WriteObjectEnd();
				Writer->WriteObjectStart();
					Writer->WriteValue(TEXT("name"), TEXT("bridge_soft_handover_enabled"));
					Writer->WriteValue(TEXT("value"), TEXT("false"));
				Writer->WriteObjectEnd();
			Writer->WriteArrayEnd();
			Writer->WriteObjectStart(TEXT("snapshots"));
				Writer->WriteValue(TEXT("snapshot_write_period_seconds"), 0);
			Writer->WriteObjectEnd();
		Writer->WriteObjectEnd(); // World section end
		Writer->WriteObjectStart(TEXT("load_balancing")); // Load balancing section begin
			Writer->WriteArrayStart("layer_configurations");
				Writer->WriteObjectStart();
					Writer->WriteValue(TEXT("layer"), TEXT("UnrealWorker"));
					Writer->WriteObjectStart("rectangle_grid");

						int Cols = 1;
						int Rows = 1;

						if (const ULevelEditorPlaySettings* PlayInSettings = GetDefault<ULevelEditorPlaySettings>())
						{
							int NumServers = 1;
							PlayInSettings->GetPlayNumberOfServers(NumServers);
		
							if (NumServers <= 2)
							{
								Cols = NumServers;
								Rows = 1;
							}
							else
							{
								// Find greatest divisor.
								for (int Divisor = FMath::Sqrt(NumServers); Divisor >= 2; Divisor--)
								{
									if (NumServers % Divisor == 0)
									{
										int GreatestDivisor = NumServers / Divisor;
										Cols = GreatestDivisor;
										Rows = NumServers / GreatestDivisor;
										break;
									}
								}								
							}
						}

						Writer->WriteValue(TEXT("cols"), Cols);
						Writer->WriteValue(TEXT("rows"), Rows);
					Writer->WriteObjectEnd();
					Writer->WriteObjectStart(TEXT("options"));
						Writer->WriteValue(TEXT("manual_worker_connection_only"), true);
					Writer->WriteObjectEnd();
				Writer->WriteObjectEnd();
			Writer->WriteArrayEnd();
		Writer->WriteObjectEnd(); // Load balancing section end
		Writer->WriteArrayStart(TEXT("workers")); // Workers section begin
			Writer->WriteObjectStart();
				Writer->WriteValue(TEXT("worker_type"), TEXT("UnrealWorker"));
				Writer->WriteRawJSONValue("flags", TEXT("[]"));
				Writer->WriteArrayStart("permissions");
					Writer->WriteObjectStart();
						Writer->WriteObjectStart(TEXT("all"));
						Writer->WriteObjectEnd();
					Writer->WriteObjectEnd();
				Writer->WriteArrayEnd();
			Writer->WriteObjectEnd();
			Writer->WriteObjectStart();
				Writer->WriteValue(TEXT("worker_type"), TEXT("UnrealClient"));
				Writer->WriteRawJSONValue("flags", TEXT("[]"));
				Writer->WriteArrayStart("permissions");
					Writer->WriteObjectStart();
						Writer->WriteObjectStart(TEXT("all"));
						Writer->WriteObjectEnd();
					Writer->WriteObjectEnd();
				Writer->WriteArrayEnd();
			Writer->WriteObjectEnd();
		Writer->WriteArrayEnd(); // Worker section end
	Writer->WriteObjectEnd(); // End of json

	Writer->Close();

	if (!FFileHelper::SaveStringToFile(Text, *LaunchConfigPath))
	{
		UE_LOG(LogSpatialGDKEditorToolbar, Log, TEXT("Failed to write output file '%s'. Perhaps the file is Read-Only?"), *LaunchConfigPath);
		return false;
	}

	return true;
}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FSpatialGDKEditorToolbarModule, SpatialGDKEditorToolbar)
