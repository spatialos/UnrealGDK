// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "SpatialGDKEditorToolbar.h"

#include "AssetRegistryModule.h"
#include "Async/Async.h"
#include "Editor.h"
#include "Editor/EditorEngine.h"
#include "EditorStyleSet.h"
#include "EngineClasses/SpatialWorldSettings.h"
#include "Framework/Application/SlateApplication.h"
#include "Framework/MultiBox/MultiBoxBuilder.h"
#include "Framework/Notifications/NotificationManager.h"
#include "GeneralProjectSettings.h"
#include "HAL/FileManager.h"
#include "HAL/PlatformFilemanager.h"
#include "Interfaces/IProjectManager.h"
#include "Internationalization/Regex.h"
#include "IOSRuntimeSettings.h"
#include "ISettingsContainer.h"
#include "ISettingsModule.h"
#include "ISettingsSection.h"
#include "LevelEditor.h"
#include "Misc/FileHelper.h"
#include "Misc/MessageDialog.h"
#include "Sound/SoundBase.h"
#include "Widgets/DeclarativeSyntaxSupport.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Notifications/SNotificationList.h"

#include "CloudDeploymentConfiguration.h"
#include "SpatialCommandUtils.h"
#include "SpatialConstants.h"
#include "SpatialGDKDefaultLaunchConfigGenerator.h"
#include "SpatialGDKDefaultWorkerJsonGenerator.h"
#include "SpatialGDKDevAuthTokenGenerator.h"
#include "SpatialGDKEditor.h"
#include "SpatialGDKEditorModule.h"
#include "SpatialGDKEditorSchemaGenerator.h"
#include "SpatialGDKEditorSettings.h"
#include "SpatialGDKServicesConstants.h"
#include "SpatialGDKServicesModule.h"
#include "SpatialGDKSettings.h"
#include "SpatialGDKEditorPackageAssembly.h"
#include "SpatialGDKEditorSnapshotGenerator.h"
#include "SpatialGDKEditorToolbarCommands.h"
#include "SpatialGDKEditorToolbarStyle.h"
#include "SpatialGDKCloudDeploymentConfiguration.h"
#include "SpatialRuntimeLoadBalancingStrategies.h"
#include "Utils/LaunchConfigEditor.h"

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

	const USpatialGDKEditorSettings* SpatialGDKEditorSettings = GetDefault<USpatialGDKEditorSettings>();

	OnPropertyChangedDelegateHandle = FCoreUObjectDelegates::OnObjectPropertyChanged.AddRaw(this, &FSpatialGDKEditorToolbarModule::OnPropertyChanged);
	bStopSpatialOnExit = SpatialGDKEditorSettings->bStopSpatialOnExit;

	FSpatialGDKServicesModule& GDKServices = FModuleManager::GetModuleChecked<FSpatialGDKServicesModule>("SpatialGDKServices");
	LocalDeploymentManager = GDKServices.GetLocalDeploymentManager();
	LocalDeploymentManager->PreInit(GetDefault<USpatialGDKSettings>()->IsRunningInChina());

	OnAutoStartLocalDeploymentChanged();

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
	SpatialGDKEditorInstance = FModuleManager::GetModuleChecked<FSpatialGDKEditorModule>("SpatialGDKEditor").GetSpatialGDKEditorInstance();
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
		FSpatialGDKEditorToolbarCommands::Get().StartNative,
		FExecuteAction(),
		FCanExecuteAction::CreateRaw(this, &FSpatialGDKEditorToolbarModule::StartNativeCanExecute),
		FIsActionChecked(),
		FIsActionButtonVisible::CreateRaw(this, &FSpatialGDKEditorToolbarModule::StartNativeIsVisible));

	InPluginCommands->MapAction(
		FSpatialGDKEditorToolbarCommands::Get().StartLocalSpatialDeployment,
		FExecuteAction::CreateRaw(this, &FSpatialGDKEditorToolbarModule::StartLocalSpatialDeploymentButtonClicked),
		FCanExecuteAction::CreateRaw(this, &FSpatialGDKEditorToolbarModule::StartLocalSpatialDeploymentCanExecute),
		FIsActionChecked(),
		FIsActionButtonVisible::CreateRaw(this, &FSpatialGDKEditorToolbarModule::StartLocalSpatialDeploymentIsVisible));

	InPluginCommands->MapAction(
		FSpatialGDKEditorToolbarCommands::Get().StartCloudSpatialDeployment,
		FExecuteAction::CreateRaw(this, &FSpatialGDKEditorToolbarModule::LaunchOrShowCloudDeployment),
		FCanExecuteAction::CreateRaw(this, &FSpatialGDKEditorToolbarModule::StartCloudSpatialDeploymentCanExecute),
		FIsActionChecked(),
		FIsActionButtonVisible::CreateRaw(this, &FSpatialGDKEditorToolbarModule::StartCloudSpatialDeploymentIsVisible));

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
		FSpatialGDKEditorToolbarCommands::Get().EnableBuildClientWorker,
		FExecuteAction::CreateRaw(this, &FSpatialGDKEditorToolbarModule::OnCheckedBuildClientWorker),
		FCanExecuteAction::CreateRaw(this, &FSpatialGDKEditorToolbarModule::AreCloudDeploymentPropertiesEditable),
		FIsActionChecked::CreateRaw(this, &FSpatialGDKEditorToolbarModule::IsBuildClientWorkerEnabled));

	InPluginCommands->MapAction(
		FSpatialGDKEditorToolbarCommands::Get().EnableBuildSimulatedPlayer,
		FExecuteAction::CreateRaw(this, &FSpatialGDKEditorToolbarModule::OnCheckedSimulatedPlayers),
		FCanExecuteAction::CreateRaw(this, &FSpatialGDKEditorToolbarModule::AreCloudDeploymentPropertiesEditable),
		FIsActionChecked::CreateRaw(this, &FSpatialGDKEditorToolbarModule::IsSimulatedPlayersEnabled));

	InPluginCommands->MapAction(
		FSpatialGDKEditorToolbarCommands::Get().OpenCloudDeploymentWindowAction,
		FExecuteAction::CreateRaw(this, &FSpatialGDKEditorToolbarModule::ShowCloudDeploymentDialog),
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

	InPluginCommands->MapAction(FSpatialGDKEditorToolbarCommands::Get().GDKRuntimeSettings,
		FExecuteAction::CreateRaw(this, &FSpatialGDKEditorToolbarModule::GDKRuntimeSettingsClicked)
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
		Builder.AddMenuEntry(FSpatialGDKEditorToolbarCommands::Get().StartNative);
		Builder.AddMenuEntry(FSpatialGDKEditorToolbarCommands::Get().StartLocalSpatialDeployment);
		Builder.AddMenuEntry(FSpatialGDKEditorToolbarCommands::Get().StartCloudSpatialDeployment);
		Builder.AddMenuEntry(FSpatialGDKEditorToolbarCommands::Get().StopSpatialDeployment);
		Builder.AddMenuEntry(FSpatialGDKEditorToolbarCommands::Get().LaunchInspectorWebPageAction);
#if PLATFORM_WINDOWS
		Builder.AddMenuEntry(FSpatialGDKEditorToolbarCommands::Get().OpenCloudDeploymentWindowAction);
#endif
		Builder.AddMenuEntry(FSpatialGDKEditorToolbarCommands::Get().CreateSpatialGDKSchema);
		Builder.AddMenuEntry(FSpatialGDKEditorToolbarCommands::Get().CreateSpatialGDKSnapshot);
		Builder.AddMenuEntry(FSpatialGDKEditorToolbarCommands::Get().StartSpatialService);
		Builder.AddMenuEntry(FSpatialGDKEditorToolbarCommands::Get().StopSpatialService);
	}
	Builder.EndSection();
}

