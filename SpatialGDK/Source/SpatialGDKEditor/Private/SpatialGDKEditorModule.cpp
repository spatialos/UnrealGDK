// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "SpatialGDKEditorModule.h"

#include "Editor.h"
#include "GeneralProjectSettings.h"
#include "ISettingsContainer.h"
#include "ISettingsModule.h"
#include "ISettingsSection.h"
#include "LocalReceptionistProxyServerManager.h"
#include "Misc/MessageDialog.h"
#include "PropertyEditor/Public/PropertyEditorModule.h"

#include "SpatialCommandUtils.h"
#include "SpatialGDKDefaultLaunchConfigGenerator.h"
#include "SpatialGDKEditor.h"
#include "SpatialGDKEditorCommandLineArgsManager.h"
#include "SpatialGDKEditorLayoutDetails.h"
#include "SpatialGDKEditorPackageAssembly.h"
#include "SpatialGDKEditorSchemaGenerator.h"
#include "SpatialGDKEditorSettings.h"
#include "SpatialGDKFunctionalTests/Public/SpatialFunctionalTest.h"
#include "SpatialGDKSettings.h"
#include "SpatialLaunchConfigCustomization.h"
#include "SpatialRuntimeVersionCustomization.h"

#include "Engine/World.h"
#include "EngineClasses/SpatialWorldSettings.h"
#include "EngineUtils.h"
#include "IAutomationControllerModule.h"
#include "SpatialFunctionalTest.h"
#include "Utils/LaunchConfigurationEditor.h"
#include "WorkerTypeCustomization.h"

DEFINE_LOG_CATEGORY(LogSpatialGDKEditorModule);

#define LOCTEXT_NAMESPACE "FSpatialGDKEditorModule"

FSpatialGDKEditorModule::FSpatialGDKEditorModule()
	: CommandLineArgsManager(MakeUnique<FSpatialGDKEditorCommandLineArgsManager>())
{
}

void FSpatialGDKEditorModule::StartupModule()
{
	RegisterSettings();

	SpatialGDKEditorInstance = MakeShareable(new FSpatialGDKEditor());
	CommandLineArgsManager->Init();

	// This is relying on the module loading phase - SpatialGDKServices module should be already loaded
	FSpatialGDKServicesModule& GDKServices = FModuleManager::GetModuleChecked<FSpatialGDKServicesModule>("SpatialGDKServices");
	LocalReceptionistProxyServerManager = GDKServices.GetLocalReceptionistProxyServerManager();

	// Allow Spatial Plugin to stop PIE after Automation Manager completes the tests
	IAutomationControllerModule& AutomationControllerModule =
		FModuleManager::LoadModuleChecked<IAutomationControllerModule>(TEXT("AutomationController"));
	IAutomationControllerManagerPtr AutomationController = AutomationControllerModule.GetAutomationController();
	AutomationController->OnTestsComplete().AddLambda([]() {
		// Make sure to clear the snapshot in case something happened with Tests (or they weren't ran properly).
		ASpatialFunctionalTest::ClearAllTakenSnapshots();

#if ENGINE_MINOR_VERSION < 25
		if (GetDefault<USpatialGDKEditorSettings>()->bStopPIEOnTestingCompleted && GEditor->EditorWorld != nullptr)
#else
		if (GetDefault<USpatialGDKEditorSettings>()->bStopPIEOnTestingCompleted && GEditor->IsPlayingSessionInEditor())
#endif
		{
			GEditor->EndPlayMap();
		}
	});
}

void FSpatialGDKEditorModule::ShutdownModule()
{
	if (UObjectInitialized())
	{
		UnregisterSettings();
	}
}

void FSpatialGDKEditorModule::TakeSnapshot(UWorld* World, FSpatialSnapshotTakenFunc OnSnapshotTaken)
{
	FSpatialGDKServicesModule& GDKServices = FModuleManager::GetModuleChecked<FSpatialGDKServicesModule>("SpatialGDKServices");
	GDKServices.GetLocalDeploymentManager()->TakeSnapshot(World, OnSnapshotTaken);
}

