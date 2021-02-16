// Fill out your copyright notice in the Description page of Project Settings.
#include "SpatialTestSettings.h"

FSpatialTestSettings::FSpatialTestSettings()
	: OriginalSpatialGDKSettings(nullptr)
	, OriginalLevelEditorPlaySettings(nullptr)
	, OriginalSpatialGDKEditorSettings(nullptr)
	, OriginalGeneralProjectSettings(nullptr)
	, OriginalEditorPerformanceSettings(nullptr)
{
}

void FSpatialTestSettings::Override(const FString& MapName)
{
	// Back up the existing settings so they can be reverted later
	Backup();
	// First override the settings from the base config file, if it exists
	Load(BaseOverridesFilename);
	// Then override the settings from the map specific config file, if it exists
	FString MapOverridesFilename = OverrideSettingsBaseFilename + FPackageName::GetShortName(MapName) + (OverrideSettingsFileExtension);
	Load(MapOverridesFilename);
}

void FSpatialTestSettings::Backup()
{
	// Duplicate original settings but use different outer - if same outer is reused the object is not duplicated and pointer is to the same
	// Object Having the same name causes runtime exceptions
	// Use additional object flag RF_Standalone to avoid early GC - destroy when restoring settings
	OriginalSpatialGDKSettings = NewObject<USpatialGDKSettings>(
		GetTransientPackage(), USpatialGDKSettings::StaticClass(), NAME_None,
		EObjectFlags(RF_Public | RF_ClassDefaultObject | RF_ArchetypeObject | RF_Standalone), GetMutableDefault<USpatialGDKSettings>());

	OriginalLevelEditorPlaySettings =
		NewObject<ULevelEditorPlaySettings>(GetTransientPackage(), ULevelEditorPlaySettings::StaticClass(), NAME_None,
											EObjectFlags(RF_Public | RF_ClassDefaultObject | RF_ArchetypeObject | RF_Standalone),
											GetMutableDefault<ULevelEditorPlaySettings>());

	OriginalSpatialGDKEditorSettings =
		NewObject<USpatialGDKEditorSettings>(GetTransientPackage(), USpatialGDKEditorSettings::StaticClass(), NAME_None,
											 EObjectFlags(RF_Public | RF_ClassDefaultObject | RF_ArchetypeObject | RF_Standalone),
											 GetMutableDefault<USpatialGDKEditorSettings>());

	OriginalGeneralProjectSettings = NewObject<UGeneralProjectSettings>(
		GetTransientPackage(), UGeneralProjectSettings::StaticClass(), NAME_None,
		EObjectFlags(RF_Public | RF_ClassDefaultObject | RF_ArchetypeObject | RF_Standalone), GetMutableDefault<UGeneralProjectSettings>());

	OriginalEditorPerformanceSettings =
		NewObject<UEditorPerformanceSettings>(GetTransientPackage(), UEditorPerformanceSettings::StaticClass(), NAME_None,
											  EObjectFlags(RF_Public | RF_ClassDefaultObject | RF_ArchetypeObject | RF_Standalone),
											  GetMutableDefault<UEditorPerformanceSettings>());
}

void FSpatialTestSettings::Restore()
{
	if (OriginalSpatialGDKSettings != nullptr)
	{
		// Restore original settings - delete CDO first and then replace with copied settings
		USpatialGDKSettings::StaticClass()->ClassDefaultObject->ConditionalBeginDestroy();
		USpatialGDKSettings::StaticClass()->ClassDefaultObject = OriginalSpatialGDKSettings;
		OriginalSpatialGDKSettings = nullptr;
	}

	if (OriginalLevelEditorPlaySettings != nullptr)
	{
		// Restore original settings - delete CDO first and then replace with copied settings
		ULevelEditorPlaySettings::StaticClass()->ClassDefaultObject->ConditionalBeginDestroy();
		ULevelEditorPlaySettings::StaticClass()->ClassDefaultObject = OriginalLevelEditorPlaySettings;
		OriginalLevelEditorPlaySettings = nullptr;
	}

	if (OriginalSpatialGDKEditorSettings != nullptr)
	{
		// Restore original settings - delete CDO first and then replace with copied settings
		USpatialGDKEditorSettings::StaticClass()->ClassDefaultObject->ConditionalBeginDestroy();
		USpatialGDKEditorSettings::StaticClass()->ClassDefaultObject = OriginalSpatialGDKEditorSettings;
		OriginalSpatialGDKEditorSettings = nullptr;
	}

	if (OriginalGeneralProjectSettings != nullptr)
	{
		// Restore original settings - delete CDO first and then replace with copied settings
		UGeneralProjectSettings::StaticClass()->ClassDefaultObject->ConditionalBeginDestroy();
		UGeneralProjectSettings::StaticClass()->ClassDefaultObject = OriginalGeneralProjectSettings;
		OriginalGeneralProjectSettings = nullptr;
	}

	if (OriginalEditorPerformanceSettings != nullptr)
	{
		// Restore original settings - delete CDO first and then replace with copied settings
		UEditorPerformanceSettings::StaticClass()->ClassDefaultObject->ConditionalBeginDestroy();
		UEditorPerformanceSettings::StaticClass()->ClassDefaultObject = OriginalEditorPerformanceSettings;
		OriginalEditorPerformanceSettings = nullptr;
	}
}

void FSpatialTestSettings::Load(const FString& TestSettingOverridesFilename)
{
	GetMutableDefault<ULevelEditorPlaySettings>()->LoadConfig(ULevelEditorPlaySettings::StaticClass(), *TestSettingOverridesFilename);
	GetMutableDefault<USpatialGDKSettings>()->LoadConfig(USpatialGDKSettings::StaticClass(), *TestSettingOverridesFilename);
	GetMutableDefault<USpatialGDKEditorSettings>()->LoadConfig(USpatialGDKEditorSettings::StaticClass(), *TestSettingOverridesFilename);
	GetMutableDefault<UGeneralProjectSettings>()->LoadConfig(UGeneralProjectSettings::StaticClass(), *TestSettingOverridesFilename);
	GetMutableDefault<UEditorPerformanceSettings>()->LoadConfig(UEditorPerformanceSettings::StaticClass(), *TestSettingOverridesFilename);
}
