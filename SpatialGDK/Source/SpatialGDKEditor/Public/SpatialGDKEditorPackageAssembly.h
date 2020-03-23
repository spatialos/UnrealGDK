// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "Logging/LogMacros.h"

DECLARE_LOG_CATEGORY_EXTERN(LogSpatialGDKEditorPackageAssembly, Log, All);

SPATIALGDKEDITOR_API bool SpatialGDKBuildAssemblyServerWorker();

SPATIALGDKEDITOR_API bool SpatialGDKBuildAssemblyClientWorker();

SPATIALGDKEDITOR_API bool SpatialGDKBuildAssemblySimulatedPlayerWorker();