void FSpatialGDKEditorToolbarModule::AddToolbarExtension(FToolBarBuilder& Builder)
{
	Builder.AddSeparator(NAME_None);
	Builder.AddToolBarButton(FSpatialGDKEditorToolbarCommands::Get().StartNative);
	Builder.AddToolBarButton(FSpatialGDKEditorToolbarCommands::Get().StartLocalSpatialDeployment);
	Builder.AddToolBarButton(FSpatialGDKEditorToolbarCommands::Get().StartCloudSpatialDeployment);
	Builder.AddToolBarButton(FSpatialGDKEditorToolbarCommands::Get().StopSpatialDeployment);
	Builder.AddComboButton(
		FUIAction(),
		FOnGetContent::CreateRaw(this, &FSpatialGDKEditorToolbarModule::CreateStartDropDownMenuContent),
		LOCTEXT("StartDropDownMenu_Label", "SpatialOS Network Options"),
		TAttribute<FText>(),
		FSlateIcon(FEditorStyle::GetStyleSetName(), "GDK.Start"),
		true
	);
	Builder.AddToolBarButton(FSpatialGDKEditorToolbarCommands::Get().LaunchInspectorWebPageAction);
#if PLATFORM_WINDOWS
	Builder.AddToolBarButton(FSpatialGDKEditorToolbarCommands::Get().OpenCloudDeploymentWindowAction);
	Builder.AddComboButton(
		FUIAction(),
		FOnGetContent::CreateRaw(this, &FSpatialGDKEditorToolbarModule::CreateLaunchDeploymentMenuContent),
		LOCTEXT("GDKDeploymentCombo_Label", "Deployment Tools"),
		TAttribute<FText>(),
		FSlateIcon(FEditorStyle::GetStyleSetName(), "GDK.Cloud"),
		true
	);
#endif
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
		MenuBuilder.AddMenuEntry(FSpatialGDKEditorToolbarCommands::Get().OpenLaunchConfigurationEditorAction);
	}
	MenuBuilder.EndSection();

	return MenuBuilder.MakeWidget();
}

