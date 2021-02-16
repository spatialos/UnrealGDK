// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "Editor/EditorPerformanceSettings.h"
#include "GeneralProjectSettings.h"
#include "Settings/LevelEditorPlaySettings.h"
#include "SpatialGDKEditorSettings.h"
#include "SpatialGDKSettings.h"

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

	// Backup up original settings in memory and then overrides them with the new settings specified in the config files
	void Override(const FString& MapName);

	// Restores the original settings
	void Restore();

protected:
	// Backup the original settings so they can be restored later
	void Backup();

	// Load settings from config file which will override current settings
	void Load(const FString& TestSettingOverridesFilename);

	// Extension for config files
	const FString OverrideSettingsFileExtension = TEXT(".ini");
	// Map override config base filename applied to specific map, if exists
	const FString OverrideSettingsBaseFilename = FPaths::ProjectConfigDir() + TEXT("TestOverrides");
	// Base override config file applied to all maps, if exists
	const FString BaseOverridesFilename = OverrideSettingsBaseFilename + TEXT("Base") + (OverrideSettingsFileExtension);

	// Settings classes that can be overridden using config files
	USpatialGDKSettings* OriginalSpatialGDKSettings;
	ULevelEditorPlaySettings* OriginalLevelEditorPlaySettings;
	USpatialGDKEditorSettings* OriginalSpatialGDKEditorSettings;
	UGeneralProjectSettings* OriginalGeneralProjectSettings;
	UEditorPerformanceSettings* OriginalEditorPerformanceSettings;
};
