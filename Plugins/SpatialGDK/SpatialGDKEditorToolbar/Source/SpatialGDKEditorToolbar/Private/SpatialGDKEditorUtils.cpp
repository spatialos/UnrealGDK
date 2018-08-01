// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "SpatialGDKEditorUtils.h"

DEFINE_LOG_CATEGORY(LogSpatialGDKEditorToolbarUtils)

const FConfigFile* GetConfigFile(const FString& ConfigFilePath)
{
	const FConfigFile* ConfigFile = GConfig->Find(ConfigFilePath, false);
	if (!ConfigFile)
	{
		UE_LOG(LogSpatialGDKEditorToolbarUtils, Error, TEXT("Could not open .ini file: \"%s\""), *ConfigFilePath);
		return nullptr;
	}
	return ConfigFile;
}

const FConfigFile* LoadConfigFile(const FString& ConfigFilePath)
{
	GConfig->LoadFile(ConfigFilePath);
	return GetConfigFile(ConfigFilePath);
}

const FConfigSection* GetConfigSection(const FString& ConfigFilePath, const FString& SectionName)
{
	if (const FConfigFile* ConfigFile = GetConfigFile(ConfigFilePath))
	{
		if (const FConfigSection* Section = ConfigFile->Find(SectionName))
		{
			return Section;
		}
		else
		{
			UE_LOG(LogSpatialGDKEditorToolbarUtils, Error, TEXT("Could not find section '%s' in '%s'."), *SectionName, *ConfigFilePath);
		}
	}
	return nullptr;
}
