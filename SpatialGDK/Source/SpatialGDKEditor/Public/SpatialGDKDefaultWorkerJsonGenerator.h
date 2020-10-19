// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "Logging/LogMacros.h"

DECLARE_LOG_CATEGORY_EXTERN(LogSpatialGDKDefaultWorkerJsonGenerator, Log, All);

SPATIALGDKEDITOR_API bool GenerateAllDefaultWorkerJsons(bool& bOutRedeployRequired);
SPATIALGDKEDITOR_API bool GenerateDefaultWorkerJson(const FString& WorkerJsonName, const FString& WorkerTypeName,
													bool& bOutRedeployRequired);
