// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "SpatialTestSettings.h"

#include "Engine/World.h"
#include "EngineClasses/SpatialWorldSettings.h"

DEFINE_LOG_CATEGORY(LogSpatialTestSettings);

FSpatialTestSettings::FSpatialTestSettings()
	: OriginalSpatialGDKSettings(nullptr)
	, OriginalLevelEditorPlaySettings(nullptr)
	, OriginalSpatialGDKEditorSettings(nullptr)
	, OriginalGeneralProjectSettings(nullptr)
	, OriginalEditorPerformanceSettings(nullptr)
{
}

const FString FSpatialTestSettings::OverrideSettingsFileExtension = TEXT(".ini");
const FString FSpatialTestSettings::OverrideSettingsFileDirectoryName = TEXT("MapSettingsOverrides");
const FString FSpatialTestSettings::OverrideSettingsFilePrefix = TEXT("TestOverrides");
const FString FSpatialTestSettings::OverrideSettingsBaseFilename =
	FPaths::ConvertRelativePathToFull(FPaths::ProjectConfigDir() / OverrideSettingsFileDirectoryName);
const FString FSpatialTestSettings::BaseOverridesFilename = OverrideSettingsBaseFilename + TEXT("Base") + OverrideSettingsFileExtension;
const FString FSpatialTestSettings::GeneratedOverrideSettingsDirectory =
	FPaths::ConvertRelativePathToFull(FPaths::ProjectIntermediateDir() / TEXT("Config/") / OverrideSettingsFileDirectoryName);
const FString FSpatialTestSettings::GeneratedOverrideSettingsBaseFilename = GeneratedOverrideSettingsDirectory / OverrideSettingsFilePrefix;

void FSpatialTestSettings::Override(const FString& MapName)
{
	// Back up the existing settings so they can be reverted later
	Duplicate(OriginalSpatialGDKSettings);
	Duplicate(OriginalLevelEditorPlaySettings);
	Duplicate(OriginalSpatialGDKEditorSettings);
	Duplicate(OriginalGeneralProjectSettings);
	Duplicate(OriginalEditorPerformanceSettings);

	// Base config, applies to all maps if present
	if (FPaths::FileExists(BaseOverridesFilename))
	{
		// Override the settings from the base config file
		Load(BaseOverridesFilename);
	}

	// Group config, applies to maps with the group config set in the Spatial World Settings
	const UWorld* World = GEditor->GetEditorWorldContext().World();
	check(World != nullptr);
	
	if (ASpatialWorldSettings* SpatialWorldSettings = Cast<ASpatialWorldSettings>(World->GetWorldSettings()))
	{
		FString GroupOverridesFilename =
			OverrideSettingsBaseFilename + "/" + SpatialWorldSettings->GroupConfigFilename + (OverrideSettingsFileExtension);
		if (FPaths::FileExists(GroupOverridesFilename))
		{
			// Override the settings from the group specific config file, if it exists
			Load(GroupOverridesFilename);
		}

	}

	// Map config, applied to the specific map
	FString MapOverridesFilename = OverrideSettingsBaseFilename + "/" + OverrideSettingsFilePrefix + FPackageName::GetShortName(MapName)
								   + (OverrideSettingsFileExtension);
	if (FPaths::FileExists(MapOverridesFilename))
	{
		// Override the settings from the map specific config file
		Load(MapOverridesFilename);
	}
	FString GeneratedMapOverridesFilename =
		GeneratedOverrideSettingsBaseFilename + FPackageName::GetShortName(MapName) + (OverrideSettingsFileExtension);
	if (FPaths::FileExists(GeneratedMapOverridesFilename))
	{
		// Override the settings from the generated map specific config file
		Load(GeneratedMapOverridesFilename);
	}

}

template <typename T>
void FSpatialTestSettings::Duplicate(T*& OriginalSettings)
{
	// Duplicate original settings but use different outer - if same outer is reused the object is not duplicated and pointer is to the same
	// Object Having the same name causes runtime exceptions
	// Add to root to prevent GC
	T* DuplicateSettings = NewObject<T>(GetTransientPackage(), T::StaticClass(), NAME_None, RF_NoFlags, GetMutableDefault<T>());
	DuplicateSettings->AddToRoot();
	OriginalSettings = GetMutableDefault<T>();
	OriginalSettings->AddToRoot();
	T::StaticClass()->ClassDefaultObject = DuplicateSettings;
}

void FSpatialTestSettings::Revert()
{
	Restore(OriginalSpatialGDKSettings);
	Restore(OriginalLevelEditorPlaySettings);
	Restore(OriginalSpatialGDKEditorSettings);
	Restore(OriginalGeneralProjectSettings);
	Restore(OriginalEditorPerformanceSettings);
}

template <typename T>
void FSpatialTestSettings::Restore(T*& OriginalSettings)
{
	if (OriginalSettings != nullptr)
	{
		// Restore original settings - delete CDO first and then replace with copied settings
		T::StaticClass()->ClassDefaultObject->RemoveFromRoot();
		T::StaticClass()->ClassDefaultObject = OriginalSettings;
		OriginalSettings->RemoveFromRoot();
		OriginalSettings = nullptr;
	}
}

void FSpatialTestSettings::Load(const FString& TestSettingOverridesFilename)
{
	UE_LOG(LogSpatialTestSettings, Log, TEXT("Overriding settings from file %s."), *TestSettingOverridesFilename);
	GetMutableDefault<ULevelEditorPlaySettings>()->LoadConfig(ULevelEditorPlaySettings::StaticClass(), *TestSettingOverridesFilename);
	GetMutableDefault<USpatialGDKSettings>()->LoadConfig(USpatialGDKSettings::StaticClass(), *TestSettingOverridesFilename);
	GetMutableDefault<USpatialGDKEditorSettings>()->LoadConfig(USpatialGDKEditorSettings::StaticClass(), *TestSettingOverridesFilename);
	GetMutableDefault<UGeneralProjectSettings>()->LoadConfig(UGeneralProjectSettings::StaticClass(), *TestSettingOverridesFilename);
	GetMutableDefault<UEditorPerformanceSettings>()->LoadConfig(UEditorPerformanceSettings::StaticClass(), *TestSettingOverridesFilename);
}
