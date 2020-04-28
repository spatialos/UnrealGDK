// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "SpatialGDKEditorToolbar.h"

#include "Async/Async.h"
#include "Editor.h"
#include "Editor/EditorEngine.h"
#include "EditorStyleSet.h"
#include "Framework/Application/SlateApplication.h"
#include "Framework/MultiBox/MultiBoxBuilder.h"
#include "Framework/Notifications/NotificationManager.h"
#include "HAL/PlatformFilemanager.h"
#include "Interfaces/IProjectManager.h"
#include "IOSRuntimeSettings.h"
#include "ISettingsContainer.h"
#include "ISettingsModule.h"
#include "ISettingsSection.h"
#include "LevelEditor.h"
#include "Misc/MessageDialog.h"
#include "SpatialGDKEditorToolbarCommands.h"
#include "SpatialGDKEditorToolbarStyle.h"
#include "Widgets/DeclarativeSyntaxSupport.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Notifications/SNotificationList.h"

#include "SpatialCommandUtils.h"
#include "SpatialConstants.h"
#include "SpatialGDKDefaultLaunchConfigGenerator.h"
#include "SpatialGDKDefaultWorkerJsonGenerator.h"
#include "SpatialGDKEditor.h"
#include "SpatialGDKEditorSchemaGenerator.h"
#include "SpatialGDKEditorSettings.h"
#include "SpatialGDKServicesConstants.h"
#include "SpatialGDKServicesModule.h"
#include "SpatialGDKSettings.h"
#include "SpatialGDKEditorPackageAssembly.h"
#include "SpatialGDKEditorSnapshotGenerator.h"
#include "SpatialGDKSimulatedPlayerDeployment.h"
#include "Utils/LaunchConfigEditor.h"

#include "Editor/EditorEngine.h"
#include "HAL/FileManager.h"
#include "Sound/SoundBase.h"

#include "AssetRegistryModule.h"
#include "GeneralProjectSettings.h"
#include "LevelEditor.h"
#include "Misc/FileHelper.h"
#include "EngineClasses/SpatialWorldSettings.h"
#include "EditorExtension/LBStrategyEditorExtension.h"
#include "LoadBalancing/AbstractLBStrategy.h"
#include "SpatialCommandUtils.h"
#include "SpatialGDKEditorModule.h"

DEFINE_LOG_CATEGORY(LogSpatialGDKEditorToolbar);

#define LOCTEXT_NAMESPACE "FSpatialGDKEditorToolbarModule"

FSpatialGDKEditorToolbarModule::FSpatialGDKEditorToolbarModule()
: bStopSpatialOnExit(false)
, bSchemaBuildError(false)
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

	// load sounds
	ExecutionStartSound = LoadObject<USoundBase>(nullptr, TEXT("/Engine/EditorSounds/Notifications/CompileStart_Cue.CompileStart_Cue"));
	ExecutionStartSound->AddToRoot();
	ExecutionSuccessSound = LoadObject<USoundBase>(nullptr, TEXT("/Engine/EditorSounds/Notifications/CompileSuccess_Cue.CompileSuccess_Cue"));
	ExecutionSuccessSound->AddToRoot();
	ExecutionFailSound = LoadObject<USoundBase>(nullptr, TEXT("/Engine/EditorSounds/Notifications/CompileFailed_Cue.CompileFailed_Cue"));
	ExecutionFailSound->AddToRoot();
	SpatialGDKEditorInstance = MakeShareable(new FSpatialGDKEditor());

	const USpatialGDKEditorSettings* SpatialGDKEditorSettings = GetDefault<USpatialGDKEditorSettings>();

	OnPropertyChangedDelegateHandle = FCoreUObjectDelegates::OnObjectPropertyChanged.AddRaw(this, &FSpatialGDKEditorToolbarModule::OnPropertyChanged);
	bStopSpatialOnExit = SpatialGDKEditorSettings->bStopSpatialOnExit;

	FSpatialGDKServicesModule& GDKServices = FModuleManager::GetModuleChecked<FSpatialGDKServicesModule>("SpatialGDKServices");
	LocalDeploymentManager = GDKServices.GetLocalDeploymentManager();
	LocalDeploymentManager->PreInit(GetDefault<USpatialGDKSettings>()->IsRunningInChina());

	LocalDeploymentManager->SetAutoDeploy(SpatialGDKEditorSettings->bAutoStartLocalDeployment);

	// Bind the play button delegate to starting a local spatial deployment.
	if (!UEditorEngine::TryStartSpatialDeployment.IsBound() && SpatialGDKEditorSettings->bAutoStartLocalDeployment)
	{
		UEditorEngine::TryStartSpatialDeployment.BindLambda([this]
		{
			VerifyAndStartDeployment();
		});
	}

	FEditorDelegates::PreBeginPIE.AddLambda([this](bool bIsSimulatingInEditor)
	{
		if (GIsAutomationTesting && GetDefault<UGeneralProjectSettings>()->UsesSpatialNetworking())
		{
			LocalDeploymentManager->IsServiceRunningAndInCorrectDirectory();
			LocalDeploymentManager->GetLocalDeploymentStatus();

			VerifyAndStartDeployment();
		}
	});

	FEditorDelegates::EndPIE.AddLambda([this](bool bIsSimulatingInEditor)
	{
		if (GIsAutomationTesting && GetDefault<UGeneralProjectSettings>()->UsesSpatialNetworking())
		{
			LocalDeploymentManager->TryStopLocalDeployment();
		}
	});

	LocalDeploymentManager->Init(GetOptionalExposedRuntimeIP());
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
		LocalDeploymentManager->TryStopLocalDeployment();
	}
}

