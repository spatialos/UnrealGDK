// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "TestDefinitions.h"
#include "SpatialGDKEditorSchemaGenerator.h"
#include "SpatialGDKServicesModule.h"

#include "CoreMinimal.h"

#define SCHEMA_GENERATOR_TEST(TestName) \
	TEST(SpatialGDKEditor, SchemaGenerator, TestName)

SCHEMA_GENERATOR_TEST(GIVEN_supported_class_WHEN_checked_if_supported_THEN_is_supported)
{
	//SPATIALGDKEDITOR_API bool IsSupportedClass(const UClass* SupportedClass);
	TestTrue("", false);
	return true;
}

SCHEMA_GENERATOR_TEST(GIVEN_unsupported_class_WHEN_checked_if_supported_THEN_is_not_supported)
{
	//SPATIALGDKEDITOR_API bool IsSupportedClass(const UClass* SupportedClass);
	TestTrue("", false);
	return true;
}

SCHEMA_GENERATOR_TEST(GIVEN_multiple_classes_WHEN_generated_schema_for_these_classes_THEN_corresponding_schema_files_exist)
{
	//SPATIALGDKEDITOR_API bool SpatialGDKGenerateSchemaForClasses(TSet<UClass*> Classes, FString SchemaOutputPath = "");
	TestTrue("", false);
	return true;
}

SCHEMA_GENERATOR_TEST(GIVEN_an_Actor_class_WHEN_generated_schema_for_this_class_THEN_a_file_with_valid_schema_exists)
{
	//SPATIALGDKEDITOR_API bool SpatialGDKGenerateSchemaForClasses(TSet<UClass*> Classes, FString SchemaOutputPath = "");
	TestTrue("", false);
	return true;
}

SCHEMA_GENERATOR_TEST(GIVEN_multiple_Actor_class_WHEN_generated_schema_for_this_class_THEN_files_with_valid_schema_exist)
{
	//SPATIALGDKEDITOR_API bool SpatialGDKGenerateSchemaForClasses(TSet<UClass*> Classes, FString SchemaOutputPath = "");
	TestTrue("", false);
	return true;
}

SCHEMA_GENERATOR_TEST(GIVEN_an_Actor_component_class_WHEN_generated_schema_for_this_class_THEN_a_file_with_valid_schema_exists)
{
	//SPATIALGDKEDITOR_API bool SpatialGDKGenerateSchemaForClasses(TSet<UClass*> Classes, FString SchemaOutputPath = "");
	TestTrue("", false);
	return true;
}

SCHEMA_GENERATOR_TEST(GIVEN_an_Actor_class_with_an_actor_component_WHEN_generated_schema_for_this_class_THEN_a_file_with_valid_schema_exists)
{
	//SPATIALGDKEDITOR_API bool SpatialGDKGenerateSchemaForClasses(TSet<UClass*> Classes, FString SchemaOutputPath = "");
	return true;
}

SCHEMA_GENERATOR_TEST(GIVEN_an_Actor_class_with_multiple_actor_components_WHEN_generated_schema_for_this_class_THEN_files_with_valid_schema_exist)
{
	//SPATIALGDKEDITOR_API bool SpatialGDKGenerateSchemaForClasses(TSet<UClass*> Classes, FString SchemaOutputPath = "");
	TestTrue("", false);
	return true;
}

SCHEMA_GENERATOR_TEST(GIVEN_an_Actor_class_with_multiple_object_components_WHEN_generated_schema_for_this_class_THEN_files_with_valid_schema_exist)
{
	//SPATIALGDKEDITOR_API bool SpatialGDKGenerateSchemaForClasses(TSet<UClass*> Classes, FString SchemaOutputPath = "");
	TestTrue("", false);
	return true;
}

SCHEMA_GENERATOR_TEST(GIVEN_multiple_schema_files_exist_WHEN_deleted_generated_files_THEN_no_schema_files_exist)
{
	//SPATIALGDKEDITOR_API void DeleteGeneratedSchemaFiles(FString SchemaOutputPath = "");
	TestTrue("", false);
	return true;
}

SCHEMA_GENERATOR_TEST(GIVEN_no_schema_files_exist_WHEN_deleted_generated_files_THEN_no_schema_files_exist)
{
	//SPATIALGDKEDITOR_API void DeleteGeneratedSchemaFiles(FString SchemaOutputPath = "");
	TestTrue("", false);
	return true;
}

SCHEMA_GENERATOR_TEST(GIVEN_multiple_classes_with_schema_generated_WHEN_schema_database_saved_THEN_schema_database_exists)
{
	//SPATIALGDKEDITOR_API bool SaveSchemaDatabase(FString PackagePath = "");
	TestTrue("", false);
	return true;
}

SCHEMA_GENERATOR_TEST(GIVEN_a_class_with_schema_generated_WHEN_schema_database_saved_THEN_valid_schema_database_exists)
{
	//SPATIALGDKEDITOR_API bool SaveSchemaDatabase(FString PackagePath = "");
	TestTrue("", false);
	return true;
}

SCHEMA_GENERATOR_TEST(GIVEN_multiple_classes_with_schema_generated_WHEN_schema_database_saved_THEN_valid_schema_database_exists)
{
	//SPATIALGDKEDITOR_API bool SaveSchemaDatabase(FString PackagePath = "");
	TestTrue("", false);
	return true;
}

SCHEMA_GENERATOR_TEST(GIVEN_schema_database_exists_WHEN_schema_database_deleted_THEN_no_schema_database_exists)
{
	//SPATIALGDKEDITOR_API bool DeleteSchemaDatabase(FString PackagePath = "");
	TestTrue("", false);
	return true;
}

SCHEMA_GENERATOR_TEST(GIVEN_schema_database_exists_WHEN_tried_to_load_THEN_loaded)
{
	//SPATIALGDKEDITOR_API bool TryLoadExistingSchemaDatabase();
	TestTrue("", false);
	return true;
}

SCHEMA_GENERATOR_TEST(GIVEN_schema_database_does_not_exist_WHEN_tried_to_load_THEN_not_loaded)
{
	//SPATIALGDKEDITOR_API bool TryLoadExistingSchemaDatabase();
	TestTrue("", false);
	return true;
}

SCHEMA_GENERATOR_TEST(GIVEN_source_and_destination_of_well_known_schema_files_WHEN_copied_THEN_valid_files_exist)
{
	//SPATIALGDKEDITOR_API void CopyWellKnownSchemaFiles();
	TestTrue("", false);
	return true;
}

SCHEMA_GENERATOR_TEST(GIVEN_multiple_classes_WHEN_getting_all_supported_classes_THEN_all_unsupported_classes_are_filtered)
{
	//SPATIALGDKEDITOR_API TSet<UClass*> GetAllSupportedClasses();
	TestTrue("", false);
	return true;
}
