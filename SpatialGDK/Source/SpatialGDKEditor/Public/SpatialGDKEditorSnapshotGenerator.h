// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "Logging/LogMacros.h"

DECLARE_LOG_CATEGORY_EXTERN(LogSpatialGDKSnapshot, Log, All);

SPATIALGDKEDITOR_API bool SpatialGDKGenerateSnapshot(class UWorld* World, FString SnapshotFilename);