bool FSpatialGDKEditorModule::ShouldConnectToLocalDeployment() const
{
	return GetDefault<UGeneralProjectSettings>()->UsesSpatialNetworking()
		   && GetDefault<USpatialGDKEditorSettings>()->SpatialOSNetFlowType == ESpatialOSNetFlow::LocalDeployment;
}

FString FSpatialGDKEditorModule::GetSpatialOSLocalDeploymentIP() const
{
	return GetDefault<USpatialGDKEditorSettings>()->ExposedRuntimeIP;
}

bool FSpatialGDKEditorModule::ShouldStartPIEClientsWithLocalLaunchOnDevice() const
{
	return GetDefault<USpatialGDKEditorSettings>()->bStartPIEClientsWithLocalLaunchOnDevice;
}

bool FSpatialGDKEditorModule::ShouldConnectToCloudDeployment() const
{
	return GetDefault<UGeneralProjectSettings>()->UsesSpatialNetworking()
		   && GetDefault<USpatialGDKEditorSettings>()->SpatialOSNetFlowType == ESpatialOSNetFlow::CloudDeployment;
}

FString FSpatialGDKEditorModule::GetDevAuthToken() const
{
	return GetDefault<USpatialGDKEditorSettings>()->DevelopmentAuthenticationToken;
}

FString FSpatialGDKEditorModule::GetSpatialOSCloudDeploymentName() const
{
	return GetDefault<USpatialGDKEditorSettings>()->GetPrimaryDeploymentName();
}

bool FSpatialGDKEditorModule::ShouldConnectServerToCloud() const
{
	return GetDefault<USpatialGDKEditorSettings>()->IsConnectServerToCloudEnabled();
}

bool FSpatialGDKEditorModule::TryStartLocalReceptionistProxyServer() const
{
	if (ShouldConnectToCloudDeployment() && ShouldConnectServerToCloud())
	{
		const USpatialGDKEditorSettings* EditorSettings = GetDefault<USpatialGDKEditorSettings>();
		bool bSuccess = LocalReceptionistProxyServerManager->TryStartReceptionistProxyServer(
			GetDefault<USpatialGDKSettings>()->IsRunningInChina(), EditorSettings->GetPrimaryDeploymentName(),
			EditorSettings->ListeningAddress, EditorSettings->LocalReceptionistPort);

		if (bSuccess)
		{
			UE_LOG(LogSpatialGDKEditorModule, Log, TEXT("Successfully started local receptionist proxy server!"));
		}
		else
		{
			FMessageDialog::Open(
				EAppMsgType::Ok,
				LOCTEXT("ReceptionistProxyFailure", "Failed to start local receptionist proxy server. See the logs for more information."));
		}

		return bSuccess;
	}

	return true;
}

bool FSpatialGDKEditorModule::CanExecuteLaunch() const
{
	return SpatialGDKEditorInstance->GetPackageAssemblyRef()->CanBuild();
}

bool FSpatialGDKEditorModule::CanStartSession(FText& OutErrorMessage) const
{
	if (!SpatialGDKEditorInstance->IsSchemaGenerated())
	{
		OutErrorMessage = LOCTEXT("MissingSchema",
								  "Attempted to start a local deployment but schema is not generated. You can generate it by clicking on "
								  "the Schema button in the toolbar.");
		return false;
	}

	if (ShouldConnectToCloudDeployment())
	{
		if (GetDevAuthToken().IsEmpty())
		{
			OutErrorMessage = LOCTEXT("MissingDevelopmentAuthenticationToken",
									  "You have to generate or provide a development authentication token in the SpatialOS GDK Editor "
									  "Settings section to enable connecting to a cloud deployment.");
			return false;
		}

		const USpatialGDKEditorSettings* Settings = GetDefault<USpatialGDKEditorSettings>();
		bool bIsRunningInChina = GetDefault<USpatialGDKSettings>()->IsRunningInChina();
		if (!Settings->GetPrimaryDeploymentName().IsEmpty()
			&& !SpatialCommandUtils::HasDevLoginTag(Settings->GetPrimaryDeploymentName(), bIsRunningInChina, OutErrorMessage))
		{
			return false;
		}
	}

	return true;
}