void FSpatialGDKEditorToolbarModule::Tick(float DeltaTime)
{
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
		FSpatialGDKEditorToolbarCommands::Get().DeleteSchemaDatabase,
		FExecuteAction::CreateRaw(this, &FSpatialGDKEditorToolbarModule::DeleteSchemaDatabaseButtonClicked));

	InPluginCommands->MapAction(
		FSpatialGDKEditorToolbarCommands::Get().CreateSpatialGDKSnapshot,
		FExecuteAction::CreateRaw(this, &FSpatialGDKEditorToolbarModule::CreateSnapshotButtonClicked),
		FCanExecuteAction::CreateRaw(this, &FSpatialGDKEditorToolbarModule::CanExecuteSnapshotGenerator));

	InPluginCommands->MapAction(
		FSpatialGDKEditorToolbarCommands::Get().StartSpatialDeployment,
		FExecuteAction::CreateRaw(this, &FSpatialGDKEditorToolbarModule::StartSpatialDeploymentButtonClicked),
		FCanExecuteAction::CreateRaw(this, &FSpatialGDKEditorToolbarModule::StartSpatialDeploymentCanExecute),
		FIsActionChecked(),
		FIsActionButtonVisible::CreateRaw(this, &FSpatialGDKEditorToolbarModule::StartSpatialDeploymentIsVisible));

	InPluginCommands->MapAction(
		FSpatialGDKEditorToolbarCommands::Get().StopSpatialDeployment,
		FExecuteAction::CreateRaw(this, &FSpatialGDKEditorToolbarModule::StopSpatialDeploymentButtonClicked),
		FCanExecuteAction::CreateRaw(this, &FSpatialGDKEditorToolbarModule::StopSpatialDeploymentCanExecute),
		FIsActionChecked(),
		FIsActionButtonVisible::CreateRaw(this, &FSpatialGDKEditorToolbarModule::StopSpatialDeploymentIsVisible));

	InPluginCommands->MapAction(
		FSpatialGDKEditorToolbarCommands::Get().LaunchInspectorWebPageAction,
		FExecuteAction::CreateRaw(this, &FSpatialGDKEditorToolbarModule::LaunchInspectorWebpageButtonClicked),
		FCanExecuteAction());

	InPluginCommands->MapAction(
		FSpatialGDKEditorToolbarCommands::Get().QuickDeployAction,
		FExecuteAction::CreateRaw(this, &FSpatialGDKEditorToolbarModule::LaunchOrShowDeployment),
		FCanExecuteAction());

	InPluginCommands->MapAction(
		FSpatialGDKEditorToolbarCommands::Get().EnableBuildClientWorker,
		FExecuteAction::CreateRaw(this, &FSpatialGDKEditorToolbarModule::OnCheckedBuildClientWorker),
		FCanExecuteAction(),
		FIsActionChecked::CreateRaw(this, &FSpatialGDKEditorToolbarModule::IsBuildClientWorkerEnabled));

	InPluginCommands->MapAction(
		FSpatialGDKEditorToolbarCommands::Get().EnableBuildSimulatedPlayer,
		FExecuteAction::CreateRaw(this, &FSpatialGDKEditorToolbarModule::OnCheckedSimulatedPlayers),
		FCanExecuteAction(),
		FIsActionChecked::CreateRaw(this, &FSpatialGDKEditorToolbarModule::IsSimulatedPlayersEnabled));

	InPluginCommands->MapAction(
		FSpatialGDKEditorToolbarCommands::Get().OpenSimulatedPlayerConfigurationWindowAction,
		FExecuteAction::CreateRaw(this, &FSpatialGDKEditorToolbarModule::ShowSimulatedPlayerDeploymentDialog),
		FCanExecuteAction());

	InPluginCommands->MapAction(
		FSpatialGDKEditorToolbarCommands::Get().OpenLaunchConfigurationEditorAction,
		FExecuteAction::CreateRaw(this, &FSpatialGDKEditorToolbarModule::OpenLaunchConfigurationEditor),
		FCanExecuteAction());
	
	InPluginCommands->MapAction(
		FSpatialGDKEditorToolbarCommands::Get().StartSpatialService,
		FExecuteAction::CreateRaw(this, &FSpatialGDKEditorToolbarModule::StartSpatialServiceButtonClicked),
		FCanExecuteAction::CreateRaw(this, &FSpatialGDKEditorToolbarModule::StartSpatialServiceCanExecute),
		FIsActionChecked(),
		FIsActionButtonVisible::CreateRaw(this, &FSpatialGDKEditorToolbarModule::StartSpatialServiceIsVisible));

	InPluginCommands->MapAction(
		FSpatialGDKEditorToolbarCommands::Get().StopSpatialService,
		FExecuteAction::CreateRaw(this, &FSpatialGDKEditorToolbarModule::StopSpatialServiceButtonClicked),
		FCanExecuteAction::CreateRaw(this, &FSpatialGDKEditorToolbarModule::StopSpatialServiceCanExecute),
		FIsActionChecked(),
		FIsActionButtonVisible::CreateRaw(this, &FSpatialGDKEditorToolbarModule::StopSpatialServiceIsVisible));

	InPluginCommands->MapAction(FSpatialGDKEditorToolbarCommands::Get().EnableSpatialNetworking,
		FExecuteAction::CreateRaw(this, &FSpatialGDKEditorToolbarModule::OnToggleSpatialNetworking),
		FCanExecuteAction(),
		FIsActionChecked::CreateRaw(this, &FSpatialGDKEditorToolbarModule::OnIsSpatialNetworkingEnabled)
	);

	InPluginCommands->MapAction(FSpatialGDKEditorToolbarCommands::Get().NoAutomaticConnection,
		FExecuteAction::CreateRaw(this, &FSpatialGDKEditorToolbarModule::NoAutomaticConnectionClicked),
		FCanExecuteAction::CreateRaw(this, &FSpatialGDKEditorToolbarModule::IsSpatialOSNetFlowConfigurable),
		FIsActionChecked::CreateRaw(this, &FSpatialGDKEditorToolbarModule::IsNoAutomaticConnectionSelected)
	);

	InPluginCommands->MapAction(FSpatialGDKEditorToolbarCommands::Get().LocalDeployment,
		FExecuteAction::CreateRaw(this, &FSpatialGDKEditorToolbarModule::LocalDeploymentClicked),
		FCanExecuteAction::CreateRaw(this, &FSpatialGDKEditorToolbarModule::OnIsSpatialNetworkingEnabled),
		FIsActionChecked::CreateRaw(this, &FSpatialGDKEditorToolbarModule::IsLocalDeploymentSelected)
	);

	InPluginCommands->MapAction(FSpatialGDKEditorToolbarCommands::Get().CloudDeployment,
		FExecuteAction::CreateRaw(this, &FSpatialGDKEditorToolbarModule::CloudDeploymentClicked),
		FCanExecuteAction::CreateRaw(this, &FSpatialGDKEditorToolbarModule::IsSpatialOSNetFlowConfigurable),
		FIsActionChecked::CreateRaw(this, &FSpatialGDKEditorToolbarModule::IsCloudDeploymentSelected)
	);

	InPluginCommands->MapAction(FSpatialGDKEditorToolbarCommands::Get().GDKEditorSettings,
		FExecuteAction::CreateRaw(this, &FSpatialGDKEditorToolbarModule::GDKEditorSettingsClicked)
	);
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
		Builder.AddMenuEntry(FSpatialGDKEditorToolbarCommands::Get().StartSpatialDeployment);
		Builder.AddMenuEntry(FSpatialGDKEditorToolbarCommands::Get().StopSpatialDeployment);
		Builder.AddMenuEntry(FSpatialGDKEditorToolbarCommands::Get().LaunchInspectorWebPageAction);
#if PLATFORM_WINDOWS
		Builder.AddMenuEntry(FSpatialGDKEditorToolbarCommands::Get().QuickDeployAction);
#endif
		Builder.AddMenuEntry(FSpatialGDKEditorToolbarCommands::Get().StartSpatialService);
		Builder.AddMenuEntry(FSpatialGDKEditorToolbarCommands::Get().StopSpatialService);
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
	Builder.AddToolBarButton(FSpatialGDKEditorToolbarCommands::Get().StartSpatialDeployment);
	Builder.AddToolBarButton(FSpatialGDKEditorToolbarCommands::Get().StopSpatialDeployment);
	Builder.AddComboButton(
		FUIAction(),
		FOnGetContent::CreateRaw(this, &FSpatialGDKEditorToolbarModule::CreateStartDropDownMenuContent),
		LOCTEXT("StartDropDownMenu_Lable", "SpatialOS Network Options"),
		TAttribute<FText>(),
		FSlateIcon(FEditorStyle::GetStyleSetName(), "GDK.start"),
		true
	);
	Builder.AddToolBarButton(FSpatialGDKEditorToolbarCommands::Get().LaunchInspectorWebPageAction);
