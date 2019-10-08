// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "TestDefinitions.h"
#include "SpatialGDKEditorSchemaGenerator.h"
#include "SpatialGDKServicesModule.h"

#include "CoreMinimal.h"

#define SCHEMA_GENERATOR_TEST(TestName) \
	TEST(SpatialGDKEditor, SchemaGenerator, TestName)

SCHEMA_GENERATOR_TEST(SOME_TEST)
{
	TSet<UClass*> Classes = SpatialGDKEditor::Schema::GetAllSupportedClasses();

	FString SchemaOutputFolder = FPaths::Combine(FSpatialGDKServicesModule::GetSpatialOSDirectory(), TEXT("Tests/"));

	bool bResult = SpatialGDKEditor::Schema::SpatialGDKGenerateSchemaForClasses(Classes, SchemaOutputFolder);

	SpatialGDKEditor::Schema::DeleteGeneratedSchemaFiles(SchemaOutputFolder);

	TestTrue("Run SchemaGeneration", bResult);

	return true;
}
