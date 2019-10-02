// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "Logging/LogMacros.h"

DECLARE_LOG_CATEGORY_EXTERN(LogSpatialGDKSchemaGenerator, Log, All);

namespace SpatialGDKEditor
{
	namespace Schema
	{
		SPATIALGDKEDITOR_API bool IsSupportedClass(const UClass* SupportedClass);

		SPATIALGDKEDITOR_API TSet<UClass*> GetAllSupportedClasses();
		
		SPATIALGDKEDITOR_API bool SpatialGDKGenerateSchema();
		
		SPATIALGDKEDITOR_API bool SpatialGDKGenerateSchemaForClasses(TSet<UClass*> Classes);
		
		SPATIALGDKEDITOR_API bool TryLoadExistingSchemaDatabase();
		
		SPATIALGDKEDITOR_API bool GeneratedSchemaDatabaseExists();
		
		SPATIALGDKEDITOR_API bool SaveSchemaDatabase();
		
		SPATIALGDKEDITOR_API bool DeleteSchemaDatabase();
		
		SPATIALGDKEDITOR_API void ClearGeneratedSchema();
		
		SPATIALGDKEDITOR_API bool GeneratedSchemaFolderExists();
		
		SPATIALGDKEDITOR_API void DeleteGeneratedSchemaFiles();
		
		SPATIALGDKEDITOR_API void CopyWellKnownSchemaFiles();
		
		SPATIALGDKEDITOR_API bool RunSchemaCompiler();
	}
}
