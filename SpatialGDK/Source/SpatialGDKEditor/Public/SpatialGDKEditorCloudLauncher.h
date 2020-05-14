// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "Logging/LogMacros.h"

DECLARE_LOG_CATEGORY_EXTERN(LogSpatialGDKEditorCloudLauncher, Log, All);

struct FCloudDeploymentConfiguration;

SPATIALGDKEDITOR_API bool SpatialGDKCloudLaunch(const FCloudDeploymentConfiguration& Configuration);

SPATIALGDKEDITOR_API bool SpatialGDKCloudStop();
