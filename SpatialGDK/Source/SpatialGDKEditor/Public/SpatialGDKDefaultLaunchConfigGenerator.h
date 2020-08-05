// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "Logging/LogMacros.h"

#include "SpatialGDKSettings.h"
#include "SpatialGDKEditorSettings.h"

DECLARE_LOG_CATEGORY_EXTERN(LogSpatialGDKDefaultLaunchConfigGenerator, Log, All);

class UAbstractRuntimeLoadBalancingStrategy;
struct FSpatialLaunchConfigDescription;

TMap<FName, uint32> SPATIALGDKEDITOR_API GetLayerWorkerCountMappingFromWorldSettings(const UWorld& World);

uint32 SPATIALGDKEDITOR_API GetTotalWorkerCountFromWorldSettings(const UWorld& World);

bool SPATIALGDKEDITOR_API FillWorkerConfigurationFromCurrentMap(FWorkerTypeLaunchSection& OutWorker, FIntPoint& OutWorldDimensions);

bool SPATIALGDKEDITOR_API GenerateLaunchConfig(const FString& LaunchConfigPath, const FSpatialLaunchConfigDescription* InLaunchConfigDescription, const FWorkerTypeLaunchSection& InWorker);

bool SPATIALGDKEDITOR_API ValidateGeneratedLaunchConfig(const FSpatialLaunchConfigDescription& LaunchConfigDesc, const FWorkerTypeLaunchSection& InWorker);
