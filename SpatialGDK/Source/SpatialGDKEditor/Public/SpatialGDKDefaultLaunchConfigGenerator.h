// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "Logging/LogMacros.h"

#include "SpatialGDKSettings.h"
#include "SpatialGDKEditorSettings.h"

DECLARE_LOG_CATEGORY_EXTERN(LogSpatialGDKDefaultLaunchConfigGenerator, Log, All);

class UAbstractRuntimeLoadBalancingStrategy;
struct FSpatialLaunchConfigDescription;

/** Set WorkerTypesToLaunch in level editor play settings. */
void SPATIALGDKEDITOR_API SetLevelEditorPlaySettingsWorkerTypes(const TMap<FName, FWorkerTypeLaunchSection>& InWorkers);

bool SPATIALGDKEDITOR_API GetLoadBalancingStrategyFromWorldSettings(const UWorld& World, UAbstractRuntimeLoadBalancingStrategy*& OutStrategy, FIntPoint& OutWorldDimension);

bool SPATIALGDKEDITOR_API FillWorkerConfigurationFromCurrentMap(TMap<FName, FWorkerTypeLaunchSection>& OutWorkers, FIntPoint& OutWorldDimensions);

bool SPATIALGDKEDITOR_API GenerateLaunchConfig(const FString& LaunchConfigPath, const FSpatialLaunchConfigDescription* InLaunchConfigDescription, const TMap<FName, FWorkerTypeLaunchSection>& InWorkers);

bool SPATIALGDKEDITOR_API ValidateGeneratedLaunchConfig(const FSpatialLaunchConfigDescription& LaunchConfigDesc, const TMap<FName, FWorkerTypeLaunchSection>& InWorkers);
