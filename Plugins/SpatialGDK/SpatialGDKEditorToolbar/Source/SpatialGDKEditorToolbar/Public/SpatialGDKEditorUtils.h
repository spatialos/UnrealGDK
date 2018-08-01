// Copyright (c) Improbable Worlds Ltd, All Rights Reserved
#pragma once

#include "CoreMinimal.h"

DECLARE_LOG_CATEGORY_EXTERN(LogSpatialGDKEditorToolbarUtils, Log, All)

const FConfigFile* LoadConfigFile(const FString& ConfigFilePath);

const FConfigSection* GetConfigSection(const FString& ConfigFilePath, const FString& SectionName);
