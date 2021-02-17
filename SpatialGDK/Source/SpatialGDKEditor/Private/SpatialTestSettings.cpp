// Fill out your copyright notice in the Description page of Project Settings.
#pragma optimize("", on)
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
	Duplicate<USpatialGDKSettings>(OriginalSpatialGDKSettings);
	Duplicate<ULevelEditorPlaySettings>(OriginalLevelEditorPlaySettings);
	Duplicate<USpatialGDKEditorSettings>(OriginalSpatialGDKEditorSettings);
	Duplicate<UGeneralProjectSettings>(OriginalGeneralProjectSettings);
	Duplicate<UEditorPerformanceSettings>(OriginalEditorPerformanceSettings);
	// First override the settings from the base config file, if it exists
	Load(BaseOverridesFilename);
	// Then override the settings from the map specific config file, if it exists
	FString MapOverridesFilename = OverrideSettingsBaseFilename + FPackageName::GetShortName(MapName) + (OverrideSettingsFileExtension);
	Load(MapOverridesFilename);
}

template <typename T>
void FSpatialTestSettings::Duplicate(T*& OriginalSettings)
{
	// Duplicate original settings but use different outer - if same outer is reused the object is not duplicated and pointer is to the same
	// Object Having the same name causes runtime exceptions
	// Use additional object flag RF_Standalone to avoid early GC - destroy when restoring settings
	OriginalSettings = NewObject<T>(GetTransientPackage(), T::StaticClass(), NAME_None,
											EObjectFlags(RF_Public | RF_ClassDefaultObject | RF_ArchetypeObject | RF_Standalone),
											  GetMutableDefault<T>());
}

void FSpatialTestSettings::Revert()
{
	Restore<USpatialGDKSettings>(OriginalSpatialGDKSettings);
	Restore<ULevelEditorPlaySettings>(OriginalLevelEditorPlaySettings);
	Restore<USpatialGDKEditorSettings>(OriginalSpatialGDKEditorSettings);
	Restore<UGeneralProjectSettings>(OriginalGeneralProjectSettings);
	Restore<UEditorPerformanceSettings>(OriginalEditorPerformanceSettings);
}

template <typename T>
void FSpatialTestSettings::Restore(T*& OriginalSettings)
{
	if (OriginalSettings != nullptr)
	{
		// Restore original settings - delete CDO first and then replace with copied settings
		T::StaticClass()->ClassDefaultObject->ConditionalBeginDestroy();
		T::StaticClass()->ClassDefaultObject = OriginalSettings;
		OriginalSettings = nullptr;
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
#pragma optimize("", off)
