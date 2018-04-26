// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "SpatialGDKModule.h"

#include "ModuleManager.h"
#include "Paths.h"
#include "PlatformProcess.h"
#include "SpatialGDKSettings.h"
#include "UObjectBase.h"

#define LOCTEXT_NAMESPACE "FSpatialGDKModule"

DEFINE_LOG_CATEGORY(LogSpatialGDKModule);

IMPLEMENT_MODULE(FSpatialGDKModule, SpatialGDK)

void FSpatialGDKModule::StartupModule()
{
	RegisterSettings();
}

void FSpatialGDKModule::ShutdownModule()
{
	if (UObjectInitialized())
	{
		UnregisterSettings();
	}
}

void FSpatialGDKModule::RegisterSettings()
{
	if (ISettingsModule* SettingsModule = FModuleManager::GetModulePtr<ISettingsModule>("Settings"))
	{
		ISettingsContainerPtr SettingsContainer = SettingsModule->GetContainer("Project");

		SettingsContainer->DescribeCategory(
		"SpatialGDK", LOCTEXT("RuntimeWDCategoryName", "SpatialGDK"), LOCTEXT("RuntimeWDCategoryDescription", "Configuration for the SpatialGDK module"));

		ISettingsSectionPtr SettingsSection = SettingsModule->RegisterSettings(
		"Project", "SpatialGDK", "SpatialGDK", LOCTEXT("RuntimeGeneralSettingsName", "SpatialGDK"), LOCTEXT("RuntimeGeneralSettingsDescription", "Base configuration for SpatialGDK module."), GetMutableDefault<USpatialGDKSettings>());

		if (SettingsSection.IsValid())
		{
			SettingsSection->OnModified().BindRaw(this, &FSpatialGDKModule::HandleSettingsSaved);
		}
	}
}

void FSpatialGDKModule::UnregisterSettings()
{
	if (ISettingsModule* SettingsModule = FModuleManager::GetModulePtr<ISettingsModule>("Settings"))
	{
		SettingsModule->UnregisterSettings("Project", "SpatialGDK", "SpatialGDK");
	}
}

bool FSpatialGDKModule::HandleSettingsSaved()
{
	USpatialGDKSettings* Settings = GetMutableDefault<USpatialGDKSettings>();
	Settings->SaveConfig();

	return true;
}

#undef LOCTEXT_NAMESPACE
