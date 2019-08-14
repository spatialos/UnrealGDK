// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "Logging/LogMacros.h"

DECLARE_LOG_CATEGORY_EXTERN(LogSpatialGDKDefaultLaunchConfigGenerator, Log, All);

bool SPATIALGDKEDITOR_API GenerateDefaultLaunchConfig(const FString& LaunchConfigPath);