bool FSpatialGDKEditorModule::CanStartPlaySession(FText& OutErrorMessage) const
{
	if (!GetDefault<UGeneralProjectSettings>()->UsesSpatialNetworking())
	{
		return true;
	}

	return CanStartSession(OutErrorMessage);
}

bool FSpatialGDKEditorModule::CanStartLaunchSession(FText& OutErrorMessage) const
{
	if (!GetDefault<UGeneralProjectSettings>()->UsesSpatialNetworking())
	{
		return true;
	}

	if (ShouldConnectToLocalDeployment() && GetSpatialOSLocalDeploymentIP().IsEmpty())
	{
		OutErrorMessage = LOCTEXT("MissingLocalDeploymentIP",
								  "You have to enter this machine's local network IP in the 'Local Deployment IP' field to enable "
								  "connecting to a local deployment.");
		return false;
	}

	return CanStartSession(OutErrorMessage);
}

FString FSpatialGDKEditorModule::GetMobileClientCommandLineArgs() const
{
	FString CommandLine;
	if (ShouldConnectToLocalDeployment())
	{
		CommandLine = FString::Printf(TEXT("%s -useExternalIpForBridge true"), *GetSpatialOSLocalDeploymentIP());
	}
	else if (ShouldConnectToCloudDeployment())
	{
		// 127.0.0.1 is only used to indicate that we want to connect to a deployment.
		// This address won't be used when actually trying to connect, but Unreal will try to resolve the address and close the connection
		// if it fails.
		CommandLine = TEXT("127.0.0.1 -devAuthToken ") + GetDevAuthToken();
		FString CloudDeploymentName = GetSpatialOSCloudDeploymentName();
		if (!CloudDeploymentName.IsEmpty())
		{
			CommandLine += TEXT(" -deployment ") + CloudDeploymentName;
		}
		else
		{
			UE_LOG(LogSpatialGDKEditorModule, Display,
				   TEXT("Cloud deployment name is empty. If there are multiple running deployments with 'dev_login' tag, the game will "
						"choose one randomly."));
		}
	}
	return CommandLine;
}

bool FSpatialGDKEditorModule::ShouldPackageMobileCommandLineArgs() const
{
	return GetDefault<USpatialGDKEditorSettings>()->bPackageMobileCommandLineArgs;
}

uint32 GetPIEServerWorkers()
{
	const USpatialGDKEditorSettings* EditorSettings = GetDefault<USpatialGDKEditorSettings>();
	if (EditorSettings->bGenerateDefaultLaunchConfig && !EditorSettings->LaunchConfigDesc.ServerWorkerConfiguration.bAutoNumEditorInstances)
	{
		return EditorSettings->LaunchConfigDesc.ServerWorkerConfiguration.NumEditorInstances;
	}
	else
	{
		UWorld* EditorWorld = GEditor->GetEditorWorldContext().World();
		check(EditorWorld);
		return GetWorkerCountFromWorldSettings(*EditorWorld);
	}
}

bool FSpatialGDKEditorModule::ForEveryServerWorker(TFunction<void(const FName&, int32)> Function) const
{
	if (ShouldStartLocalServer())
	{
		int32 AdditionalServerIndex = 0;
		for (uint32 i = 0; i < GetPIEServerWorkers(); ++i)
		{
			Function(SpatialConstants::DefaultServerWorkerType, AdditionalServerIndex);
			AdditionalServerIndex++;
		}

		return true;
	}

	return false;
}

