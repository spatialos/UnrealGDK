// Copyright (c) Improbable Worlds Ltd, All Rights Reserved
#pragma optimize("", off)
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

#include "SpatialConstants.h"
#include "SpatialGDKEditor.h"
#include "SpatialGDKSettings.h"
#include "SpatialGDKEditorSettings.h"

#include "Editor/EditorEngine.h"
#include "HAL/FileManager.h"
#include "Sound/SoundBase.h"

#include "AssetRegistryModule.h"
#include "GeneralProjectSettings.h"
#include "LevelEditor.h"
#include "Misc/FileHelper.h"
#include "Serialization/JsonWriter.h"
#include "Serialization/JsonReader.h"
#include "Serialization/JsonSerializer.h"

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

	// For checking whether we can stop or start.
	TimeSinceLastStatusCheck = FDateTime::Now();
	bStopCanExecute = false;
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
		CleanupSpatialProcess();
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
		FSpatialGDKEditorToolbarCommands::Get().CreateSpatialGDKSchemaFull,
		FExecuteAction::CreateRaw(this, &FSpatialGDKEditorToolbarModule::SchemaGenerateFullButtonClicked),
		FCanExecuteAction::CreateRaw(this, &FSpatialGDKEditorToolbarModule::CanExecuteSchemaGenerator));

	InPluginCommands->MapAction(
		FSpatialGDKEditorToolbarCommands::Get().CreateSpatialGDKSnapshot,
		FExecuteAction::CreateRaw(this, &FSpatialGDKEditorToolbarModule::CreateSnapshotButtonClicked),
		FCanExecuteAction::CreateRaw(this, &FSpatialGDKEditorToolbarModule::CanExecuteSnapshotGenerator));

	InPluginCommands->MapAction(
		FSpatialGDKEditorToolbarCommands::Get().StartSpatialOSStackAction,
		FExecuteAction::CreateRaw(this, &FSpatialGDKEditorToolbarModule::StartSpatialOSButtonClicked),
		FCanExecuteAction::CreateRaw(this, &FSpatialGDKEditorToolbarModule::StartSpatialOSStackCanExecute),
		FIsActionChecked(),
		FIsActionButtonVisible::CreateRaw(this, &FSpatialGDKEditorToolbarModule::StartSpatialOSStackCanExecute));

	InPluginCommands->MapAction(
		FSpatialGDKEditorToolbarCommands::Get().StopSpatialOSStackAction,
		FExecuteAction::CreateRaw(this, &FSpatialGDKEditorToolbarModule::StopSpatialOSButtonClicked),
		FCanExecuteAction::CreateRaw(this, &FSpatialGDKEditorToolbarModule::StopSpatialOSStackCanExecute),
		FIsActionChecked(),
		FIsActionButtonVisible::CreateRaw(this, &FSpatialGDKEditorToolbarModule::StopSpatialOSStackCanExecute));

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
	Builder.AddComboButton(
		FUIAction(),
		FOnGetContent::CreateRaw(this, &FSpatialGDKEditorToolbarModule::CreateGenerateSchemaMenuContent),
		LOCTEXT("GDKSchemaCombo_Label", "Schema Generation Options"),
		TAttribute<FText>(),
		FSlateIcon(FEditorStyle::GetStyleSetName(), "GDK.Schema"),
		true
	);
	Builder.AddToolBarButton(FSpatialGDKEditorToolbarCommands::Get().CreateSpatialGDKSnapshot);
	Builder.AddToolBarButton(FSpatialGDKEditorToolbarCommands::Get().StartSpatialOSStackAction);
	Builder.AddToolBarButton(FSpatialGDKEditorToolbarCommands::Get().StopSpatialOSStackAction);
	Builder.AddToolBarButton(FSpatialGDKEditorToolbarCommands::Get().LaunchInspectorWebPageAction);
}

