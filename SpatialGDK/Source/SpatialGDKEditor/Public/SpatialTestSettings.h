// Copyright (c) Improbable Worlds Ltd, All Rights Reserved
#pragma optimize("", off)
#pragma once

#include "CoreMinimal.h"
#include "Editor/EditorPerformanceSettings.h"
#include "GeneralProjectSettings.h"
#include "Logging/LogMacros.h"
#include "Settings/LevelEditorPlaySettings.h"
#include "SpatialGDKEditorSettings.h"
#include "SpatialGDKSettings.h"

DECLARE_LOG_CATEGORY_EXTERN(LogSpatialTestSettings, Log, All);

/**
 * Helper class for Spatial Functional Tests to allow users to override settings using config files.
 *
 * The currently supported settings classes for overrides are:
 *		SpatialGDKSettings
 *		SpatialGDKEditorSettings
 *		GeneralProjectSettings
 *		LevelEditorSettings
 *		EditorPerformanceSettings
 *
 * To add a new settings class, add a pointer to the class in here and update Backup, Load and Restore functions.
 */
class FSpatialTestSettings
{
public:
	FSpatialTestSettings();

	// Backup up the original settings in memory and then override them with the new settings specified in the config files
	void Override(const FString& MapName);

	// Restores the original settings
	void Revert();

protected:
	// Duplicate an original setting
	template <typename T>
	void Duplicate(T*& OriginalSettings);

	// Restore an original setting
	template <typename T>
	void Restore(T*& OriginalSettings);

	// Load settings from config file which will override current settings
	void Load(const FString& TestSettingOverridesFilename);

	const FString OverrideSettingsFileExtension = TEXT(".ini");
	const FString OverrideSettingsFilePrefix = TEXT("MapSettingsOverrides/TestOverrides");
	// Map override config base filename applied to specific map, if exists
	const FString OverrideSettingsBaseFilename = FPaths::ProjectConfigDir() + OverrideSettingsFilePrefix;
	// Base override config file applied to all maps, if exists
	const FString BaseOverridesFilename = OverrideSettingsBaseFilename + TEXT("Base") + (OverrideSettingsFileExtension);
	// Generated map override config base filename for generated maps applied to specific map, if exists
	const FString GeneratedOverrideSettingsBaseFilename = FPaths::ProjectIntermediateDir() + TEXT("Config/") + OverrideSettingsFilePrefix;

	// Settings classes that can be overridden using config files
	USpatialGDKSettings* OriginalSpatialGDKSettings;
	ULevelEditorPlaySettings* OriginalLevelEditorPlaySettings;
	USpatialGDKEditorSettings* OriginalSpatialGDKEditorSettings;
	UGeneralProjectSettings* OriginalGeneralProjectSettings;
	UEditorPerformanceSettings* OriginalEditorPerformanceSettings;
};
#pragma optimize("", on)