#if PLATFORM_WINDOWS
	Builder.AddToolBarButton(FSpatialGDKEditorToolbarCommands::Get().QuickDeployAction);
	Builder.AddComboButton(
		FUIAction(),
		FOnGetContent::CreateRaw(this, &FSpatialGDKEditorToolbarModule::CreateLaunchDeploymentMenuContent),
		LOCTEXT("GDKDeploymentCombo_Label", "Deployment Tools"),
		TAttribute<FText>(),
		FSlateIcon(FEditorStyle::GetStyleSetName(), "GDK.Cloud"),
		true
	);
#endif
	Builder.AddToolBarButton(FSpatialGDKEditorToolbarCommands::Get().StartSpatialService);
	Builder.AddToolBarButton(FSpatialGDKEditorToolbarCommands::Get().StopSpatialService);
}

TSharedRef<SWidget> FSpatialGDKEditorToolbarModule::CreateGenerateSchemaMenuContent()
{
	FMenuBuilder MenuBuilder(true /*bInShouldCloseWindowAfterMenuSelection*/, PluginCommands);
	MenuBuilder.BeginSection(NAME_None, LOCTEXT("GDKSchemaOptionsHeader", "Schema Generation"));
	{
		MenuBuilder.AddMenuEntry(FSpatialGDKEditorToolbarCommands::Get().CreateSpatialGDKSchemaFull);
		MenuBuilder.AddMenuEntry(FSpatialGDKEditorToolbarCommands::Get().DeleteSchemaDatabase);
	}
	MenuBuilder.EndSection();

	return MenuBuilder.MakeWidget();
}

TSharedRef<SWidget> FSpatialGDKEditorToolbarModule::CreateLaunchDeploymentMenuContent()
{
	FMenuBuilder MenuBuilder(true /*bInShouldCloseWindowAfterMenuSelection*/, PluginCommands);
	MenuBuilder.BeginSection(NAME_None, LOCTEXT("GDKDeploymentOptionsHeader", "Deployment Tools"));
	{
		MenuBuilder.AddMenuEntry(FSpatialGDKEditorToolbarCommands::Get().OpenSimulatedPlayerConfigurationWindowAction);
		MenuBuilder.AddMenuEntry(FSpatialGDKEditorToolbarCommands::Get().EnableBuildClientWorker);
		MenuBuilder.AddMenuEntry(FSpatialGDKEditorToolbarCommands::Get().EnableBuildSimulatedPlayer);
		MenuBuilder.AddMenuEntry(FSpatialGDKEditorToolbarCommands::Get().OpenLaunchConfigurationEditorAction);
	}
	MenuBuilder.EndSection();

	return MenuBuilder.MakeWidget();
}

void OnLocalDeploymentIPChanged(const FText& InText, ETextCommit::Type InCommitType)
{
	USpatialGDKEditorSettings* SpatialGDKEditorSettings = GetMutableDefault<USpatialGDKEditorSettings>();
	SpatialGDKEditorSettings->ExposedRuntimeIP = InText.ToString();
	SpatialGDKEditorSettings->SaveConfig();
}

void OnCloudDeploymentNameChanged(const FText& InText, ETextCommit::Type InCommitType)
{
	USpatialGDKSettings* SpatialGDKSettings = GetMutableDefault<USpatialGDKSettings>();
	SpatialGDKSettings->DevelopmentDeploymentToConnect = InText.ToString();
	SpatialGDKSettings->SaveConfig();
}

TSharedRef<SWidget> FSpatialGDKEditorToolbarModule::CreateStartDropDownMenuContent()
{
	FMenuBuilder MenuBuilder(false /*bInShouldCloseWindowAfterMenuSelection*/, PluginCommands);
	UGeneralProjectSettings* GeneralProjectSettings = GetMutableDefault<UGeneralProjectSettings>();
	USpatialGDKSettings* SpatialGDKSettings = GetMutableDefault<USpatialGDKSettings>();
	MenuBuilder.BeginSection(NAME_None, LOCTEXT("SpatialOSSettings", "SpatialOS Settings"));
	{
		MenuBuilder.AddMenuEntry(FSpatialGDKEditorToolbarCommands::Get().EnableSpatialNetworking);
	}
	MenuBuilder.EndSection();

	MenuBuilder.BeginSection(NAME_None, LOCTEXT("Game Client", "Game Client"));
	{
		MenuBuilder.AddMenuEntry(FSpatialGDKEditorToolbarCommands::Get().LocalDeployment);
		MenuBuilder.AddMenuEntry(FSpatialGDKEditorToolbarCommands::Get().CloudDeployment);
		MenuBuilder.AddMenuEntry(FSpatialGDKEditorToolbarCommands::Get().NoAutomaticConnection);
	}
	MenuBuilder.EndSection();

	MenuBuilder.BeginSection(NAME_None, LOCTEXT("", ""));
	{
		MenuBuilder.AddWidget(SNew(SEditableText)
			.OnTextChanged_Static(OnLocalDeploymentIPChanged, ETextCommit::Default)
			.OnTextCommitted_Static(OnLocalDeploymentIPChanged)
			.Text(FText::FromString(GetDefault<USpatialGDKEditorSettings>()->ExposedRuntimeIP))
			.SelectAllTextWhenFocused(true)
			.ColorAndOpacity(FLinearColor::White * 0.8f)
			.IsEnabled_Raw(this, &FSpatialGDKEditorToolbarModule::IsLocalDeploymentIPEditable)
			.Font(FEditorStyle::GetFontStyle(TEXT("SourceControl.LoginWindow.Font")))
			.IsReadOnly(!GeneralProjectSettings->UsesSpatialNetworking()),
			LOCTEXT("LocalDeploymentIPLabel", "Local Deployment IP:")
		);

		MenuBuilder.AddWidget(SNew(SEditableText)
			.OnTextChanged_Static(OnCloudDeploymentNameChanged, ETextCommit::Default)
			.OnTextCommitted_Static(OnCloudDeploymentNameChanged)
			.Text(FText::FromString(SpatialGDKSettings->DevelopmentDeploymentToConnect))
			.SelectAllTextWhenFocused(true)
			.ColorAndOpacity(FLinearColor::White * 0.8f)
			.IsEnabled_Raw(this, &FSpatialGDKEditorToolbarModule::IsCloudDeploymentNameEditable)
			.Font(FEditorStyle::GetFontStyle(TEXT("SourceControl.LoginWindow.Font")))
			.IsReadOnly(!GeneralProjectSettings->UsesSpatialNetworking()),
			LOCTEXT("CloudDeploymentNameLabel", "Cloud Deployment Name:")
		);
	}
	MenuBuilder.EndSection();

	MenuBuilder.BeginSection(NAME_None, LOCTEXT("", ""));
	{
		MenuBuilder.AddMenuEntry(FSpatialGDKEditorToolbarCommands::Get().GDKEditorSettings);
	}
	MenuBuilder.EndSection();
	
	return MenuBuilder.MakeWidget();
}