TSharedRef<SWidget> FSpatialGDKEditorToolbarModule::CreateGenerateSchemaMenuContent()
{
	FMenuBuilder MenuBuilder(true, PluginCommands);
	MenuBuilder.BeginSection(NAME_None, LOCTEXT("GDKSchemaOptionsHeader", "Schema Generation"));
	{
		MenuBuilder.AddMenuEntry(FSpatialGDKEditorToolbarCommands::Get().CreateSpatialGDKSchemaFull);
	}
	MenuBuilder.EndSection();

	return MenuBuilder.MakeWidget();
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
	ShowTaskStartNotification("Generating Schema (Incremental)");

	if (SpatialGDKEditorInstance->GenerateSchema(false))
	{
		ShowSuccessNotification("Incremental Schema Generation completed!");
	}
	else
	{
		ShowFailedNotification("Incremental Schema Generation failed");
	}
}

void FSpatialGDKEditorToolbarModule::SchemaGenerateFullButtonClicked()
{
	ShowTaskStartNotification("Generating Schema (Full)");

	if (SpatialGDKEditorInstance->GenerateSchema(true))
	{
		ShowSuccessNotification("Full Schema Generation completed!");
	}
	else
	{
		ShowFailedNotification("Full Schema Generation failed");
	}
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

bool FSpatialGDKEditorToolbarModule::ValidateGeneratedLaunchConfig() const
{
	const USpatialGDKEditorSettings* SpatialGDKEditorSettings = GetDefault<USpatialGDKEditorSettings>();
	const USpatialGDKSettings* SpatialGDKRuntimeSettings = GetDefault<USpatialGDKSettings>();
	const FSpatialLaunchConfigDescription& LaunchConfigDescription = SpatialGDKEditorSettings->LaunchConfigDesc;

	if (const FString* EnableChunkInterest = LaunchConfigDescription.World.LegacyFlags.Find(TEXT("enable_chunk_interest")))
	{
		if (SpatialGDKRuntimeSettings->bUsingQBI && (*EnableChunkInterest == TEXT("true")))
		{
			const EAppReturnType::Type Result = FMessageDialog::Open(EAppMsgType::YesNo, FText::FromString(TEXT("The legacy flag \"enable_chunk_interest\" is set to true in the generated launch configuration. This flag needs to be set to false when QBI is enabled.\n\nDo you want to configure your launch config settings now?")));

			if (Result == EAppReturnType::Yes)
			{
				FModuleManager::LoadModuleChecked<ISettingsModule>("Settings").ShowViewer("Project", "SpatialGDKEditor", "Editor Settings");
			}

			return false;
		}
		else if (!SpatialGDKRuntimeSettings->bUsingQBI && (*EnableChunkInterest == TEXT("false")))
		{
			const EAppReturnType::Type Result = FMessageDialog::Open(EAppMsgType::YesNo, FText::FromString(TEXT("The legacy flag \"enable_chunk_interest\" is set to false in the generated launch configuration. This flag needs to be set to true when QBI is disabled.\n\nDo you want to configure your launch config settings now?")));

			if (Result == EAppReturnType::Yes)
			{
				FModuleManager::LoadModuleChecked<ISettingsModule>("Settings").ShowViewer("Project", "SpatialGDKEditor", "Editor Settings");
			}

			return false;
		}
	}
	else
	{
		const EAppReturnType::Type Result = FMessageDialog::Open(EAppMsgType::YesNo, FText::FromString(TEXT("The legacy flag \"enable_chunk_interest\" is not specified in the generated launch configuration.\n\nDo you want to configure your launch config settings now?")));

		if (Result == EAppReturnType::Yes)
		{
			FModuleManager::LoadModuleChecked<ISettingsModule>("Settings").ShowViewer("Project", "SpatialGDKEditor", "Editor Settings");
		}

		return false;
	}

	int32 PlayNumberOfServers;
	GetDefault<ULevelEditorPlaySettings>()->GetPlayNumberOfServers(PlayNumberOfServers);
	if (!SpatialGDKRuntimeSettings->bEnableHandover && PlayNumberOfServers > 1)
	{
		const EAppReturnType::Type Result = FMessageDialog::Open(EAppMsgType::YesNo, FText::FromString(TEXT("Property handover is disabled and multiple launch servers are specified.\nThis is not supported.\n\nDo you want to configure your project settings now?")));

		if (Result == EAppReturnType::Yes)
		{
			FModuleManager::LoadModuleChecked<ISettingsModule>("Settings").ShowViewer("Project", "SpatialGDKEditor", "Runtime Settings");
		}

		return false;
	}

	return true;
}

void FSpatialGDKEditorToolbarModule::StartSpatialOSButtonClicked()
{
	const USpatialGDKEditorSettings* SpatialGDKSettings = GetDefault<USpatialGDKEditorSettings>();

	FString LaunchConfig;
	if (SpatialGDKSettings->bGenerateDefaultLaunchConfig)
	{
		if (!ValidateGeneratedLaunchConfig())
		{
			return;
		}

		LaunchConfig = FPaths::Combine(FPaths::ConvertRelativePathToFull(FPaths::ProjectIntermediateDir()), TEXT("Improbable/DefaultLaunchConfig.json"));
		GenerateDefaultLaunchConfig(LaunchConfig);
	}
	else
	{
		LaunchConfig = SpatialGDKSettings->GetSpatialOSLaunchConfig();
	}

	const FString ExecuteAbsolutePath = SpatialGDKSettings->GetSpatialOSDirectory();

	FString SpotExe = SpatialGDKSettings->GetSpotPath();

	// TODO: Read the project name from the spatialos.json

	// TODO: Removed build-build config. Add it back
	const FString SpatialCmdArgument = FString::Printf(
		TEXT("alpha deployment create --launch-config=\"%s\" --name=testname --project-name=your_project_name_here --json %s ^& pause"), *LaunchConfig, *SpatialGDKSettings->GetSpatialOSCommandLineLaunchFlags());

	UE_LOG(LogSpatialGDKEditorToolbar, Log, TEXT("Starting spot.exe with `%s` arguments."), *SpatialCmdArgument);

	// Create pipes to read output of spot.
	void* ReadPipe;
	void* WritePipe;
	if(!FPlatformProcess::CreatePipe(ReadPipe, WritePipe))
	{
		UE_LOG(LogSpatialGDKEditorToolbar, Error, TEXT("Failed to create pipes for local launch!"));
		return;
	}

	SpatialOSStackProcHandle = FPlatformProcess::CreateProc(
		*(SpotExe), *SpatialCmdArgument, true, false, false, &SpatialOSStackProcessID, 0,
		*ExecuteAbsolutePath, WritePipe, ReadPipe);

	FNotificationInfo Info(SpatialOSStackProcHandle.IsValid() == true
							 ? FText::FromString(TEXT("SpatialOS Starting..."))
							 : FText::FromString(TEXT("Failed to start SpatialOS")));
	Info.ExpireDuration = 3.0f;
	Info.bUseSuccessFailIcons = true;
	TSharedPtr<SNotificationItem> NotificationItem = FSlateNotificationManager::Get().AddNotification(Info);

	// TODO: This is blocking, move to async.
	FPlatformProcess::WaitForProc(SpatialOSStackProcHandle);

	FString WritePipeResult = FPlatformProcess::ReadPipe(WritePipe);
	FString ReadPipeResult = FPlatformProcess::ReadPipe(ReadPipe);

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

		UE_LOG(LogSpatialGDKEditorToolbar, Log, TEXT("SPOT: %s"), *WritePipeResult);
		UE_LOG(LogSpatialGDKEditorToolbar, Log, TEXT("SPOT: %s"), *ReadPipeResult);
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

		CleanupSpatialProcess();
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

TSharedPtr<FJsonObject> ParseJson(FString RawJsonString)
{
	TSharedPtr<FJsonObject> JsonParsed;
	TSharedRef<TJsonReader<TCHAR>> JsonReader = TJsonReaderFactory<TCHAR>::Create(RawJsonString);
	FJsonSerializer::Deserialize(JsonReader, JsonParsed);

	return JsonParsed;
}

// TODO: How often is this called? might be expensive to call Spot all the time.
bool FSpatialGDKEditorToolbarModule::StopSpatialOSStackCanExecute()
{
	FDateTime ModTime = TimeSinceLastStatusCheck;
	ModTime += FTimespan::FromSeconds(2);
	FDateTime NowTime = FDateTime::Now();

	if(NowTime < ModTime)
	{
		return bStopCanExecute;
	}

	// TODO: Seems strange to have this here, is it expensive?
	const USpatialGDKEditorSettings* SpatialGDKSettings = GetDefault<USpatialGDKEditorSettings>();

	FString SpotExe = SpatialGDKSettings->GetSpotPath();

	const FString SpotListArgument = FString::Printf(
		TEXT("alpha deployment list --project-name=your_project_name_here --json ^& pause"));

	// Create pipes to read output of spot.
	void* ReadPipe;
	void* WritePipe;
	FPlatformProcess::CreatePipe(ReadPipe, WritePipe);

	FProcHandle SpotListHandle;
	uint32 SpotListProcessID;

	SpotListHandle = FPlatformProcess::CreateProc(
		*(SpotExe), *SpotListArgument, true, false, false, &SpotListProcessID, 0,
		*SpatialGDKSettings->GetSpatialOSDirectory(), WritePipe, ReadPipe);

	if (!SpotListHandle.IsValid())
	{
		UE_LOG(LogSpatialGDKEditorToolbar, Error, TEXT("Failed to spot list"));
		return bStopCanExecute;
	}


	FString WritePipeResult = FPlatformProcess::ReadPipe(WritePipe);
	FString ReadPipeResult = FPlatformProcess::ReadPipe(ReadPipe);

	// Make sure the pipe doesn't get clogged.
	AsyncTask(ENamedThreads::AnyBackgroundThreadNormalTask, [&ReadPipe, &ReadPipeResult, &SpotListHandle]
	{
		while(FPlatformProcess::IsProcRunning(SpotListHandle))
		{
			ReadPipeResult += FPlatformProcess::ReadPipe(ReadPipe);
		}
	});

	// TODO: This is blocking, move to async.
	if (FPlatformProcess::IsProcRunning(SpotListHandle))
	{
		FPlatformProcess::WaitForProc(SpotListHandle);
	}

	TSharedPtr<FJsonObject> SpotJsonResult = ParseJson(ReadPipeResult);
	if(!SpotJsonResult.IsValid())
	{
		UE_LOG(LogSpatialGDKEditorToolbar, Error, TEXT("Json parsing of spot result failed. Can't check deployment status."));
	}

	TSharedPtr<FJsonObject> SpotJsonContent = SpotJsonResult->GetObjectField(TEXT("content"));

	UE_LOG(LogSpatialGDKEditorToolbar, Log, TEXT("Spot deployments --------------"));

	// TODO: This is crashing randomly, use TryGetArrayField.
	TArray<TSharedPtr<FJsonValue>> deployments = SpotJsonContent->GetArrayField(TEXT("deployments"));

	for (TSharedPtr<FJsonValue> JsonDeployment : deployments)
	{
		FString DeploymentStatus = JsonDeployment->AsObject()->GetStringField(TEXT("status"));
		if (DeploymentStatus == TEXT("RUNNING"))
		{
			FString DeploymentId = JsonDeployment->AsObject()->GetStringField(TEXT("id"));

			UE_LOG(LogSpatialGDKEditorToolbar, Log, TEXT("Running deployment found: %s"), *DeploymentId);
			bStopCanExecute = true;
		}
	}

	TimeSinceLastStatusCheck = FDateTime::Now();
	return bStopCanExecute;
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

void FSpatialGDKEditorToolbarModule::CleanupSpatialProcess()
{
	FPlatformProcess::CloseProc(SpatialOSStackProcHandle);
	SpatialOSStackProcessID = 0;

	OnSpatialShutdown.Broadcast();
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
	if (const USpatialGDKEditorSettings* SpatialGDKSettings = GetDefault<USpatialGDKEditorSettings>())
	{
		FString Text;
		TSharedRef< TJsonWriter<> > Writer = TJsonWriterFactory<>::Create(&Text);

		const FSpatialLaunchConfigDescription& LaunchConfigDescription = SpatialGDKSettings->LaunchConfigDesc;

		// Populate json file for launch config
		Writer->WriteObjectStart(); // Start of json
			Writer->WriteValue(TEXT("template"), LaunchConfigDescription.Template); // Template section
			Writer->WriteObjectStart(TEXT("world")); // World section begin
				Writer->WriteObjectStart(TEXT("dimensions"));
					Writer->WriteValue(TEXT("x_meters"), LaunchConfigDescription.World.Dimensions.X);
					Writer->WriteValue(TEXT("z_meters"), LaunchConfigDescription.World.Dimensions.Y);
				Writer->WriteObjectEnd();
			Writer->WriteValue(TEXT("chunk_edge_length_meters"), LaunchConfigDescription.World.ChunkEdgeLengthMeters);
			Writer->WriteValue(TEXT("streaming_query_interval"), LaunchConfigDescription.World.StreamingQueryIntervalSeconds);
			Writer->WriteArrayStart(TEXT("legacy_flags"));
			for (auto& Flag : LaunchConfigDescription.World.LegacyFlags)
			{
				WriteFlagSection(Writer, Flag.Key, Flag.Value);
			}
			Writer->WriteArrayEnd();
			Writer->WriteArrayStart(TEXT("legacy_javaparams"));
			for (auto& Parameter : LaunchConfigDescription.World.LegacyJavaParams)
			{
				WriteFlagSection(Writer, Parameter.Key, Parameter.Value);
			}
			Writer->WriteArrayEnd();
			Writer->WriteObjectStart(TEXT("snapshots"));
				Writer->WriteValue(TEXT("snapshot_write_period_seconds"), LaunchConfigDescription.World.SnapshotWritePeriodSeconds);
			Writer->WriteObjectEnd();
		Writer->WriteObjectEnd(); // World section end
		Writer->WriteObjectStart(TEXT("load_balancing")); // Load balancing section begin
			Writer->WriteArrayStart("layer_configurations");
			for (const FWorkerTypeLaunchSection& Worker : LaunchConfigDescription.Workers)
			{
				WriteLoadbalancingSection(Writer, Worker.WorkerTypeName, Worker.Columns, Worker.Rows, Worker.bManualWorkerConnectionOnly);
			}
			Writer->WriteArrayEnd();
			Writer->WriteObjectEnd(); // Load balancing section end
			Writer->WriteArrayStart(TEXT("workers")); // Workers section begin
			for (const FWorkerTypeLaunchSection& Worker : LaunchConfigDescription.Workers)
			{
				WriteWorkerSection(Writer, Worker);
			}
			// Write the client worker section
			FWorkerTypeLaunchSection ClientWorker;
			ClientWorker.WorkerTypeName = SpatialConstants::ClientWorkerType;
			ClientWorker.WorkerPermissions.bAllPermissions = true;
			ClientWorker.bLoginRateLimitEnabled = false;
			WriteWorkerSection(Writer, ClientWorker);
			Writer->WriteArrayEnd(); // Worker section end
		Writer->WriteObjectEnd(); // End of json

		Writer->Close();

		if (!FFileHelper::SaveStringToFile(Text, *LaunchConfigPath))
		{
			UE_LOG(LogSpatialGDKEditorToolbar, Log, TEXT("Failed to write output file '%s'. It might be that the file is read-only."), *LaunchConfigPath);
			return false;
		}

		return true;
	}

	return false;
}

bool FSpatialGDKEditorToolbarModule::WriteFlagSection(TSharedRef< TJsonWriter<> > Writer, const FString& Key, const FString& Value) const
{
	Writer->WriteObjectStart();
		Writer->WriteValue(TEXT("name"), Key);
		Writer->WriteValue(TEXT("value"), Value);
	Writer->WriteObjectEnd();

	return true;
}

bool FSpatialGDKEditorToolbarModule::WriteWorkerSection(TSharedRef< TJsonWriter<> > Writer, const FWorkerTypeLaunchSection& Worker) const
{
	Writer->WriteObjectStart();
		Writer->WriteValue(TEXT("worker_type"), *Worker.WorkerTypeName);
		Writer->WriteArrayStart(TEXT("flags"));
		for (const auto& Flag : Worker.Flags)
		{
			WriteFlagSection(Writer, Flag.Key, Flag.Value);
		}
		Writer->WriteArrayEnd();
		Writer->WriteArrayStart(TEXT("permissions"));
			Writer->WriteObjectStart();
			if (Worker.WorkerPermissions.bAllPermissions)
			{
				Writer->WriteObjectStart(TEXT("all"));
				Writer->WriteObjectEnd();
			}
			else
			{
				Writer->WriteObjectStart(TEXT("entity_creation"));
					Writer->WriteValue(TEXT("allow"), Worker.WorkerPermissions.bAllowEntityCreation);
				Writer->WriteObjectEnd();
				Writer->WriteObjectStart(TEXT("entity_deletion"));
					Writer->WriteValue(TEXT("allow"), Worker.WorkerPermissions.bAllowEntityDeletion);
				Writer->WriteObjectEnd();
				Writer->WriteObjectStart(TEXT("entity_query"));
					Writer->WriteValue(TEXT("allow"), Worker.WorkerPermissions.bAllowEntityQuery);
					Writer->WriteArrayStart("components");
					for (const FString& Component : Worker.WorkerPermissions.Components)
					{
						Writer->WriteValue(Component);
					}
					Writer->WriteArrayEnd();
				Writer->WriteObjectEnd();
			}
			Writer->WriteObjectEnd();
		Writer->WriteArrayEnd();
		if (Worker.MaxConnectionCapacityLimit > 0)
		{
			Writer->WriteObjectStart(TEXT("connection_capacity_limit"));
				Writer->WriteValue(TEXT("max_capacity"), Worker.MaxConnectionCapacityLimit);
			Writer->WriteObjectEnd();
		}
		if (Worker.bLoginRateLimitEnabled)
		{
			Writer->WriteObjectStart(TEXT("login_rate_limit"));
				Writer->WriteValue(TEXT("duration"), Worker.LoginRateLimit.Duration);
				Writer->WriteValue(TEXT("requests_per_duration"), Worker.LoginRateLimit.RequestsPerDuration);
			Writer->WriteObjectEnd();
		}
	Writer->WriteObjectEnd();

	return true;
}

bool FSpatialGDKEditorToolbarModule::WriteLoadbalancingSection(TSharedRef< TJsonWriter<> > Writer, const FString& WorkerType, const int32 Columns, const int32 Rows, const bool ManualWorkerConnectionOnly) const
{
	Writer->WriteObjectStart();
	Writer->WriteValue(TEXT("layer"), WorkerType);
		Writer->WriteObjectStart("rectangle_grid");
			Writer->WriteValue(TEXT("cols"), Columns);
			Writer->WriteValue(TEXT("rows"), Rows);
		Writer->WriteObjectEnd();
		Writer->WriteObjectStart(TEXT("options"));
			Writer->WriteValue(TEXT("manual_worker_connection_only"), ManualWorkerConnectionOnly);
		Writer->WriteObjectEnd();
	Writer->WriteObjectEnd();

	return true;
}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FSpatialGDKEditorToolbarModule, SpatialGDKEditorToolbar)
