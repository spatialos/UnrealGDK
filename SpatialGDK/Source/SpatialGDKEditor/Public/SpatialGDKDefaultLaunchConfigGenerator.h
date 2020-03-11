// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "Logging/LogMacros.h"

DECLARE_LOG_CATEGORY_EXTERN(LogSpatialGDKDefaultLaunchConfigGenerator, Log, All);

struct FSpatialLaunchConfigDescription;

bool SPATIALGDKEDITOR_API GenerateDefaultLaunchConfig(const FString& LaunchConfigPath, const FSpatialLaunchConfigDescription* InLaunchConfigDescription);

bool SPATIALGDKEDITOR_API ValidateGeneratedLaunchConfig(const FSpatialLaunchConfigDescription& LaunchConfigDesc);
