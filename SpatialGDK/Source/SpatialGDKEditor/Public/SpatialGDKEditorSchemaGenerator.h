// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "Logging/LogMacros.h"

DECLARE_LOG_CATEGORY_EXTERN(LogSpatialGDKSchemaGenerator, Log, All);

namespace SpatialGDKEditor
{
namespace Schema
{
SPATIALGDKEDITOR_API bool IsSupportedClass(const UClass* SupportedClass);

SPATIALGDKEDITOR_API TSet<UClass*> GetAllSupportedClasses(const TArray<UObject*>& AllClasses);

SPATIALGDKEDITOR_API bool SpatialGDKGenerateSchema();

SPATIALGDKEDITOR_API bool SpatialGDKGenerateSchemaForClasses(TSet<UClass*> Classes, FString SchemaOutputPath = "");

SPATIALGDKEDITOR_API void GenerateSchemaForSublevels();

SPATIALGDKEDITOR_API void GenerateSchemaForSublevels(const FString& SchemaOutputPath, const TMultiMap<FName, FName>& LevelNamesToPaths);

SPATIALGDKEDITOR_API void GenerateSchemaForRPCEndpoints();

SPATIALGDKEDITOR_API void GenerateSchemaForRPCEndpoints(const FString& SchemaOutputPath);

SPATIALGDKEDITOR_API void GenerateSchemaForNCDs();

SPATIALGDKEDITOR_API void GenerateSchemaForNCDs(const FString& SchemaOutputPath);

SPATIALGDKEDITOR_API bool LoadGeneratorStateFromSchemaDatabase(const FString& FileName);

SPATIALGDKEDITOR_API bool IsAssetReadOnly(const FString& FileName);

SPATIALGDKEDITOR_API bool GeneratedSchemaDatabaseExists();

SPATIALGDKEDITOR_API bool SaveSchemaDatabase(const FString& PackagePath);

SPATIALGDKEDITOR_API bool DeleteSchemaDatabase(const FString& PackagePath);

SPATIALGDKEDITOR_API void ResetSchemaGeneratorState();

SPATIALGDKEDITOR_API void ResetSchemaGeneratorStateAndCleanupFolders();

SPATIALGDKEDITOR_API bool GeneratedSchemaFolderExists();

SPATIALGDKEDITOR_API bool RefreshSchemaFiles(const FString& SchemaOutputPath);

SPATIALGDKEDITOR_API void CopyWellKnownSchemaFiles(const FString& GDKSchemaCopyDir, const FString& CoreSDKSchemaCopyDir);

SPATIALGDKEDITOR_API bool RunSchemaCompiler();
} // namespace Schema
} // namespace SpatialGDKEditor