void FSpatialGDKEditorToolbarModule::CreateSnapshotButtonClicked()
{
	OnShowTaskStartNotification("Started snapshot generation");

	const USpatialGDKEditorSettings* Settings = GetDefault<USpatialGDKEditorSettings>();

	SpatialGDKEditorInstance->GenerateSnapshot(
		GEditor->GetEditorWorldContext().World(), Settings->GetSpatialOSSnapshotToSave(),
		FSimpleDelegate::CreateLambda([this]() { OnShowSuccessNotification("Snapshot successfully generated!"); }),
		FSimpleDelegate::CreateLambda([this]() { OnShowFailedNotification("Snapshot generation failed!"); }),
		FSpatialGDKEditorErrorHandler::CreateLambda([](FString ErrorText) { FMessageDialog::Debugf(FText::FromString(ErrorText)); }));
}

void FSpatialGDKEditorToolbarModule::DeleteSchemaDatabaseButtonClicked()
{
	if (FMessageDialog::Open(EAppMsgType::YesNo, LOCTEXT("DeleteSchemaDatabasePrompt", "Are you sure you want to delete the schema database?")) == EAppReturnType::Yes)
	{
		OnShowTaskStartNotification(TEXT("Deleting schema database"));
		if (SpatialGDKEditor::Schema::DeleteSchemaDatabase(SpatialConstants::SCHEMA_DATABASE_FILE_PATH))
		{
			OnShowSuccessNotification(TEXT("Schema database deleted"));
		}
		else
		{
			OnShowFailedNotification(TEXT("Failed to delete schema database"));
		}
	}
}

void FSpatialGDKEditorToolbarModule::SchemaGenerateButtonClicked()
{
	GenerateSchema(false);
}

void FSpatialGDKEditorToolbarModule::SchemaGenerateFullButtonClicked()
{
	GenerateSchema(true);
}		

void FSpatialGDKEditorToolbarModule::OnShowTaskStartNotification(const FString& NotificationText)
{
	AsyncTask(ENamedThreads::GameThread, [NotificationText]
	{
		if (FSpatialGDKEditorToolbarModule* Module = FModuleManager::GetModulePtr<FSpatialGDKEditorToolbarModule>("SpatialGDKEditorToolbar"))
		{
			Module->ShowTaskStartNotification(NotificationText);
		}
	});
}

void FSpatialGDKEditorToolbarModule::ShowTaskStartNotification(const FString& NotificationText)
{
	// If a task notification already exists then expire it.
	if (TaskNotificationPtr.IsValid())
	{
		TaskNotificationPtr.Pin()->ExpireAndFadeout();
	}

	if (GEditor && ExecutionStartSound)
	{
		GEditor->PlayEditorSound(ExecutionStartSound);
	}

	FNotificationInfo Info(FText::AsCultureInvariant(NotificationText));
	Info.Image = FSpatialGDKEditorToolbarStyle::Get().GetBrush(TEXT("SpatialGDKEditorToolbar.SpatialOSLogo"));
	Info.ExpireDuration = 5.0f;
	Info.bFireAndForget = false;

	TaskNotificationPtr = FSlateNotificationManager::Get().AddNotification(Info);

	if (TaskNotificationPtr.IsValid())
	{
		TaskNotificationPtr.Pin()->SetCompletionState(SNotificationItem::CS_Pending);
	}
}

void FSpatialGDKEditorToolbarModule::OnShowSuccessNotification(const FString& NotificationText)
{
	AsyncTask(ENamedThreads::GameThread, [NotificationText]
	{
		if (FSpatialGDKEditorToolbarModule* Module = FModuleManager::GetModulePtr<FSpatialGDKEditorToolbarModule>("SpatialGDKEditorToolbar"))
		{
			Module->ShowSuccessNotification(NotificationText);
		}
	});
}

void FSpatialGDKEditorToolbarModule::ShowSuccessNotification(const FString& NotificationText)
{
	TSharedPtr<SNotificationItem> Notification = TaskNotificationPtr.Pin();
	if (Notification.IsValid())
	{
		Notification->SetFadeInDuration(0.1f);
		Notification->SetFadeOutDuration(0.5f);
		Notification->SetExpireDuration(5.0f);
		Notification->SetText(FText::AsCultureInvariant(NotificationText));
		Notification->SetCompletionState(SNotificationItem::CS_Success);
		Notification->ExpireAndFadeout();

		if (GEditor && ExecutionSuccessSound)
		{
			GEditor->PlayEditorSound(ExecutionSuccessSound);
		}
	}
}

void FSpatialGDKEditorToolbarModule::OnShowFailedNotification(const FString& NotificationText)
{
	AsyncTask(ENamedThreads::GameThread, [NotificationText]
	{
		if (FSpatialGDKEditorToolbarModule* Module = FModuleManager::GetModulePtr<FSpatialGDKEditorToolbarModule>("SpatialGDKEditorToolbar"))
		{
			Module->ShowFailedNotification(NotificationText);
		}
	});
}

void FSpatialGDKEditorToolbarModule::ShowFailedNotification(const FString& NotificationText)
{
	TSharedPtr<SNotificationItem> Notification = TaskNotificationPtr.Pin();
	if (Notification.IsValid())
	{
		Notification->SetFadeInDuration(0.1f);
		Notification->SetFadeOutDuration(0.5f);
		Notification->SetExpireDuration(5.0);
		Notification->SetText(FText::AsCultureInvariant(NotificationText));
		Notification->SetCompletionState(SNotificationItem::CS_Fail);
		Notification->ExpireAndFadeout();

		if (GEditor && ExecutionFailSound)
		{
			GEditor->PlayEditorSound(ExecutionFailSound);
		}
	}
}

void FSpatialGDKEditorToolbarModule::StartSpatialServiceButtonClicked()
{
	AsyncTask(ENamedThreads::AnyBackgroundThreadNormalTask, [this]
	{
		FDateTime StartTime = FDateTime::Now();
		OnShowTaskStartNotification(TEXT("Starting spatial service..."));

		// If the runtime IP is to be exposed, pass it to the spatial service on startup
		const USpatialGDKEditorSettings* SpatialGDKSettings = GetDefault<USpatialGDKEditorSettings>();
		const bool bSpatialServiceStarted = LocalDeploymentManager->TryStartSpatialService(GetOptionalExposedRuntimeIP());
		if (!bSpatialServiceStarted)
		{
			OnShowFailedNotification(TEXT("Spatial service failed to start"));
			UE_LOG(LogSpatialGDKEditorToolbar, Error, TEXT("Could not start spatial service."));
			return;
		}

		FTimespan Span = FDateTime::Now() - StartTime;

		OnShowSuccessNotification(TEXT("Spatial service started!"));
		UE_LOG(LogSpatialGDKEditorToolbar, Log, TEXT("Spatial service started in %f seconds."), Span.GetTotalSeconds());
	});
}

void FSpatialGDKEditorToolbarModule::StopSpatialServiceButtonClicked()
{
	AsyncTask(ENamedThreads::AnyBackgroundThreadNormalTask, [this]
	{
		FDateTime StartTime = FDateTime::Now();
		OnShowTaskStartNotification(TEXT("Stopping spatial service..."));

		if (!LocalDeploymentManager->TryStopSpatialService())
		{
			OnShowFailedNotification(TEXT("Spatial service failed to stop"));
			UE_LOG(LogSpatialGDKEditorToolbar, Error, TEXT("Could not stop spatial service."));
			return;
		}

		FTimespan Span = FDateTime::Now() - StartTime;

		OnShowSuccessNotification(TEXT("Spatial service stopped!"));
		UE_LOG(LogSpatialGDKEditorToolbar, Log, TEXT("Spatial service stopped in %f secoonds."), Span.GetTotalSeconds());
	});
}