FPlayInEditorSettingsOverride FSpatialGDKEditorModule::GetPlayInEditorSettingsOverrideForTesting(UWorld* World) const
{
	// By default, clear that the runtime/test was loaded from a snapshot taken for a given world.
	ASpatialFunctionalTest::ClearLoadedFromTakenSnapshot();

	FPlayInEditorSettingsOverride PIESettingsOverride = ISpatialGDKEditorModule::GetPlayInEditorSettingsOverrideForTesting(World);
	if (const ASpatialWorldSettings* SpatialWorldSettings = Cast<ASpatialWorldSettings>(World->GetWorldSettings()))
	{
		EMapTestingMode TestingMode = SpatialWorldSettings->TestingSettings.TestingMode;
		if (TestingMode != EMapTestingMode::UseCurrentSettings)
		{
			TActorIterator<ASpatialFunctionalTest> SpatialTestIt(World);
			if (TestingMode == EMapTestingMode::Detect)
			{
				if (SpatialTestIt)
				{
					TestingMode = EMapTestingMode::ForceSpatial;
				}
				else
				{
					TActorIterator<AFunctionalTest> NativeTestIt(World);
					if (!NativeTestIt)
					{
						// if there's no AFunctionalTests assume it's a Unit Test, so use current settings
						return PIESettingsOverride;
					}
					TestingMode = EMapTestingMode::ForceNativeOffline;
				}
			}

			int NumberOfClients = 1;

			PIESettingsOverride.bUseSpatial = false; // turn off by default

			switch (TestingMode)
			{
			case EMapTestingMode::ForceNativeOffline:
				PIESettingsOverride.PlayNetMode = EPlayNetMode::PIE_Standalone;
				break;
			case EMapTestingMode::ForceNativeAsListenServer:
				PIESettingsOverride.PlayNetMode = EPlayNetMode::PIE_ListenServer;
				break;
			case EMapTestingMode::ForceNativeAsClient:
				PIESettingsOverride.PlayNetMode = EPlayNetMode::PIE_Client;
				break;
			case EMapTestingMode::ForceSpatial:
				PIESettingsOverride.bUseSpatial = true; // turn on for Spatial
				PIESettingsOverride.PlayNetMode = EPlayNetMode::PIE_Client;
				for (; SpatialTestIt; ++SpatialTestIt)
				{
					NumberOfClients = FMath::Max(SpatialTestIt->GetNumRequiredClients(), NumberOfClients);
				}
				{
					FString SnapshotForMap = ASpatialFunctionalTest::GetTakenSnapshotPath(World);

					if (!SnapshotForMap.IsEmpty())
					{
						PIESettingsOverride.ForceUseSnapshot = SnapshotForMap;
						// Set that we're loading from taken snapshot.
						ASpatialFunctionalTest::SetLoadedFromTakenSnapshot();
					}
				}
				break;
			default:
				checkf(false, TEXT("Unsupported Testing Mode"));
				break;
			}

			PIESettingsOverride.NumberOfClients = NumberOfClients;
		}
	}
	return PIESettingsOverride;
}

bool FSpatialGDKEditorModule::ShouldStartLocalServer() const
{
	if (!GetDefault<UGeneralProjectSettings>()->UsesSpatialNetworking())
	{
		// Always start the PIE server(s) if Spatial networking is disabled.
		return true;
	}

	if (ShouldConnectToLocalDeployment())
	{
		// Start the PIE server(s) if we're connecting to a local deployment.
		return true;
	}
	if (ShouldConnectToCloudDeployment() && ShouldConnectServerToCloud())
	{
		// Start the PIE server(s) if we're connecting to a cloud deployment and using receptionist proxy for the server(s).
		return true;
	}
	return false;
}

