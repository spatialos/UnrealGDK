// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "TestDefinitions.h"
#include "SpatialGDKEditorSchemaGenerator.h"
#include "SpatialGDKServicesModule.h"

#include "SchemaGenObjectStub.h"

#include "CoreMinimal.h"

#define SCHEMA_GENERATOR_TEST(TestName) \
	TEST(SpatialGDKEditor, SchemaGenerator, TestName)

SCHEMA_GENERATOR_TEST(SOME_TEST)
{
	UClass* ObjectStubClass = USchemaGenObjectStub::StaticClass();
	TSet<UClass*> Classes;
	Classes.Add(ObjectStubClass);
	//TSet<UClass*> Classes = SpatialGDKEditor::Schema::GetAllSupportedClasses();

	FString SchemaOutputFolder = FPaths::Combine(FSpatialGDKServicesModule::GetSpatialOSDirectory(), TEXT("Tests/"));
	FString DatabaseAssetPath = TEXT("/Game/Spatial/Tests/SchemaDatabase");

	// TODO(Alex): try load schemadatabase, SpatialGDKGenerateSchemaForClasses should take a ptr to database
	bool bResult = SpatialGDKEditor::Schema::SpatialGDKGenerateSchemaForClasses(Classes, SchemaOutputFolder);
	bResult &= SpatialGDKEditor::Schema::SaveSchemaDatabase(DatabaseAssetPath);

	FString OtherDatabaseAssetPath = TEXT("Spatial/Tests/SchemaDatabase");
	bResult &= SpatialGDKEditor::Schema::DeleteSchemaDatabase(OtherDatabaseAssetPath);
	SpatialGDKEditor::Schema::DeleteGeneratedSchemaFiles(SchemaOutputFolder);

	TestTrue("Run SchemaGeneration", bResult);

	SpatialGDKEditor::Schema::RunSchemaCompiler();

	return true;
}
