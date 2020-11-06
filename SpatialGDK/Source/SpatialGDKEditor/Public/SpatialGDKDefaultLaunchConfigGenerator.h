// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "Logging/LogMacros.h"

#include "SpatialGDKEditorSettings.h"
#include "SpatialGDKSettings.h"

DECLARE_LOG_CATEGORY_EXTERN(LogSpatialGDKDefaultLaunchConfigGenerator, Log, All);

class UAbstractRuntimeLoadBalancingStrategy;
struct FSpatialLaunchConfigDescription;

uint32 SPATIALGDKEDITOR_API GetWorkerCountFromWorldSettings(const UWorld& World, bool bForceNonEditorSettings = false);

bool SPATIALGDKEDITOR_API GenerateLaunchConfig(const FString& LaunchConfigPath,
											   const FSpatialLaunchConfigDescription* InLaunchConfigDescription, bool bGenerateCloudConfig);

bool SPATIALGDKEDITOR_API ConvertToClassicConfig(const FString& LaunchConfigPath,
												 const FSpatialLaunchConfigDescription* InLaunchConfigDescription);

bool SPATIALGDKEDITOR_API ValidateGeneratedLaunchConfig(const FSpatialLaunchConfigDescription& LaunchConfigDesc);
