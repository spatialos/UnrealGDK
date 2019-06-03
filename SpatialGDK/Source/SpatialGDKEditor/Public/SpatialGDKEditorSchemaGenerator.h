// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "Logging/LogMacros.h"

DECLARE_LOG_CATEGORY_EXTERN(LogSpatialGDKSchemaGenerator, Log, All);

SPATIALGDKEDITOR_API bool SpatialGDKGenerateSchema();

SPATIALGDKEDITOR_API void ClearGeneratedSchema();

SPATIALGDKEDITOR_API void DeleteGeneratedSchemaFiles();

SPATIALGDKEDITOR_API bool TryLoadExistingSchemaDatabase();