bool FSpatialGDKEditorToolbarModule::FillWorkerLaunchConfigFromWorldSettings(UWorld& World, FWorkerTypeLaunchSection& OutLaunchConfig, FIntPoint& OutWorldDimension)
{
	const ASpatialWorldSettings* WorldSettings = Cast<ASpatialWorldSettings>(World.GetWorldSettings());

	if (!WorldSettings)
	{
		UE_LOG(LogSpatialGDKEditorToolbar, Error, TEXT("Missing SpatialWorldSettings on map %s"), *World.GetMapName());
		return false;
	}

	if (!WorldSettings->LoadBalanceStrategy)
	{
		UE_LOG(LogSpatialGDKEditorToolbar, Error, TEXT("Missing Load balancing strategy on map %s"), *World.GetMapName());
		return false;
	}

	FSpatialGDKEditorModule& EditorModule = FModuleManager::GetModuleChecked<FSpatialGDKEditorModule>("SpatialGDKEditor");

	if (!EditorModule.GetLBStrategyExtensionManager().GetDefaultLaunchConfiguration(WorldSettings->LoadBalanceStrategy->GetDefaultObject<UAbstractLBStrategy>(), OutLaunchConfig, OutWorldDimension))
	{
		UE_LOG(LogSpatialGDKEditorToolbar, Error, TEXT("Could not get the number of worker to launch for load balancing strategy %s"), *WorldSettings->LoadBalanceStrategy->GetName());
		return false;
	}

	return true;
}

void FSpatialGDKEditorToolbarModule::VerifyAndStartDeployment()
{
	// Don't try and start a local deployment if spatial networking is disabled.
	if (!GetDefault<UGeneralProjectSettings>()->UsesSpatialNetworking())
	{
		UE_LOG(LogSpatialGDKEditorToolbar, Error, TEXT("Attempted to start a local deployment but spatial networking is disabled."));
		return;
	}

	if (!IsSnapshotGenerated())
	{
		UE_LOG(LogSpatialGDKEditorToolbar, Error, TEXT("Attempted to start a local deployment but snapshot is not generated."));
		return;
	}

	if (!IsSchemaGenerated())
	{
		UE_LOG(LogSpatialGDKEditorToolbar, Error, TEXT("Attempted to start a local deployment but schema is not generated."));
		return;
	}

	if (bSchemaBuildError)
	{
		UE_LOG(LogSpatialGDKEditorToolbar, Warning, TEXT("Schema did not previously compile correctly, you may be running a stale build."));

		EAppReturnType::Type Result = FMessageDialog::Open(EAppMsgType::YesNo, FText::FromString("Last schema generation failed or failed to run the schema compiler. Schema will most likely be out of date, which may lead to undefined behavior. Are you sure you want to continue?"));
		if (Result == EAppReturnType::No)
		{
			return;
		}
	}

	// Get the latest launch config.
	const USpatialGDKSettings* SpatialGDKSettings = GetDefault<USpatialGDKSettings>();
	const USpatialGDKEditorSettings* SpatialGDKEditorSettings = GetDefault<USpatialGDKEditorSettings>();

	FString LaunchConfig;
	if (SpatialGDKEditorSettings->bGenerateDefaultLaunchConfig)
	{
		bool bRedeployRequired = false;
		if (!GenerateAllDefaultWorkerJsons(bRedeployRequired))
		{
			return;
		}
		if (bRedeployRequired)
		{
			LocalDeploymentManager->SetRedeployRequired();
		}

		UWorld* EditorWorld = GEditor->GetEditorWorldContext().World();
		check(EditorWorld);

		LaunchConfig = FPaths::Combine(FPaths::ConvertRelativePathToFull(FPaths::ProjectIntermediateDir()), FString::Printf(TEXT("Improbable/%s_LocalLaunchConfig.json"), *EditorWorld->GetMapName()));

		FSpatialLaunchConfigDescription LaunchConfigDescription = SpatialGDKEditorSettings->LaunchConfigDesc;
		if (SpatialGDKSettings->bEnableUnrealLoadBalancer)
		{
			FIntPoint WorldDimensions;
			FWorkerTypeLaunchSection WorkerLaunch;

			if (FillWorkerLaunchConfigFromWorldSettings(*EditorWorld, WorkerLaunch, WorldDimensions))
			{
				LaunchConfigDescription.World.Dimensions = WorldDimensions;
				LaunchConfigDescription.ServerWorkers.Empty(SpatialGDKSettings->ServerWorkerTypes.Num());

				for (auto WorkerType : SpatialGDKSettings->ServerWorkerTypes)
				{
					LaunchConfigDescription.ServerWorkers.Add(WorkerLaunch);
					LaunchConfigDescription.ServerWorkers.Last().WorkerTypeName = WorkerType;
				}
			}
		}

		for (auto& WorkerLaunchSection : LaunchConfigDescription.ServerWorkers)
		{
			WorkerLaunchSection.bManualWorkerConnectionOnly = true;
		}

		if (!ValidateGeneratedLaunchConfig(LaunchConfigDescription))
		{
			return;
		}

		GenerateDefaultLaunchConfig(LaunchConfig, &LaunchConfigDescription);
		LaunchConfigDescription.SetLevelEditorPlaySettingsWorkerTypes();

		// Also create default launch config for cloud deployments.
		{
			for (auto& WorkerLaunchSection : LaunchConfigDescription.ServerWorkers)
			{
				WorkerLaunchSection.bManualWorkerConnectionOnly = false;
			}

			FString CloudLaunchConfig = FPaths::Combine(FPaths::ConvertRelativePathToFull(FPaths::ProjectIntermediateDir()), FString::Printf(TEXT("Improbable/%s_CloudLaunchConfig.json"), *EditorWorld->GetMapName()));
			GenerateDefaultLaunchConfig(CloudLaunchConfig, &LaunchConfigDescription);
		}
	}
	else
	{
		LaunchConfig = SpatialGDKEditorSettings->GetSpatialOSLaunchConfig();
	}

	const FString LaunchFlags = SpatialGDKEditorSettings->GetSpatialOSCommandLineLaunchFlags();
	const FString SnapshotName = SpatialGDKEditorSettings->GetSpatialOSSnapshotToLoad();
	const FString RuntimeVersion = SpatialGDKEditorSettings->GetSpatialOSRuntimeVersionForLocal();

	AsyncTask(ENamedThreads::AnyBackgroundThreadNormalTask, [this, LaunchConfig, LaunchFlags, SnapshotName, RuntimeVersion]
	{
		// If the last local deployment is still stopping then wait until it's finished.
		while (LocalDeploymentManager->IsDeploymentStopping())
		{
			FPlatformProcess::Sleep(0.1f);
		}

		// If schema or worker configurations have been changed then we must restart the deployment.
		if (LocalDeploymentManager->IsRedeployRequired() && LocalDeploymentManager->IsLocalDeploymentRunning())
		{
			UE_LOG(LogSpatialGDKEditorToolbar, Display, TEXT("Local deployment must restart."));
			OnShowTaskStartNotification(TEXT("Local deployment restarting.")); 
			LocalDeploymentManager->TryStopLocalDeployment();
		}
		else if (LocalDeploymentManager->IsLocalDeploymentRunning())
		{
			// A good local deployment is already running.
			return;
		}

		FLocalDeploymentManager::LocalDeploymentCallback CallBack = [this](bool bSuccess)
		{
			if (bSuccess)
			{
				OnShowSuccessNotification(TEXT("Local deployment started!"));
			}
			else
			{
				OnShowFailedNotification(TEXT("Local deployment failed to start"));
			}
		};

		OnShowTaskStartNotification(TEXT("Starting local deployment..."));
		LocalDeploymentManager->TryStartLocalDeployment(LaunchConfig, RuntimeVersion, LaunchFlags, SnapshotName, GetOptionalExposedRuntimeIP(), CallBack);
	});
}

