// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "SpatialGDKEditorModule.h"

#include "SpatialGDKEditorSettings.h"

#include "ISettingsModule.h"
#include "ISettingsContainer.h"
#include "ISettingsSection.h"

#define LOCTEXT_NAMESPACE "FSpatialGDKEditorModule"

void FSpatialGDKEditorModule::StartupModule()
{
	RegisterSettings();
}

void FSpatialGDKEditorModule::ShutdownModule()
{
	if (UObjectInitialized())
	{
		UnregisterSettings();
	}
}

void FSpatialGDKEditorModule::RegisterSettings()
{
	if (ISettingsModule* SettingsModule = FModuleManager::GetModulePtr<ISettingsModule>("Settings"))
	{
		ISettingsContainerPtr SettingsContainer = SettingsModule->GetContainer("Project");

		SettingsContainer->DescribeCategory("SpatialGDKEditor", LOCTEXT("RuntimeWDCategoryName", "SpatialOS GDK for Unreal"),
			LOCTEXT("RuntimeWDCategoryDescription", "Configuration for the SpatialOS GDK for Unreal"));

		ISettingsSectionPtr SettingsSection = SettingsModule->RegisterSettings("Project", "SpatialGDKEditor", "Settings",
			LOCTEXT("RuntimeGeneralSettingsName", "Settings"),
			LOCTEXT("RuntimeGeneralSettingsDescription", "Configuration for the SpatialOS GDK for Unreal"),
			GetMutableDefault<USpatialGDKEditorSettings>());

		if (SettingsSection.IsValid())
		{
			SettingsSection->OnModified().BindRaw(this, &FSpatialGDKEditorModule::HandleSettingsSaved);
		}
	}
}

void FSpatialGDKEditorModule::UnregisterSettings()
{
	if (ISettingsModule* SettingsModule = FModuleManager::GetModulePtr<ISettingsModule>("Settings"))
	{
		SettingsModule->UnregisterSettings("Project", "SpatialGDKEditor", "Settings");
	}
}

bool FSpatialGDKEditorModule::HandleSettingsSaved()
{
	GetMutableDefault<USpatialGDKEditorSettings>()->SaveConfig();

	return true;
}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FSpatialGDKEditorModule, SpatialGDKEditor);
