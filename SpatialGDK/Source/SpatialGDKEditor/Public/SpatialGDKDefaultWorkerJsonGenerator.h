// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "Logging/LogMacros.h"

DECLARE_LOG_CATEGORY_EXTERN(LogSpatialGDKDefaultWorkerJsonGenerator, Log, All);

SPATIALGDKEDITOR_API bool GenerateDefaultWorkerJson(bool& bRedeployRequired);