void OnLocalDeploymentIPChanged(const FText& InText, ETextCommit::Type InCommitType)
{
	if (InCommitType != ETextCommit::OnEnter && InCommitType != ETextCommit::OnUserMovedFocus)
	{
		return;
	}

	const FString& InputIpAddress = InText.ToString();
	const FRegexPattern IpV4PatternRegex(SpatialConstants::Ipv4Pattern);
	FRegexMatcher IpV4RegexMatcher(IpV4PatternRegex, InputIpAddress);
	if (!InputIpAddress.IsEmpty() && !IpV4RegexMatcher.FindNext())
	{
		FMessageDialog::Open(EAppMsgType::Ok, FText::FromString(TEXT("Please input a valid IP address.")));
		UE_LOG(LogSpatialGDKEditorToolbar, Error, TEXT("Invalid IP address: %s"), *InputIpAddress);
		return;
	}

	USpatialGDKEditorSettings* SpatialGDKEditorSettings = GetMutableDefault<USpatialGDKEditorSettings>();
	SpatialGDKEditorSettings->SetExposedRuntimeIP(InputIpAddress);
	UE_LOG(LogSpatialGDKEditorToolbar, Display, TEXT("Setting local deployment IP address to %s"), *InputIpAddress);
}

void OnCloudDeploymentNameChanged(const FText& InText, ETextCommit::Type InCommitType)
{
	if (InCommitType != ETextCommit::OnEnter && InCommitType != ETextCommit::OnUserMovedFocus)
	{
		return;
	}

	const FString& InputDeploymentName = InText.ToString();
	const FRegexPattern DeploymentNamePatternRegex(SpatialConstants::DeploymentPattern);
	FRegexMatcher DeploymentNameRegexMatcher(DeploymentNamePatternRegex, InputDeploymentName);
	if (!InputDeploymentName.IsEmpty() && !DeploymentNameRegexMatcher.FindNext())
	{
		FMessageDialog::Open(EAppMsgType::Ok, FText::FromString(FString::Printf(TEXT("Please input a valid deployment name. %s"), *SpatialConstants::DeploymentPatternHint)));
		UE_LOG(LogSpatialGDKEditorToolbar, Error, TEXT("Invalid deployment name: %s"), *InputDeploymentName);
		return;
	}

	USpatialGDKEditorSettings* SpatialGDKEditorSettings = GetMutableDefault<USpatialGDKEditorSettings>();
	SpatialGDKEditorSettings->SetDevelopmentDeploymentToConnect(InputDeploymentName);

	UE_LOG(LogSpatialGDKEditorToolbar, Display, TEXT("Setting cloud deployment name to %s"), *InputDeploymentName);
}

