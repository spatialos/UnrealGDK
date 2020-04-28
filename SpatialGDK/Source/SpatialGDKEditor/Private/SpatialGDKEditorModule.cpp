// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "SpatialGDKEditorModule.h"

#include "GeneralProjectSettings.h"
#include "SpatialGDKSettings.h"
#include "SpatialGDKEditorSettings.h"
#include "SpatialGDKEditorLayoutDetails.h"

#include "ISettingsModule.h"
#include "ISettingsContainer.h"
#include "ISettingsSection.h"
#include "PropertyEditor/Public/PropertyEditorModule.h"
#include "WorkerTypeCustomization.h"

#include "EditorExtension/GridLBStrategyEditorExtension.h"

#define LOCTEXT_NAMESPACE "FSpatialGDKEditorModule"

FSpatialGDKEditorModule::FSpatialGDKEditorModule()
	: ExtensionManager(MakeUnique<FLBStrategyEditorExtensionManager>())
{

}

void FSpatialGDKEditorModule::StartupModule()
{
	RegisterSettings();

	ExtensionManager->RegisterExtension<FGridLBStrategyEditorExtension>();
}

void FSpatialGDKEditorModule::ShutdownModule()
{
	ExtensionManager->Cleanup();

	if (UObjectInitialized())
	{
		UnregisterSettings();
	}
}

FString FSpatialGDKEditorModule::GetSpatialOSCloudDeploymentName() const
{
	return GetDefault<USpatialGDKEditorSettings>()->DevelopmentDeploymentToConnect;
}

FString FSpatialGDKEditorModule::GetSpatialOSLocalDeploymentIP() const
{
	const USpatialGDKEditorSettings* SpatialGDKEditorSettings = GetDefault<USpatialGDKEditorSettings>();
	return SpatialGDKEditorSettings->ExposedRuntimeIP;
}

int FSpatialGDKEditorModule::GetSpatialOSNetFlowType() const
{
	const USpatialGDKEditorSettings* SpatialGDKEditorSettings = GetDefault<USpatialGDKEditorSettings>();
	return SpatialGDKEditorSettings->SpatialOSNetFlowType;
}

bool FSpatialGDKEditorModule::ShouldConnectToLocalDeployment() const
{
	return GetDefault<UGeneralProjectSettings>()->UsesSpatialNetworking() && GetDefault<USpatialGDKEditorSettings>()->SpatialOSNetFlowType == ESpatialOSNetFlow::LocalDeployment;
}

bool FSpatialGDKEditorModule::ShouldConnectToCloudDeployment() const
{
	return GetDefault<UGeneralProjectSettings>()->UsesSpatialNetworking() && GetDefault<USpatialGDKEditorSettings>()->SpatialOSNetFlowType == ESpatialOSNetFlow::CloudDeployment;
}

FString FSpatialGDKEditorModule::GetDevAuthToken() const
{
	return GetDefault<USpatialGDKEditorSettings>()->DevelopmentAuthenticationToken;
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
