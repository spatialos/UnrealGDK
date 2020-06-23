// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "SpatialGDKEditorModule.h"

#include "EditorExtension/GridLBStrategyEditorExtension.h"
#include "GeneralProjectSettings.h"
#include "ISettingsContainer.h"
#include "ISettingsModule.h"
#include "ISettingsSection.h"
#include "LocalReceptionistProxyServerManager.h"
#include "Misc/MessageDialog.h"
#include "PropertyEditor/Public/PropertyEditorModule.h"
#include "SpatialCommandUtils.h"
#include "SpatialGDKEditor.h"
#include "SpatialGDKEditorCommandLineArgsManager.h"
#include "SpatialGDKEditorLayoutDetails.h"
#include "SpatialGDKEditorPackageAssembly.h"
#include "SpatialGDKEditorSchemaGenerator.h"
#include "SpatialGDKEditorSettings.h"
#include "SpatialGDKSettings.h"
#include "SpatialLaunchConfigCustomization.h"
#include "Utils/LaunchConfigurationEditor.h"
#include "SpatialRuntimeVersionCustomization.h"
#include "WorkerTypeCustomization.h"

#define LOCTEXT_NAMESPACE "FSpatialGDKEditorModule"

FSpatialGDKEditorModule::FSpatialGDKEditorModule()
	: ExtensionManager(MakeUnique<FLBStrategyEditorExtensionManager>())
	, CommandLineArgsManager(MakeUnique<FSpatialGDKEditorCommandLineArgsManager>())
{

}

void FSpatialGDKEditorModule::StartupModule()
{
	RegisterSettings();

	ExtensionManager->RegisterExtension<FGridLBStrategyEditorExtension>();
	SpatialGDKEditorInstance = MakeShareable(new FSpatialGDKEditor());
	CommandLineArgsManager->Init();

	FSpatialGDKServicesModule& GDKServices = FModuleManager::GetModuleChecked<FSpatialGDKServicesModule>("SpatialGDKServices");
	LocalReceptionistProxyServerManager = GDKServices.GetLocalReceptionistProxyServerManager();
}

void FSpatialGDKEditorModule::ShutdownModule()
{
	ExtensionManager->Cleanup();

	if (UObjectInitialized())
	{
		UnregisterSettings();
	}
}

bool FSpatialGDKEditorModule::ShouldConnectToLocalDeployment() const
{
	return GetDefault<UGeneralProjectSettings>()->UsesSpatialNetworking() && GetDefault<USpatialGDKEditorSettings>()->SpatialOSNetFlowType == ESpatialOSNetFlow::LocalDeployment;
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
	return GetDefault<UGeneralProjectSettings>()->UsesSpatialNetworking() && GetDefault<USpatialGDKEditorSettings>()->SpatialOSNetFlowType == ESpatialOSNetFlow::CloudDeployment;
}

FString FSpatialGDKEditorModule::GetDevAuthToken() const
{
	return GetDefault<USpatialGDKEditorSettings>()->DevelopmentAuthenticationToken;
}

FString FSpatialGDKEditorModule::GetSpatialOSCloudDeploymentName() const
{
	return GetDefault<USpatialGDKEditorSettings>()->GetPrimaryDeploymentName();
}

bool FSpatialGDKEditorModule::ShouldStartLocalServer() const
{
	return GetDefault<USpatialGDKEditorSettings>()->IsStartLocalServerWorkerEnabled();
}

