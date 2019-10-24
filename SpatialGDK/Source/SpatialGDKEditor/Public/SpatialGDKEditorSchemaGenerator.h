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
		
		SPATIALGDKEDITOR_API bool LoadGeneratorStateFromSchemaDatabase(FString FileName = "");

		SPATIALGDKEDITOR_API bool IsAssetReadOnly(FString FileName);
		
		SPATIALGDKEDITOR_API bool GeneratedSchemaDatabaseExists();
		
		SPATIALGDKEDITOR_API void GenerateSchemaForSublevels(const TMultiMap<FName, FName>& LevelNamesToPaths, const FString& SchemaPath);

		SPATIALGDKEDITOR_API bool SaveSchemaDatabase(FString PackagePath = "");
		
		SPATIALGDKEDITOR_API bool DeleteSchemaDatabase(FString PackagePath = "");
		
		SPATIALGDKEDITOR_API void ResetSchemaGeneratorState();

		SPATIALGDKEDITOR_API void ResetSchemaGeneratorStateAndCleanupFolders();
		
		SPATIALGDKEDITOR_API bool GeneratedSchemaFolderExists();
		
		SPATIALGDKEDITOR_API void DeleteGeneratedSchemaFiles(FString SchemaOutputPath = "");

		SPATIALGDKEDITOR_API void CreateGeneratedSchemaFolder();
		
		SPATIALGDKEDITOR_API void CopyWellKnownSchemaFiles(FString GDKSchemaCopyDir = "", FString CoreSDKSchemaCopyDir = "");
		
		SPATIALGDKEDITOR_API bool RunSchemaCompiler();
	}
}
