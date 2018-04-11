// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "SpatialGDKModule.h"

#include "ModuleManager.h"
#include "Paths.h"
#include "PlatformProcess.h"
#include "SpatialOSSettings.h"
#include "UObjectBase.h"

#define LOCTEXT_NAMESPACE "FSpatialGDKModule"

DEFINE_LOG_CATEGORY(LogSpatialGDKModule);

IMPLEMENT_MODULE(FSpatialGDKModule, SpatialGDK)

void FSpatialGDKModule::StartupModule()
{
 // RegisterSettings();
}

void FSpatialGDKModule::ShutdownModule()
{
  if (UObjectInitialized())
  {
    //UnregisterSettings();
  }
}

// void FSpatialGDKModule::RegisterSettings()
// {
//   if (ISettingsModule* SettingsModule = FModuleManager::GetModulePtr<ISettingsModule>("Settings"))
//   {
//     ISettingsContainerPtr SettingsContainer = SettingsModule->GetContainer("Project");

//     SettingsContainer->DescribeCategory(
//         "SpatialOS", LOCTEXT("RuntimeWDCategoryName", "SpatialOS"),
//         LOCTEXT("RuntimeWDCategoryDescription", "Configuration for the SpatialOS module"));

//     ISettingsSectionPtr SettingsSection = SettingsModule->RegisterSettings(
//         "Project", "SpatialOS", "SpatialOS", LOCTEXT("RuntimeGeneralSettingsName", "SpatialOS"),
//         LOCTEXT("RuntimeGeneralSettingsDescription", "Base configuration for SpatialOS module."),
//         GetMutableDefault<USpatialOSSettings>());

//     if (SettingsSection.IsValid())
//     {
//       SettingsSection->OnModified().BindRaw(this, &FSpatialGDKModule::HandleSettingsSaved);
//     }
//   }
// }

// void FSpatialGDKModule::UnregisterSettings()
// {
//   if (ISettingsModule* SettingsModule = FModuleManager::GetModulePtr<ISettingsModule>("Settings"))
//   {
//     SettingsModule->UnregisterSettings("Project", "SpatialOS", "SpatialOS");
//   }
// }

// bool FSpatialGDKModule::HandleSettingsSaved()
// {
//   USpatialOSSettings* Settings = GetMutableDefault<USpatialOSSettings>();
//   Settings->SaveConfig();

//   return true;
// }

#undef LOCTEXT_NAMESPACE