bool FSpatialGDKEditorModule::TryStartLocalReceptionistProxyServer() const
{
	if(ShouldConnectToCloudDeployment() && ShouldStartLocalServer())
	{
			bool bSuccess = LocalReceptionistProxyServerManager->TryStartReceptionistProxyServer(GetDefault<USpatialGDKSettings>()->IsRunningInChina(), GetDefault<USpatialGDKEditorSettings>()->DevelopmentDeploymentToConnect, GetDefault<USpatialGDKEditorSettings>()->ListeningAddress, GetDefault<USpatialGDKEditorSettings>()->LocalReceptionistPort);

			if (bSuccess)
			{
				UE_LOG(LogTemp, Log, TEXT("%s"), *LOCTEXT("ProxyStartedSuccessfully", "Successfully started local receptionist proxy server!").ToString());
			}
			else
			{
				UE_LOG(LogTemp, Error, TEXT("%s"), *LOCTEXT("FailedToStartProxy", "Failed to start local receptionist proxy server").ToString());
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
		OutErrorMessage = LOCTEXT("MissingSchema", "Attempted to start a local deployment but schema is not generated. You can generate it by clicking on the Schema button in the toolbar.");
		return false;
	}

	if (ShouldConnectToCloudDeployment())
	{
		if (GetDevAuthToken().IsEmpty())
		{
			OutErrorMessage = LOCTEXT("MissingDevelopmentAuthenticationToken", "You have to generate or provide a development authentication token in the SpatialOS GDK Editor Settings section to enable connecting to a cloud deployment.");
			return false;
		}

		const USpatialGDKEditorSettings* Settings = GetDefault<USpatialGDKEditorSettings>();
		bool bIsRunningInChina = GetDefault<USpatialGDKSettings>()->IsRunningInChina();
		if (!Settings->GetPrimaryDeploymentName().IsEmpty() && !SpatialCommandUtils::HasDevLoginTag(Settings->GetPrimaryDeploymentName(), bIsRunningInChina, OutErrorMessage))
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
		OutErrorMessage = LOCTEXT("MissingLocalDeploymentIP", "You have to enter this machine's local network IP in the 'Local Deployment IP' field to enable connecting to a local deployment.");
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
		CommandLine = TEXT("connect.to.spatialos -devAuthToken ") + GetDevAuthToken();
		FString CloudDeploymentName = GetSpatialOSCloudDeploymentName();
		if (!CloudDeploymentName.IsEmpty())
		{
			CommandLine += TEXT(" -deployment ") + CloudDeploymentName;
		}
		else
		{
			UE_LOG(LogTemp, Display, TEXT("Cloud deployment name is empty. If there are multiple running deployments with 'dev_login' tag, the game will choose one randomly."));
		}
	}
	return CommandLine;
}

bool FSpatialGDKEditorModule::ShouldPackageMobileCommandLineArgs() const
{
	return GetDefault<USpatialGDKEditorSettings>()->bPackageMobileCommandLineArgs;
}

void FSpatialGDKEditorModule::RegisterSettings()
{
	if (ISettingsModule* SettingsModule = FModuleManager::GetModulePtr<ISettingsModule>("Settings"))
	{
		ISettingsContainerPtr SettingsContainer = SettingsModule->GetContainer("Project");

		SettingsContainer->DescribeCategory("SpatialGDKEditor", LOCTEXT("RuntimeWDCategoryName", "SpatialOS GDK for Unreal"),
			LOCTEXT("RuntimeWDCategoryDescription", "Configuration for the SpatialOS GDK for Unreal"));

		ISettingsSectionPtr EditorSettingsSection = SettingsModule->RegisterSettings("Project", "SpatialGDKEditor", "Editor Settings",
			LOCTEXT("SpatialEditorGeneralSettingsName", "Editor Settings"),
			LOCTEXT("SpatialEditorGeneralSettingsDescription", "Editor configuration for the SpatialOS GDK for Unreal"),
			GetMutableDefault<USpatialGDKEditorSettings>());

		if (EditorSettingsSection.IsValid())
		{
			EditorSettingsSection->OnModified().BindRaw(this, &FSpatialGDKEditorModule::HandleEditorSettingsSaved);
		}

		ISettingsSectionPtr RuntimeSettingsSection = SettingsModule->RegisterSettings("Project", "SpatialGDKEditor", "Runtime Settings",
			LOCTEXT("SpatialRuntimeGeneralSettingsName", "Runtime Settings"),
			LOCTEXT("SpatialRuntimeGeneralSettingsDescription", "Runtime configuration for the SpatialOS GDK for Unreal"),
			GetMutableDefault<USpatialGDKSettings>());

		if (RuntimeSettingsSection.IsValid())
		{
			RuntimeSettingsSection->OnModified().BindRaw(this, &FSpatialGDKEditorModule::HandleRuntimeSettingsSaved);
		}
	}

	FPropertyEditorModule& PropertyModule = FModuleManager::LoadModuleChecked<FPropertyEditorModule>("PropertyEditor");
	PropertyModule.RegisterCustomPropertyTypeLayout("WorkerType", FOnGetPropertyTypeCustomizationInstance::CreateStatic(&FWorkerTypeCustomization::MakeInstance));
	PropertyModule.RegisterCustomPropertyTypeLayout("SpatialLaunchConfigDescription", FOnGetPropertyTypeCustomizationInstance::CreateStatic(&FSpatialLaunchConfigCustomization::MakeInstance));
	PropertyModule.RegisterCustomPropertyTypeLayout("RuntimeVariantVersion", FOnGetPropertyTypeCustomizationInstance::CreateStatic(&FSpatialRuntimeVersionCustomization::MakeInstance));
	PropertyModule.RegisterCustomClassLayout(USpatialGDKEditorSettings::StaticClass()->GetFName(), FOnGetDetailCustomizationInstance::CreateStatic(&FSpatialGDKEditorLayoutDetails::MakeInstance));
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