void FSpatialGDKEditorModule::RegisterSettings()
{
	if (ISettingsModule* SettingsModule = FModuleManager::GetModulePtr<ISettingsModule>("Settings"))
	{
		ISettingsContainerPtr SettingsContainer = SettingsModule->GetContainer("Project");

		SettingsContainer->DescribeCategory("SpatialGDKEditor", LOCTEXT("RuntimeWDCategoryName", "SpatialOS GDK for Unreal"),
											LOCTEXT("RuntimeWDCategoryDescription", "Configuration for the SpatialOS GDK for Unreal"));

		ISettingsSectionPtr EditorSettingsSection = SettingsModule->RegisterSettings(
			"Project", "SpatialGDKEditor", "Editor Settings", LOCTEXT("SpatialEditorGeneralSettingsName", "Editor Settings"),
			LOCTEXT("SpatialEditorGeneralSettingsDescription", "Editor configuration for the SpatialOS GDK for Unreal"),
			GetMutableDefault<USpatialGDKEditorSettings>());

		if (EditorSettingsSection.IsValid())
		{
			EditorSettingsSection->OnModified().BindRaw(this, &FSpatialGDKEditorModule::HandleEditorSettingsSaved);
		}

		ISettingsSectionPtr RuntimeSettingsSection = SettingsModule->RegisterSettings(
			"Project", "SpatialGDKEditor", "Runtime Settings", LOCTEXT("SpatialRuntimeGeneralSettingsName", "Runtime Settings"),
			LOCTEXT("SpatialRuntimeGeneralSettingsDescription", "Runtime configuration for the SpatialOS GDK for Unreal"),
			GetMutableDefault<USpatialGDKSettings>());

		if (RuntimeSettingsSection.IsValid())
		{
			RuntimeSettingsSection->OnModified().BindRaw(this, &FSpatialGDKEditorModule::HandleRuntimeSettingsSaved);
		}
	}

	FPropertyEditorModule& PropertyModule = FModuleManager::LoadModuleChecked<FPropertyEditorModule>("PropertyEditor");
	PropertyModule.RegisterCustomPropertyTypeLayout(
		"WorkerType", FOnGetPropertyTypeCustomizationInstance::CreateStatic(&FWorkerTypeCustomization::MakeInstance));
	PropertyModule.RegisterCustomPropertyTypeLayout(
		"SpatialLaunchConfigDescription",
		FOnGetPropertyTypeCustomizationInstance::CreateStatic(&FSpatialLaunchConfigCustomization::MakeInstance));
	PropertyModule.RegisterCustomPropertyTypeLayout(
		"RuntimeVariantVersion", FOnGetPropertyTypeCustomizationInstance::CreateStatic(&FSpatialRuntimeVersionCustomization::MakeInstance));
	PropertyModule.RegisterCustomClassLayout(
		USpatialGDKEditorSettings::StaticClass()->GetFName(),
		FOnGetDetailCustomizationInstance::CreateStatic(&FSpatialGDKEditorLayoutDetails::MakeInstance));
}

void FSpatialGDKEditorModule::UnregisterSettings()
{
	if (ISettingsModule* SettingsModule = FModuleManager::GetModulePtr<ISettingsModule>("Settings"))
	{
		SettingsModule->UnregisterSettings("Project", "SpatialGDKEditor", "Editor Settings");
		SettingsModule->UnregisterSettings("Project", "SpatialGDKEditor", "Runtime Settings");
	}
	FPropertyEditorModule& PropertyModule = FModuleManager::LoadModuleChecked<FPropertyEditorModule>("PropertyEditor");
	PropertyModule.UnregisterCustomPropertyTypeLayout("FWorkerAssociation");
}

bool FSpatialGDKEditorModule::HandleEditorSettingsSaved()
{
	GetMutableDefault<USpatialGDKEditorSettings>()->SaveConfig();

	return true;
}

bool FSpatialGDKEditorModule::HandleRuntimeSettingsSaved()
{
	GetMutableDefault<USpatialGDKSettings>()->SaveConfig();

	return true;
}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FSpatialGDKEditorModule, SpatialGDKEditor);
