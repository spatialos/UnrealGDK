// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "Logging/LogMacros.h"

#include "SpatialGDKSettings.h"
#include "SpatialGDKEditorSettings.h"

DECLARE_LOG_CATEGORY_EXTERN(LogSpatialGDKDefaultLaunchConfigGenerator, Log, All);

class UAbstractRuntimeLoadBalancingStrategy;
struct FSpatialLaunchConfigDescription;

uint32 SPATIALGDKEDITOR_API GetWorkerCountFromWorldSettings(const UWorld& World);

bool SPATIALGDKEDITOR_API FillWorkerConfigurationFromCurrentMap(FWorkerTypeLaunchSection& OutWorker, FIntPoint& OutWorldDimensions);

bool SPATIALGDKEDITOR_API GenerateLaunchConfig(const FString& LaunchConfigPath, const FSpatialLaunchConfigDescription* InLaunchConfigDescription, const FWorkerTypeLaunchSection& InWorker);

bool SPATIALGDKEDITOR_API ValidateGeneratedLaunchConfig(FSpatialLaunchConfigDescription& LaunchConfigDesc, const FWorkerTypeLaunchSection& InWorker);