void FSpatialGDKEditorToolbarModule::StartSpatialDeploymentButtonClicked()
{
	VerifyAndStartDeployment();
}

void FSpatialGDKEditorToolbarModule::StopSpatialDeploymentButtonClicked()
{
	AsyncTask(ENamedThreads::AnyBackgroundThreadNormalTask, [this]
	{
		OnShowTaskStartNotification(TEXT("Stopping local deployment..."));
		if (LocalDeploymentManager->TryStopLocalDeployment())
		{
			OnShowSuccessNotification(TEXT("Successfully stopped local deployment"));
		}
		else
		{
			OnShowFailedNotification(TEXT("Failed to stop local deployment!"));
		}
	});	
}

void FSpatialGDKEditorToolbarModule::LaunchInspectorWebpageButtonClicked()
{
	FString WebError;
	FPlatformProcess::LaunchURL(TEXT("http://localhost:31000/inspector"), TEXT(""), &WebError);
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

bool FSpatialGDKEditorToolbarModule::StartSpatialDeploymentIsVisible() const
{
	if (LocalDeploymentManager->IsSpatialServiceRunning())
	{
		return !LocalDeploymentManager->IsLocalDeploymentRunning();
	}
	else
	{
		return true;
	}
}

bool FSpatialGDKEditorToolbarModule::StartSpatialDeploymentCanExecute() const
{
	return !LocalDeploymentManager->IsDeploymentStarting() && GetDefault<UGeneralProjectSettings>()->UsesSpatialNetworking() && GetDefault<USpatialGDKEditorSettings>()->SpatialOSNetFlowType == ESpatialOSNetFlow::LocalDeployment;
}

bool FSpatialGDKEditorToolbarModule::StopSpatialDeploymentIsVisible() const
{
	return LocalDeploymentManager->IsSpatialServiceRunning() && LocalDeploymentManager->IsLocalDeploymentRunning();
}

bool FSpatialGDKEditorToolbarModule::StopSpatialDeploymentCanExecute() const
{
	return !LocalDeploymentManager->IsDeploymentStopping();
}

bool FSpatialGDKEditorToolbarModule::StartSpatialServiceIsVisible() const
{
	const USpatialGDKEditorSettings* SpatialGDKSettings = GetDefault<USpatialGDKEditorSettings>();

	return SpatialGDKSettings->bShowSpatialServiceButton && !LocalDeploymentManager->IsSpatialServiceRunning();
}

bool FSpatialGDKEditorToolbarModule::StartSpatialServiceCanExecute() const
{
	return !LocalDeploymentManager->IsServiceStarting();
}

bool FSpatialGDKEditorToolbarModule::StopSpatialServiceIsVisible() const
{
	const USpatialGDKEditorSettings* SpatialGDKSettings = GetDefault<USpatialGDKEditorSettings>();

	return SpatialGDKSettings->bShowSpatialServiceButton && LocalDeploymentManager->IsSpatialServiceRunning();
}

void FSpatialGDKEditorToolbarModule::OnToggleSpatialNetworking()
{
	UGeneralProjectSettings* GeneralProjectSettings = GetMutableDefault<UGeneralProjectSettings>();
	UProperty* SpatialNetworkingProperty = UGeneralProjectSettings::StaticClass()->FindPropertyByName(FName("bSpatialNetworking"));

	GeneralProjectSettings->SetUsesSpatialNetworking(!GeneralProjectSettings->UsesSpatialNetworking());
	GeneralProjectSettings->UpdateSinglePropertyInConfigFile(SpatialNetworkingProperty, GeneralProjectSettings->GetDefaultConfigFilename());
}

bool FSpatialGDKEditorToolbarModule::OnIsSpatialNetworkingEnabled() const
{
	return GetDefault<UGeneralProjectSettings>()->UsesSpatialNetworking();
}

void FSpatialGDKEditorToolbarModule::GDKEditorSettingsClicked() const
{
	FModuleManager::LoadModuleChecked<ISettingsModule>("Settings").ShowViewer("Project", "SpatialGDKEditor", "Editor Settings");
}

bool FSpatialGDKEditorToolbarModule::IsNoAutomaticConnectionSelected() const
{
	const USpatialGDKEditorSettings* SpatialGDKEditorSettings = GetDefault<USpatialGDKEditorSettings>();
	return SpatialGDKEditorSettings->SpatialOSNetFlowType == ESpatialOSNetFlow::NoAutomaticConnection;
}

bool FSpatialGDKEditorToolbarModule::IsLocalDeploymentSelected() const
{
	const USpatialGDKEditorSettings* SpatialGDKEditorSettings = GetDefault<USpatialGDKEditorSettings>();
	return SpatialGDKEditorSettings->SpatialOSNetFlowType == ESpatialOSNetFlow::LocalDeployment;
}

bool FSpatialGDKEditorToolbarModule::IsCloudDeploymentSelected() const
{
	const USpatialGDKEditorSettings* SpatialGDKEditorSettings = GetDefault<USpatialGDKEditorSettings>();
	return SpatialGDKEditorSettings->SpatialOSNetFlowType == ESpatialOSNetFlow::CloudDeployment;
}

bool FSpatialGDKEditorToolbarModule::IsSpatialOSNetFlowConfigurable() const
{
	return OnIsSpatialNetworkingEnabled() && !(LocalDeploymentManager->IsLocalDeploymentRunning());
}

void FSpatialGDKEditorToolbarModule::NoAutomaticConnectionClicked() const
{
	USpatialGDKEditorSettings* SpatialGDKEditorSettings = GetMutableDefault<USpatialGDKEditorSettings>();
	SpatialGDKEditorSettings->SpatialOSNetFlowType = ESpatialOSNetFlow::NoAutomaticConnection;
	SpatialGDKEditorSettings->SaveConfig();
}

void FSpatialGDKEditorToolbarModule::LocalDeploymentClicked() const
{
	USpatialGDKEditorSettings* SpatialGDKEditorSettings = GetMutableDefault<USpatialGDKEditorSettings>();
	SpatialGDKEditorSettings->SpatialOSNetFlowType = ESpatialOSNetFlow::LocalDeployment;
	SpatialGDKEditorSettings->SaveConfig();
}

void FSpatialGDKEditorToolbarModule::CloudDeploymentClicked() const
{
	USpatialGDKEditorSettings* SpatialGDKEditorSettings = GetMutableDefault<USpatialGDKEditorSettings>();
	SpatialGDKEditorSettings->SpatialOSNetFlowType = ESpatialOSNetFlow::CloudDeployment;
	USpatialGDKSettings* SpatialGDKSettings = GetMutableDefault<USpatialGDKSettings>();

	FString DevAuthToken;
	if (!SpatialCommandUtils::GenerateDevAuthToken(SpatialGDKSettings->IsRunningInChina(), DevAuthToken))
	{
		UE_LOG(LogSpatialGDKEditorToolbar, Error, TEXT("Failed to generate a development authentication token."));
	}
	SpatialGDKEditorSettings->DevelopmentAuthenticationToken = DevAuthToken;
	SpatialGDKEditorSettings->SaveConfig();
	SpatialGDKEditorSettings->SetRuntimeDevelopmentAuthenticationToken();
}

bool FSpatialGDKEditorToolbarModule::IsLocalDeploymentIPEditable() const
{
	const USpatialGDKEditorSettings* SpatialGDKEditorSettings = GetDefault<USpatialGDKEditorSettings>();
	return GetDefault<UGeneralProjectSettings>()->UsesSpatialNetworking() && (SpatialGDKEditorSettings->SpatialOSNetFlowType == ESpatialOSNetFlow::LocalDeployment);
}

bool FSpatialGDKEditorToolbarModule::IsCloudDeploymentNameEditable() const
{
	const USpatialGDKEditorSettings* SpatialGDKEditorSettings = GetDefault<USpatialGDKEditorSettings>();
	return GetDefault<UGeneralProjectSettings>()->UsesSpatialNetworking() && (SpatialGDKEditorSettings->SpatialOSNetFlowType == ESpatialOSNetFlow::CloudDeployment);
}

bool FSpatialGDKEditorToolbarModule::StopSpatialServiceCanExecute() const
{
	return !LocalDeploymentManager->IsServiceStopping();
}

void FSpatialGDKEditorToolbarModule::OnPropertyChanged(UObject* ObjectBeingModified, FPropertyChangedEvent& PropertyChangedEvent)
{
	if (USpatialGDKEditorSettings* Settings = Cast<USpatialGDKEditorSettings>(ObjectBeingModified))
	{
		FName PropertyName = PropertyChangedEvent.Property != nullptr
				? PropertyChangedEvent.Property->GetFName()
				: NAME_None;
		if (PropertyName.ToString() == TEXT("bStopSpatialOnExit"))
		{
			/*
			* This updates our own local copy of bStopSpatialOnExit as Settings change.
			* We keep the copy of the variable as all the USpatialGDKEditorSettings references get
			* cleaned before all the available callbacks that IModuleInterface exposes. This means that we can't access
			* this variable through its references after the engine is closed.
			*/
			bStopSpatialOnExit = Settings->bStopSpatialOnExit;
		}
		else if (PropertyName.ToString() == TEXT("bAutoStartLocalDeployment"))
		{
			// TODO: UNR-1776 Workaround for SpatialNetDriver requiring editor settings.
			LocalDeploymentManager->SetAutoDeploy(Settings->bAutoStartLocalDeployment);

			if (Settings->bAutoStartLocalDeployment)
			{
				// Bind the TryStartSpatialDeployment delegate if autostart is enabled.
				UEditorEngine::TryStartSpatialDeployment.BindLambda([this]
				{
					VerifyAndStartDeployment();
				});
			}
			else
			{
				// Unbind the TryStartSpatialDeployment if autostart is disabled.
				UEditorEngine::TryStartSpatialDeployment.Unbind();
			}
		}
	}
}

void FSpatialGDKEditorToolbarModule::ShowSimulatedPlayerDeploymentDialog()
{
	// Create and open the cloud configuration dialog
	SimulatedPlayerDeploymentWindowPtr = SNew(SWindow)
		.Title(LOCTEXT("SimulatedPlayerConfigurationTitle", "Cloud Deployment"))
		.HasCloseButton(true)
		.SupportsMaximize(false)
		.SupportsMinimize(false)
		.SizingRule(ESizingRule::Autosized);

	SimulatedPlayerDeploymentWindowPtr->SetContent(
		SNew(SBox)
		.WidthOverride(700.0f)
		[
			SAssignNew(SimulatedPlayerDeploymentConfigPtr, SSpatialGDKSimulatedPlayerDeployment)
			.SpatialGDKEditor(SpatialGDKEditorInstance)
			.ParentWindow(SimulatedPlayerDeploymentWindowPtr)
		]
	);

	FSlateApplication::Get().AddWindow(SimulatedPlayerDeploymentWindowPtr.ToSharedRef());
}

void FSpatialGDKEditorToolbarModule::OpenLaunchConfigurationEditor()
{
	ULaunchConfigurationEditor::LaunchTransientUObjectEditor<ULaunchConfigurationEditor>(TEXT("Launch Configuration Editor"));
}

void FSpatialGDKEditorToolbarModule::LaunchOrShowDeployment()
{
	if (CanLaunchDeployment())
	{
		OnLaunchDeployment();
		return;
	}

	ShowSimulatedPlayerDeploymentDialog();
}

void FSpatialGDKEditorToolbarModule::GenerateSchema(bool bFullScan)
{
	LocalDeploymentManager->SetRedeployRequired();

	bSchemaBuildError = false;

	if (SpatialGDKEditorInstance->FullScanRequired())
	{
		OnShowTaskStartNotification("Initial Schema Generation");

		if (SpatialGDKEditorInstance->GenerateSchema(true))
		{
			OnShowSuccessNotification("Initial Schema Generation completed!");
		}
		else
		{
			OnShowFailedNotification("Initial Schema Generation failed");
			bSchemaBuildError = true;
		}
	}
	else if (bFullScan)
	{
		OnShowTaskStartNotification("Generating Schema (Full)");

		if (SpatialGDKEditorInstance->GenerateSchema(true))
		{
			OnShowSuccessNotification("Full Schema Generation completed!");
		}
		else
		{
			OnShowFailedNotification("Full Schema Generation failed");
			bSchemaBuildError = true;
		}
	}
	else
	{
		OnShowTaskStartNotification("Generating Schema (Incremental)");

		if (SpatialGDKEditorInstance->GenerateSchema(false))
		{
			OnShowSuccessNotification("Incremental Schema Generation completed!");
		}
		else
		{
			OnShowFailedNotification("Incremental Schema Generation failed");
			bSchemaBuildError = true;
		}
	}
}

bool FSpatialGDKEditorToolbarModule::IsSnapshotGenerated() const
{
	const USpatialGDKEditorSettings* SpatialGDKSettings = GetDefault<USpatialGDKEditorSettings>();
	return FPaths::FileExists(SpatialGDKSettings->GetSpatialOSSnapshotToLoadPath());
}

bool FSpatialGDKEditorToolbarModule::IsSchemaGenerated() const
{
	FString DescriptorPath = FPaths::Combine(SpatialGDKServicesConstants::SpatialOSDirectory, TEXT("build/assembly/schema/schema.descriptor"));
	FString GdkFolderPath = FPaths::Combine(SpatialGDKServicesConstants::SpatialOSDirectory, TEXT("schema/unreal/gdk"));
	return FPaths::FileExists(DescriptorPath) && FPaths::DirectoryExists(GdkFolderPath) && SpatialGDKEditor::Schema::GeneratedSchemaDatabaseExists();
}

FString FSpatialGDKEditorToolbarModule::GetOptionalExposedRuntimeIP() const
{
	const USpatialGDKEditorSettings* SpatialGDKEditorSettings = GetDefault<USpatialGDKEditorSettings>();
	if (SpatialGDKEditorSettings->SpatialOSNetFlowType == ESpatialOSNetFlow::LocalDeployment && GetDefault<UGeneralProjectSettings>()->UsesSpatialNetworking())
	{
		return SpatialGDKEditorSettings->ExposedRuntimeIP;
	}
	else
	{
		return TEXT("");
	}
}

FReply FSpatialGDKEditorToolbarModule::OnLaunchDeployment()
{
	const USpatialGDKEditorSettings* SpatialGDKSettings = GetDefault<USpatialGDKEditorSettings>();

	if (!SpatialGDKSettings->IsDeploymentConfigurationValid())
	{
		OnShowFailedNotification(TEXT("Deployment configuration is not valid."));

		return FReply::Unhandled();
	}

	AddDeploymentTag(TEXT("dev_login"));
	
	if (SpatialGDKSettings->IsGenerateSchemaEnabled())
	{
		SpatialGDKEditorInstance->GenerateSchema(false);
	}

	if (SpatialGDKSettings->IsGenerateSnapshotEnabled())
	{
		SpatialGDKGenerateSnapshot(GEditor->GetEditorWorldContext().World(), SpatialGDKSettings->GetSpatialOSSnapshotToSave());
	}

	TSharedRef<FSpatialGDKPackageAssembly> PackageAssembly = SpatialGDKEditorInstance->GetPackageAssemblyRef();
	PackageAssembly->OnSuccess.BindRaw(this, &FSpatialGDKEditorToolbarModule::OnBuildSuccess);
	PackageAssembly->BuildAllAndUpload(
		SpatialGDKSettings->GetAssemblyName(),
		SpatialGDKSettings->AssemblyWindowsPlatform,
		SpatialGDKSettings->AssemblyBuildConfiguration,
		TEXT(""),
		SpatialGDKSettings->bForceAssemblyOverwrite
	);

	return FReply::Handled();
}

void FSpatialGDKEditorToolbarModule::OnBuildSuccess()
{
	const USpatialGDKEditorSettings* SpatialGDKSettings = GetDefault<USpatialGDKEditorSettings>();
	if (SpatialGDKSettings->IsSimulatedPlayersEnabled())
	{
		IPlatformFile& PlatformFile = FPlatformFileManager::Get().GetPlatformFile();
		FString BuiltWorkerFolder = GetDefault<USpatialGDKEditorSettings>()->GetBuiltWorkerFolder();
		FString BuiltSimPlayersName = TEXT("UnrealSimulatedPlayer@Linux.zip");
		FString BuiltSimPlayerPath = FPaths::Combine(BuiltWorkerFolder, BuiltSimPlayersName);

		if (!PlatformFile.FileExists(*BuiltSimPlayerPath))
		{
			FString MissingSimPlayerBuildText = FString::Printf(TEXT("Warning: Detected that %s is missing. To launch a successful SimPlayer deployment ensure that SimPlayers is built and uploaded.\n\nWould you still like to continue with the deployment?"), *BuiltSimPlayersName);
			EAppReturnType::Type UserAnswer = FMessageDialog::Open(EAppMsgType::YesNo, FText::FromString(MissingSimPlayerBuildText));
			if (UserAnswer == EAppReturnType::No || UserAnswer == EAppReturnType::Cancel)
			{
				OnShowFailedNotification("Failed to launch cloud deployment. SimulatedPlayer archive is missing.");
				return;
			}
		}
	}

	auto LaunchCloudDeployment = [this]()
	{
		SpatialGDKEditorInstance->LaunchCloudDeployment(
			FSimpleDelegate::CreateLambda([this]()
			{
				this->OnShowSuccessNotification("Successfully launched cloud deployment.");
			}),

			FSimpleDelegate::CreateLambda([this]()
			{
				this->OnShowFailedNotification("Failed to launch cloud deployment. See output logs for details.");
			})
		);
	};

#if ENGINE_MINOR_VERSION <= 22
	AttemptSpatialAuthResult = Async<bool>(EAsyncExecution::Thread, []() { return SpatialCommandUtils::AttemptSpatialAuth(GetDefault<USpatialGDKSettings>()->IsRunningInChina()); },
#else
	AttemptSpatialAuthResult = Async(EAsyncExecution::Thread, []() { return SpatialCommandUtils::AttemptSpatialAuth(GetDefault<USpatialGDKSettings>()->IsRunningInChina()); },
#endif
		[this, LaunchCloudDeployment]()
	{
		if (AttemptSpatialAuthResult.IsReady() && AttemptSpatialAuthResult.Get() == true)
		{
			LaunchCloudDeployment();
		}
		else
		{
			OnShowTaskStartNotification(TEXT("Spatial auth failed attempting to launch cloud deployment."));
		}
	});
}

bool FSpatialGDKEditorToolbarModule::IsDeploymentConfigurationValid() const
{
	const USpatialGDKEditorSettings* SpatialGDKSettings = GetDefault<USpatialGDKEditorSettings>();
	return SpatialGDKSettings->GetPrimaryDeploymentName().IsEmpty() || SpatialGDKSettings->GetAssemblyName().IsEmpty() ? false : true;
}

bool FSpatialGDKEditorToolbarModule::CanBuildAndUpload() const
{
	return SpatialGDKEditorInstance->GetPackageAssemblyRef()->CanBuild();
}

bool FSpatialGDKEditorToolbarModule::CanLaunchDeployment() const
{
	return IsDeploymentConfigurationValid() && CanBuildAndUpload();
}

bool FSpatialGDKEditorToolbarModule::IsSimulatedPlayersEnabled() const
{
	return GetDefault<USpatialGDKEditorSettings>()->IsSimulatedPlayersEnabled();
}

void FSpatialGDKEditorToolbarModule::OnCheckedSimulatedPlayers()
{
	GetMutableDefault<USpatialGDKEditorSettings>()->SetSimulatedPlayersEnabledState(!IsSimulatedPlayersEnabled());
}

bool FSpatialGDKEditorToolbarModule::IsBuildClientWorkerEnabled() const
{
	return GetDefault<USpatialGDKEditorSettings>()->IsBuildClientWorkerEnabled();
}

void FSpatialGDKEditorToolbarModule::OnCheckedBuildClientWorker()
{
	GetMutableDefault<USpatialGDKEditorSettings>()->SetBuildClientWorker(!IsBuildClientWorkerEnabled());
}

void FSpatialGDKEditorToolbarModule::AddDeploymentTag(const FString& Tag)
{
	USpatialGDKEditorSettings* SpatialGDKSettings = GetMutableDefault<USpatialGDKEditorSettings>();

	FString Tags = SpatialGDKSettings->GetDeploymentTags();
	TArray<FString> OutArray;
	Tags.ParseIntoArray(OutArray, TEXT(" "));
	bool HasDevLoginTag = false;
	for (INT i = 0; i < OutArray.Num(); ++i)
	{
		if (0 == OutArray[i].Trim().Compare(Tag))
		{
			HasDevLoginTag = true;
			break;
		}
	}

	if (false == HasDevLoginTag)
	{
		Tags += TEXT(" ");
		Tags += Tag;
		SpatialGDKSettings->SetDeploymentTags(Tags);
	}
}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FSpatialGDKEditorToolbarModule, SpatialGDKEditorToolbar)