TSharedRef<SWidget> FSpatialGDKEditorToolbarModule::CreateStartDropDownMenuContent()
{
	FMenuBuilder MenuBuilder(false /*bInShouldCloseWindowAfterMenuSelection*/, PluginCommands);
	UGeneralProjectSettings* GeneralProjectSettings = GetMutableDefault<UGeneralProjectSettings>();
	USpatialGDKEditorSettings* SpatialGDKEditorSettings = GetMutableDefault<USpatialGDKEditorSettings>();
	MenuBuilder.BeginSection("SpatialOSSettings", LOCTEXT("SpatialOSSettingsLabel", "SpatialOS Settings"));
	{
		MenuBuilder.AddMenuEntry(FSpatialGDKEditorToolbarCommands::Get().EnableSpatialNetworking);
	}
	MenuBuilder.EndSection();

	MenuBuilder.BeginSection("ConnectionFlow", LOCTEXT("ConnectionFlowLabel", "Connection Flow"));
	{
		MenuBuilder.AddMenuEntry(FSpatialGDKEditorToolbarCommands::Get().LocalDeployment);
		MenuBuilder.AddMenuEntry(FSpatialGDKEditorToolbarCommands::Get().CloudDeployment);
	}
	MenuBuilder.EndSection();

	MenuBuilder.BeginSection("AdditionalProperties");
	{
		MenuBuilder.AddWidget(SNew(SEditableText)
			.OnTextCommitted_Static(OnLocalDeploymentIPChanged)
			.Text(FText::FromString(GetDefault<USpatialGDKEditorSettings>()->ExposedRuntimeIP))
			.SelectAllTextWhenFocused(true)
			.ColorAndOpacity(FLinearColor::White * 0.8f)
			.IsEnabled_Raw(this, &FSpatialGDKEditorToolbarModule::IsLocalDeploymentIPEditable)
			.Font(FEditorStyle::GetFontStyle(TEXT("SourceControl.LoginWindow.Font"))),
			LOCTEXT("LocalDeploymentIPLabel", "Local Deployment IP:")
		);

		MenuBuilder.AddWidget(SNew(SEditableText)
			.OnTextCommitted_Static(OnCloudDeploymentNameChanged)
			.Text(FText::FromString(SpatialGDKEditorSettings->DevelopmentDeploymentToConnect))
			.SelectAllTextWhenFocused(true)
			.ColorAndOpacity(FLinearColor::White * 0.8f)
			.IsEnabled_Raw(this, &FSpatialGDKEditorToolbarModule::AreCloudDeploymentPropertiesEditable)
			.Font(FEditorStyle::GetFontStyle(TEXT("SourceControl.LoginWindow.Font"))),
			LOCTEXT("CloudDeploymentNameLabel", "Cloud Deployment Name:")
		);
		MenuBuilder.AddMenuEntry(FSpatialGDKEditorToolbarCommands::Get().EnableBuildClientWorker);
		MenuBuilder.AddMenuEntry(FSpatialGDKEditorToolbarCommands::Get().EnableBuildSimulatedPlayer);
	}
	MenuBuilder.EndSection();

	MenuBuilder.BeginSection("SettingsShortcuts");
	{
		MenuBuilder.AddMenuEntry(FSpatialGDKEditorToolbarCommands::Get().GDKEditorSettings);
		MenuBuilder.AddMenuEntry(FSpatialGDKEditorToolbarCommands::Get().GDKRuntimeSettings);
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

void FSpatialGDKEditorToolbarModule::OnShowSingleFailureNotification(const FString& NotificationText)
{
	AsyncTask(ENamedThreads::GameThread, [NotificationText]
	{
		if (FSpatialGDKEditorToolbarModule* Module = FModuleManager::GetModulePtr<FSpatialGDKEditorToolbarModule>("SpatialGDKEditorToolbar"))
		{
			Module->ShowSingleFailureNotification(NotificationText);
		}
	});
}

void FSpatialGDKEditorToolbarModule::ShowSingleFailureNotification(const FString& NotificationText)
{
	// If a task notification already exists then expire it.
	if (TaskNotificationPtr.IsValid())
	{
		TaskNotificationPtr.Pin()->ExpireAndFadeout();
	}

	FNotificationInfo Info(FText::AsCultureInvariant(NotificationText));
	Info.Image = FSpatialGDKEditorToolbarStyle::Get().GetBrush(TEXT("SpatialGDKEditorToolbar.SpatialOSLogo"));
	Info.ExpireDuration = 5.0f;
	Info.bFireAndForget = false;

	TaskNotificationPtr = FSlateNotificationManager::Get().AddNotification(Info);
	ShowFailedNotification(NotificationText);
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
		USingleWorkerRuntimeStrategy* DefaultStrategy = USingleWorkerRuntimeStrategy::StaticClass()->GetDefaultObject<USingleWorkerRuntimeStrategy>();
		UAbstractRuntimeLoadBalancingStrategy* LoadBalancingStrat = DefaultStrategy;

		if (TryGetLoadBalancingStrategyFromWorldSettings(*EditorWorld, LoadBalancingStrat, LaunchConfigDescription.World.Dimensions))
		{
			LoadBalancingStrat->AddToRoot();
		}

		FWorkerTypeLaunchSection Conf = SpatialGDKEditorSettings->LaunchConfigDesc.ServerWorkerConfig;
		// Force manual connection to true as this is the config for PIE.
		Conf.bManualWorkerConnectionOnly = true;
		if (Conf.bAutoNumEditorInstances)
		{
			Conf.NumEditorInstances = GetWorkerCountFromWorldSettings(*EditorWorld);
		}

		if (!ValidateGeneratedLaunchConfig(LaunchConfigDescription, Conf))
		{
			return;
		}

		GenerateLaunchConfig(LaunchConfig, &LaunchConfigDescription, Conf);
		SetLevelEditorPlaySettingsWorkerType(Conf);

		// Also create default launch config for cloud deployments.
		{
			// Revert to the setting's flag value for manual connection.
			Conf.bManualWorkerConnectionOnly = SpatialGDKEditorSettings->LaunchConfigDesc.ServerWorkerConfig.bManualWorkerConnectionOnly;
			FString CloudLaunchConfig = FPaths::Combine(FPaths::ConvertRelativePathToFull(FPaths::ProjectIntermediateDir()), FString::Printf(TEXT("Improbable/%s_CloudLaunchConfig.json"), *EditorWorld->GetMapName()));
			GenerateLaunchConfig(CloudLaunchConfig, &LaunchConfigDescription, Conf);
		}

		if (LoadBalancingStrat != DefaultStrategy)
		{
			LoadBalancingStrat->RemoveFromRoot();
		}
	}
	else
	{
		LaunchConfig = SpatialGDKEditorSettings->GetSpatialOSLaunchConfig();

		SetLevelEditorPlaySettingsWorkerType(SpatialGDKEditorSettings->LaunchConfigDesc.ServerWorkerConfig);
	}

	const FString LaunchFlags = SpatialGDKEditorSettings->GetSpatialOSCommandLineLaunchFlags();
	const FString SnapshotName = SpatialGDKEditorSettings->GetSpatialOSSnapshotToLoad();
	const FString RuntimeVersion = SpatialGDKEditorSettings->GetSelectedRuntimeVariantVersion().GetVersionForLocal();

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

void FSpatialGDKEditorToolbarModule::StartLocalSpatialDeploymentButtonClicked()
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
	// Get the runtime variant currently being used as this affects which Inspector to use.
	FString InspectorURL;
	if (GetDefault<USpatialGDKEditorSettings>()->GetSpatialOSRuntimeVariant() == ESpatialOSRuntimeVariant::Standard)
	{
		InspectorURL = SpatialGDKServicesConstants::InspectorURL;
	}
	else
	{
		InspectorURL = SpatialGDKServicesConstants::InspectorV2URL;
	}

	FString WebError;
	FPlatformProcess::LaunchURL(*InspectorURL, TEXT(""), &WebError);
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

bool FSpatialGDKEditorToolbarModule::StartNativeIsVisible() const
{
	return !GetDefault<UGeneralProjectSettings>()->UsesSpatialNetworking();
}

bool FSpatialGDKEditorToolbarModule::StartNativeCanExecute() const
{
	return false;
}

bool FSpatialGDKEditorToolbarModule::StartLocalSpatialDeploymentIsVisible() const
{
	return !LocalDeploymentManager->IsLocalDeploymentRunning() && GetDefault<UGeneralProjectSettings>()->UsesSpatialNetworking() && GetDefault<USpatialGDKEditorSettings>()->SpatialOSNetFlowType == ESpatialOSNetFlow::LocalDeployment;
}

bool FSpatialGDKEditorToolbarModule::StartLocalSpatialDeploymentCanExecute() const
{
	return !LocalDeploymentManager->IsServiceStarting() && !LocalDeploymentManager->IsDeploymentStarting();
}

bool FSpatialGDKEditorToolbarModule::StartCloudSpatialDeploymentIsVisible() const
{
	return GetDefault<UGeneralProjectSettings>()->UsesSpatialNetworking() && GetDefault<USpatialGDKEditorSettings>()->SpatialOSNetFlowType == ESpatialOSNetFlow::CloudDeployment;
}

bool FSpatialGDKEditorToolbarModule::StartCloudSpatialDeploymentCanExecute() const
{
#if PLATFORM_MAC
	// Launching cloud deployments is not supported on Mac
	// TODO: UNR-3396 - allow launching cloud deployments from mac
	return false;
#else
	return CanBuildAndUpload();
#endif
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

void FSpatialGDKEditorToolbarModule::GDKRuntimeSettingsClicked() const
{
	FModuleManager::LoadModuleChecked<ISettingsModule>("Settings").ShowViewer("Project", "SpatialGDKEditor", "Runtime Settings");
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

void FSpatialGDKEditorToolbarModule::LocalDeploymentClicked()
{
	USpatialGDKEditorSettings* SpatialGDKEditorSettings = GetMutableDefault<USpatialGDKEditorSettings>();
	SpatialGDKEditorSettings->SetSpatialOSNetFlowType(ESpatialOSNetFlow::LocalDeployment);

	OnAutoStartLocalDeploymentChanged();
}

void FSpatialGDKEditorToolbarModule::CloudDeploymentClicked()
{
	USpatialGDKEditorSettings* SpatialGDKEditorSettings = GetMutableDefault<USpatialGDKEditorSettings>();
	SpatialGDKEditorSettings->SetSpatialOSNetFlowType(ESpatialOSNetFlow::CloudDeployment);

	TSharedRef<FSpatialGDKDevAuthTokenGenerator> DevAuthTokenGenerator = SpatialGDKEditorInstance->GetDevAuthTokenGeneratorRef();
	DevAuthTokenGenerator->AsyncGenerateDevAuthToken();

	OnAutoStartLocalDeploymentChanged();
}

bool FSpatialGDKEditorToolbarModule::IsLocalDeploymentIPEditable() const
{
	const USpatialGDKEditorSettings* SpatialGDKEditorSettings = GetDefault<USpatialGDKEditorSettings>();
	return GetDefault<UGeneralProjectSettings>()->UsesSpatialNetworking() && (SpatialGDKEditorSettings->SpatialOSNetFlowType == ESpatialOSNetFlow::LocalDeployment);
}

bool FSpatialGDKEditorToolbarModule::AreCloudDeploymentPropertiesEditable() const
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
			OnAutoStartLocalDeploymentChanged();
		}
	}
}

void FSpatialGDKEditorToolbarModule::ShowCloudDeploymentDialog()
{
	// Create and open the cloud configuration dialog
	if (CloudDeploymentSettingsWindowPtr.IsValid())
	{
		CloudDeploymentSettingsWindowPtr->BringToFront();
	}
	else
	{
		CloudDeploymentSettingsWindowPtr = SNew(SWindow)
			.Title(LOCTEXT("CloudDeploymentConfigurationTitle", "Cloud Deployment Configuration"))
			.HasCloseButton(true)
			.SupportsMaximize(false)
			.SupportsMinimize(false)
			.SizingRule(ESizingRule::Autosized);

		CloudDeploymentSettingsWindowPtr->SetContent(
			SNew(SBox)
			.WidthOverride(700.0f)
			[
				SAssignNew(CloudDeploymentConfigPtr, SSpatialGDKCloudDeploymentConfiguration)
				.SpatialGDKEditor(SpatialGDKEditorInstance)
				.ParentWindow(CloudDeploymentSettingsWindowPtr)
			]
		);
		CloudDeploymentSettingsWindowPtr->SetOnWindowClosed(FOnWindowClosed::CreateLambda([=](const TSharedRef<SWindow>& WindowArg)
		{
			CloudDeploymentSettingsWindowPtr = nullptr;
		}));
		FSlateApplication::Get().AddWindow(CloudDeploymentSettingsWindowPtr.ToSharedRef());
	}
}

void FSpatialGDKEditorToolbarModule::OpenLaunchConfigurationEditor()
{
	ULaunchConfigurationEditor::LaunchTransientUObjectEditor<ULaunchConfigurationEditor>(TEXT("Launch Configuration Editor"), nullptr);
}

void FSpatialGDKEditorToolbarModule::LaunchOrShowCloudDeployment()
{
	if (CanStartCloudDeployment())
	{
		OnStartCloudDeployment();
	}
	else
	{
		ShowCloudDeploymentDialog();
	}
}

void FSpatialGDKEditorToolbarModule::GenerateSchema(bool bFullScan)
{
	LocalDeploymentManager->SetRedeployRequired();

	bSchemaBuildError = false;

	if (SpatialGDKEditorInstance->FullScanRequired())
	{
		OnShowTaskStartNotification("Initial Schema Generation");

		if (SpatialGDKEditorInstance->GenerateSchema(FSpatialGDKEditor::FullAssetScan))
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

		if (SpatialGDKEditorInstance->GenerateSchema(FSpatialGDKEditor::FullAssetScan))
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

		if (SpatialGDKEditorInstance->GenerateSchema(FSpatialGDKEditor::InMemoryAsset))
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
	if (SpatialGDKEditorSettings->SpatialOSNetFlowType == ESpatialOSNetFlow::LocalDeployment)
	{
		return SpatialGDKEditorSettings->ExposedRuntimeIP;
	}
	else
	{
		return TEXT("");
	}
}

void FSpatialGDKEditorToolbarModule::OnAutoStartLocalDeploymentChanged()
{
	const USpatialGDKEditorSettings* Settings = GetDefault<USpatialGDKEditorSettings>();

	// Only auto start local deployment when the setting is checked AND local deployment connection flow is selected.
	bool bShouldAutoStartLocalDeployment = (Settings->bAutoStartLocalDeployment && Settings->SpatialOSNetFlowType == ESpatialOSNetFlow::LocalDeployment);

	// TODO: UNR-1776 Workaround for SpatialNetDriver requiring editor settings.
	LocalDeploymentManager->SetAutoDeploy(bShouldAutoStartLocalDeployment);

	if (bShouldAutoStartLocalDeployment)
	{
		if (!UEditorEngine::TryStartSpatialDeployment.IsBound())
		{
			// Bind the TryStartSpatialDeployment delegate if autostart is enabled.
			UEditorEngine::TryStartSpatialDeployment.BindLambda([this]
			{
				VerifyAndStartDeployment();
			});
		}
	}
	else
	{
		if (UEditorEngine::TryStartSpatialDeployment.IsBound())
		{
			// Unbind the TryStartSpatialDeployment if autostart is disabled.
			UEditorEngine::TryStartSpatialDeployment.Unbind();
		}
	}
}

FReply FSpatialGDKEditorToolbarModule::OnStartCloudDeployment()
{
	const USpatialGDKEditorSettings* SpatialGDKSettings = GetDefault<USpatialGDKEditorSettings>();

	if (!SpatialGDKSettings->IsDeploymentConfigurationValid())
	{
		OnShowFailedNotification(TEXT("Deployment configuration is not valid."));

		return FReply::Unhandled();
	}

	AddDeploymentTagIfMissing(SpatialConstants::DEV_LOGIN_TAG);

	CloudDeploymentConfiguration.InitFromSettings();

	if (CloudDeploymentConfiguration.bBuildAndUploadAssembly)
	{
		if (CloudDeploymentConfiguration.bGenerateSchema)
		{
			if (!SpatialGDKEditorInstance->GenerateSchema(FSpatialGDKEditor::InMemoryAsset))
			{
				OnShowSingleFailureNotification(TEXT("Generate schema failed."));
				return FReply::Unhandled();
			}
		}

		if (CloudDeploymentConfiguration.bGenerateSnapshot)
		{
			if (!SpatialGDKGenerateSnapshot(GEditor->GetEditorWorldContext().World(), CloudDeploymentConfiguration.SnapshotPath))
			{
				OnShowSingleFailureNotification(TEXT("Generate snapshot failed."));
				return FReply::Unhandled();
			}
		}

		TSharedRef<FSpatialGDKPackageAssembly> PackageAssembly = SpatialGDKEditorInstance->GetPackageAssemblyRef();
		PackageAssembly->OnSuccess.BindRaw(this, &FSpatialGDKEditorToolbarModule::OnBuildSuccess);
		PackageAssembly->BuildAndUploadAssembly(CloudDeploymentConfiguration);
	}
	else
	{
		UE_LOG(LogSpatialGDKEditorToolbar, Display, TEXT("Skipping building and uploading assembly."));
		OnBuildSuccess();
	}

	return FReply::Handled();
}

void FSpatialGDKEditorToolbarModule::OnBuildSuccess()
{
	auto StartCloudDeployment = [this]()
	{
		OnShowTaskStartNotification(FString::Printf(TEXT("Starting cloud deployment: %s"), *CloudDeploymentConfiguration.PrimaryDeploymentName));
		SpatialGDKEditorInstance->StartCloudDeployment(
			CloudDeploymentConfiguration,
			FSimpleDelegate::CreateLambda([this]()
			{
				OnShowSuccessNotification("Successfully started cloud deployment.");
				USpatialGDKEditorSettings* SpatialGDKEditorSettings = GetMutableDefault<USpatialGDKEditorSettings>();
				const FString& DeploymentName = SpatialGDKEditorSettings->GetPrimaryDeploymentName();
				SpatialGDKEditorSettings->SetDevelopmentDeploymentToConnect(DeploymentName);
				UE_LOG(LogSpatialGDKEditorToolbar, Display, TEXT("Setting deployment to connect to %s"), *DeploymentName)
			}),
			FSimpleDelegate::CreateLambda([this]()
			{
				OnShowFailedNotification("Failed to start cloud deployment. See output logs for details.");
			})
		);
	};

	AttemptSpatialAuthResult = Async(EAsyncExecution::Thread, []() { return SpatialCommandUtils::AttemptSpatialAuth(GetDefault<USpatialGDKSettings>()->IsRunningInChina()); },
		[this, StartCloudDeployment]()
	{
		if (AttemptSpatialAuthResult.IsReady() && AttemptSpatialAuthResult.Get() == true)
		{
			StartCloudDeployment();
		}
		else
		{
			OnShowFailedNotification(TEXT("Failed to launch cloud deployment. Unable to authenticate with SpatialOS."));
		}
	});
}

bool FSpatialGDKEditorToolbarModule::IsDeploymentConfigurationValid() const
{
	const USpatialGDKEditorSettings* SpatialGDKSettings = GetDefault<USpatialGDKEditorSettings>();
	return !FSpatialGDKServicesModule::GetProjectName().IsEmpty()
		&& !SpatialGDKSettings->GetPrimaryDeploymentName().IsEmpty()
		&& !SpatialGDKSettings->GetAssemblyName().IsEmpty()
		&& !SpatialGDKSettings->GetSnapshotPath().IsEmpty()
		&& !SpatialGDKSettings->GetPrimaryLaunchConfigPath().IsEmpty();
}

bool FSpatialGDKEditorToolbarModule::CanBuildAndUpload() const
{
	return SpatialGDKEditorInstance->GetPackageAssemblyRef()->CanBuild();
}

bool FSpatialGDKEditorToolbarModule::CanStartCloudDeployment() const
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

void FSpatialGDKEditorToolbarModule::AddDeploymentTagIfMissing(const FString& TagToAdd)
{
	if (TagToAdd.IsEmpty())
	{
		return;
	}

	USpatialGDKEditorSettings* SpatialGDKSettings = GetMutableDefault<USpatialGDKEditorSettings>();

	FString Tags = SpatialGDKSettings->GetDeploymentTags();
	TArray<FString> ExistingTags;
	Tags.ParseIntoArray(ExistingTags, TEXT(" "));

	if (!ExistingTags.Contains(TagToAdd))
	{
		if (ExistingTags.Num() > 0)
		{
			Tags += TEXT(" ");
		}

		Tags += TagToAdd;
		SpatialGDKSettings->SetDeploymentTags(Tags);
	}
}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FSpatialGDKEditorToolbarModule, SpatialGDKEditorToolbar)
