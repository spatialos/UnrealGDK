// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "TestDefinitions.h"
#include "SpatialGDKEditorSchemaGenerator.h"
#include "SpatialGDKServicesModule.h"
#include "SchemaGenObjectStub.h"
#include "ExpectedGeneratedSchemaFileContents.h"

#include "CoreMinimal.h"
#include "HAL/PlatformFilemanager.h"
#include "Misc/FileHelper.h"

#define SCHEMA_GENERATOR_TEST(TestName) \
	TEST(SpatialGDKEditor, SchemaGenerator, TestName)

namespace
{
FString LoadSchemaFileForClass(const FString& SchemaOutputFolder, const UClass* CurrentClass)
{
	FString SchemaFileFolder = TEXT("");

	if (!CurrentClass->IsChildOf<AActor>())
	{
		SchemaFileFolder = TEXT("Subobjects");
	}

	FString FileContent;
	FFileHelper::LoadFileToString(FileContent, *FPaths::SetExtension(FPaths::Combine(FPaths::Combine(SchemaOutputFolder, SchemaFileFolder), CurrentClass->GetName()), TEXT(".schema")));

	return FileContent;
}

int id = 0;

void ResetID()
{
	id = 10000;
}

int GetID()
{
	return id++;
}

TMap<FString, FString> ExpectedContents =
{
	TPair<FString, FString>
		{
			"SpatialTypeActor",
			ExpectedFileContent::ASpatialTypeActor
		},
	TPair<FString, FString>
		{
			"NonSpatialTypeActor",
			ExpectedFileContent::ANonSpatialTypeActor
		},
	TPair<FString, FString>
		{
			"SpatialTypeActorComponent",
			ExpectedFileContent::ASpatialTypeActorComponent
		},
	TPair<FString, FString>
		{
			"SpatialTypeActorWithActorComponent",
			ExpectedFileContent::ASpatialTypeActorWithActorComponent
		},
	TPair<FString, FString>
		{
			"SpatialTypeActorWithMultipleActorComponents",
			ExpectedFileContent::ASpatialTypeActorWithMultipleActorComponents
		},
	TPair<FString, FString>
		{
			"SpatialTypeActorWithMultipleObjectComponents",
			ExpectedFileContent::ASpatialTypeActorWithMultipleObjectComponents
		}
};

bool ValidateGeneratedSchemaForClass(const FString& FileContent, const UClass* CurrentClass)
{
	if (FString* ExpectedContentPtr = ExpectedContents.Find(CurrentClass->GetName()))
	{
		FString ExpectedContent = *ExpectedContentPtr;
		ExpectedContent.ReplaceInline(TEXT("{{id}}"), *FString::FromInt(GetID()));
		return (FileContent.Compare(ExpectedContent) == 0);
	}
	else
	{
		return false;
	}
}
}

SCHEMA_GENERATOR_TEST(GIVEN_spatial_type_class_WHEN_checked_if_supported_THEN_is_supported)
{
	// GIVEN
	const UClass* SupportedClass = USpatialTypeObjectStub::StaticClass();

	// WHEN
	bool bIsSupported = SpatialGDKEditor::Schema::IsSupportedClass(SupportedClass);

	// THEN
	TestTrue("Spatial type class is supported", bIsSupported);
	return true;
}

SCHEMA_GENERATOR_TEST(GIVEN_class_derived_from_spatial_type_class_WHEN_checked_if_supported_THEN_is_supported)
{
	// GIVEN
	const UClass* SupportedClass = UChildOfSpatialTypeObjectStub::StaticClass();

	// WHEN
	bool bIsSupported = SpatialGDKEditor::Schema::IsSupportedClass(SupportedClass);

	// THEN
	TestTrue("Child of a Spatial type class is supported", bIsSupported);
	return true;
}

SCHEMA_GENERATOR_TEST(GIVEN_null_pointer_WHEN_checked_if_supported_THEN_is_not_supported)
{
	// GIVEN
	const UClass* SupportedClass = nullptr;

	// WHEN
	bool bIsSupported = SpatialGDKEditor::Schema::IsSupportedClass(SupportedClass);

	// THEN
	TestFalse("Null pointer is not a valid argument", bIsSupported);
	return true;
}

SCHEMA_GENERATOR_TEST(GIVEN_non_spatial_type_class_WHEN_checked_if_supported_THEN_is_not_supported)
{
	// GIVEN
	const UClass* SupportedClass = UNotSpatialTypeObjectStub::StaticClass();

	// WHEN
	bool bIsSupported = SpatialGDKEditor::Schema::IsSupportedClass(SupportedClass);

	// THEN
	TestFalse("Non spatial type class is not supported", bIsSupported);
	return true;
}

SCHEMA_GENERATOR_TEST(GIVEN_class_derived_from_non_spatial_type_class_WHEN_checked_if_supported_THEN_is_not_supported)
{
	// GIVEN
	const UClass* SupportedClass = UChildOfNotSpatialTypeObjectStub::StaticClass();

	// WHEN
	bool bIsSupported = SpatialGDKEditor::Schema::IsSupportedClass(SupportedClass);

	// THEN
	TestFalse("Child of Non-Spatial type class is not supported", bIsSupported);
	return true;
}

SCHEMA_GENERATOR_TEST(GIVEN_a_class_with_not_spatial_tag_WHEN_checked_if_supported_THEN_is_not_supported)
{
	// GIVEN
	const UClass* SupportedClass = UNotSpatialTypeObjectStub::StaticClass();

	// WHEN
	bool bIsSupported = SpatialGDKEditor::Schema::IsSupportedClass(SupportedClass);

	// THEN
	TestFalse("Class with Not Spatial flag is not supported", bIsSupported);
	return true;
}

SCHEMA_GENERATOR_TEST(GIVEN_a_class_without_any_spatial_tags_WHEN_checked_if_supported_THEN_is_not_supported)
{
	// GIVEN
	const UClass* SupportedClass = UNoSpatialFlagsObjectStub ::StaticClass();

	// WHEN
	bool bIsSupported = SpatialGDKEditor::Schema::IsSupportedClass(SupportedClass);

	// THEN
	TestFalse("Class without Spatial flags is not supported", bIsSupported);
	return true;
}

SCHEMA_GENERATOR_TEST(GIVEN_child_of_a_class_without_any_spatial_tags_WHEN_checked_if_supported_THEN_is_not_supported)
{
	// GIVEN
	const UClass* SupportedClass = UChildOfNoSpatialFlagsObjectStub::StaticClass();

	// WHEN
	bool bIsSupported = SpatialGDKEditor::Schema::IsSupportedClass(SupportedClass);

	// THEN
	TestFalse("Child class of class without Spatial flags is not supported", bIsSupported);
	return true;
}

SCHEMA_GENERATOR_TEST(GIVEN_multiple_classes_WHEN_generated_schema_for_these_classes_THEN_corresponding_schema_files_exist)
{
	// GIVEN
	TSet<UClass*> Classes;
	// TODO(Alex): Don't have that many classes?
	Classes.Add(USchemaGenObjectStub::StaticClass());
	Classes.Add(USpatialTypeObjectStub::StaticClass());
	Classes.Add(UChildOfSpatialTypeObjectStub::StaticClass());
	Classes.Add(UNotSpatialTypeObjectStub::StaticClass());
	Classes.Add(UChildOfNotSpatialTypeObjectStub::StaticClass());
	Classes.Add(UNoSpatialFlagsObjectStub::StaticClass());
	Classes.Add(UChildOfNoSpatialFlagsObjectStub::StaticClass());
	Classes.Add(ASpatialTypeActor::StaticClass());

	FString SchemaOutputFolder = FPaths::Combine(FSpatialGDKServicesModule::GetSpatialOSDirectory(), TEXT("Tests/"));
	SpatialGDKEditor::Schema::ResetSchemaGeneratorState();
	ResetID();

	// WHEN
	SpatialGDKEditor::Schema::SpatialGDKGenerateSchemaForClasses(Classes, SchemaOutputFolder);

	// THEN
	bool bExpectedFilesExist = true;
	for (const auto& CurrentClass : Classes)
	{
		if (LoadSchemaFileForClass(SchemaOutputFolder, CurrentClass).IsEmpty())
		{
			bExpectedFilesExist = false;
			break;
		}
	}

	TestTrue("All expected schema files have been generated", bExpectedFilesExist);

	// CLEANUP
	SpatialGDKEditor::Schema::DeleteGeneratedSchemaFiles(SchemaOutputFolder);

	return true;
}

SCHEMA_GENERATOR_TEST(GIVEN_an_Actor_class_WHEN_generated_schema_for_this_class_THEN_a_file_with_valid_schema_exists)
{
	// GIVEN
	UClass* CurrentClass = ASpatialTypeActor::StaticClass();
	TSet<UClass*> Classes;
	Classes.Add(CurrentClass);

	FString SchemaOutputFolder = FPaths::Combine(FSpatialGDKServicesModule::GetSpatialOSDirectory(), TEXT("Tests/"));
	SpatialGDKEditor::Schema::ResetSchemaGeneratorState();
	ResetID();

	// WHEN
	SpatialGDKEditor::Schema::SpatialGDKGenerateSchemaForClasses(Classes, SchemaOutputFolder);

	// THEN
	FString FileContent = LoadSchemaFileForClass(SchemaOutputFolder, CurrentClass);
	TestTrue("Generated Actor schema is valid", ValidateGeneratedSchemaForClass(FileContent, CurrentClass));

	// CLEANUP
	SpatialGDKEditor::Schema::DeleteGeneratedSchemaFiles(SchemaOutputFolder);

	return true;
}

SCHEMA_GENERATOR_TEST(GIVEN_multiple_Actor_classes_WHEN_generated_schema_for_these_classes_THEN_files_with_valid_schema_exist)
{
	// GIVEN
	TSet<UClass*> Classes;
	Classes.Add(ASpatialTypeActor::StaticClass());
	Classes.Add(ANonSpatialTypeActor::StaticClass());
	Classes.Sort([](const UClass& A, const UClass& B)
	{
		return A.GetPathName() < B.GetPathName();
	});

	FString SchemaOutputFolder = FPaths::Combine(FSpatialGDKServicesModule::GetSpatialOSDirectory(), TEXT("Tests/"));
	SpatialGDKEditor::Schema::ResetSchemaGeneratorState();
	ResetID();

	// WHEN
	SpatialGDKEditor::Schema::SpatialGDKGenerateSchemaForClasses(Classes, SchemaOutputFolder);

	// THEN
	bool bValidSchemaExists = true;
	for (const auto& CurrentClass : Classes)
	{
		FString FileContent = LoadSchemaFileForClass(SchemaOutputFolder, CurrentClass);
		if(!ValidateGeneratedSchemaForClass(FileContent, CurrentClass))
		{
			bValidSchemaExists = false;
			break;
		}
	}

	TestTrue("Generated Actor schema is valid", bValidSchemaExists);

	// CLEANUP
	SpatialGDKEditor::Schema::DeleteGeneratedSchemaFiles(SchemaOutputFolder);

	return true;
}

SCHEMA_GENERATOR_TEST(GIVEN_an_Actor_component_class_WHEN_generated_schema_for_this_class_THEN_a_file_with_valid_schema_exists)
{
	// GIVEN
	UClass* CurrentClass = USpatialTypeActorComponent::StaticClass();
	TSet<UClass*> Classes;
	Classes.Add(CurrentClass);

	FString SchemaOutputFolder = FPaths::Combine(FSpatialGDKServicesModule::GetSpatialOSDirectory(), TEXT("Tests/"));
	SpatialGDKEditor::Schema::ResetSchemaGeneratorState();
	ResetID();

	// WHEN
	SpatialGDKEditor::Schema::SpatialGDKGenerateSchemaForClasses(Classes, SchemaOutputFolder);

	// THEN
	FString FileContent = LoadSchemaFileForClass(SchemaOutputFolder, CurrentClass);
	TestTrue("Generated Actor schema is valid", ValidateGeneratedSchemaForClass(FileContent, CurrentClass));

	// CLEANUP
	SpatialGDKEditor::Schema::DeleteGeneratedSchemaFiles(SchemaOutputFolder);

	return true;
}

SCHEMA_GENERATOR_TEST(GIVEN_an_Actor_class_with_an_actor_component_WHEN_generated_schema_for_this_class_THEN_a_file_with_valid_schema_exists)
{
	// GIVEN
	UClass* CurrentClass = ASpatialTypeActorWithActorComponent::StaticClass();
	TSet<UClass*> Classes;
	Classes.Add(CurrentClass);

	FString SchemaOutputFolder = FPaths::Combine(FSpatialGDKServicesModule::GetSpatialOSDirectory(), TEXT("Tests/"));
	SpatialGDKEditor::Schema::ResetSchemaGeneratorState();
	ResetID();

	// WHEN
	SpatialGDKEditor::Schema::SpatialGDKGenerateSchemaForClasses(Classes, SchemaOutputFolder);

	// THEN
	FString FileContent = LoadSchemaFileForClass(SchemaOutputFolder, CurrentClass);
	TestTrue("Generated Actor schema is valid", ValidateGeneratedSchemaForClass(FileContent, CurrentClass));

	// CLEANUP
	SpatialGDKEditor::Schema::DeleteGeneratedSchemaFiles(SchemaOutputFolder);

	return true;
}

SCHEMA_GENERATOR_TEST(GIVEN_an_Actor_class_with_multiple_actor_components_WHEN_generated_schema_for_this_class_THEN_files_with_valid_schema_exist)
{
	// GIVEN
	UClass* CurrentClass = ASpatialTypeActorWithMultipleActorComponents::StaticClass();
	TSet<UClass*> Classes;
	Classes.Add(CurrentClass);

	FString SchemaOutputFolder = FPaths::Combine(FSpatialGDKServicesModule::GetSpatialOSDirectory(), TEXT("Tests/"));
	SpatialGDKEditor::Schema::ResetSchemaGeneratorState();
	ResetID();

	// WHEN
	SpatialGDKEditor::Schema::SpatialGDKGenerateSchemaForClasses(Classes, SchemaOutputFolder);

	// THEN
	FString FileContent = LoadSchemaFileForClass(SchemaOutputFolder, CurrentClass);
	TestTrue("Generated Actor schema is valid", ValidateGeneratedSchemaForClass(FileContent, CurrentClass));

	// CLEANUP
	SpatialGDKEditor::Schema::DeleteGeneratedSchemaFiles(SchemaOutputFolder);

	return true;
}

SCHEMA_GENERATOR_TEST(GIVEN_an_Actor_class_with_multiple_object_components_WHEN_generated_schema_for_this_class_THEN_files_with_valid_schema_exist)
{
	// GIVEN
	UClass* CurrentClass = ASpatialTypeActorWithMultipleObjectComponents::StaticClass();
	TSet<UClass*> Classes;
	Classes.Add(CurrentClass);

	FString SchemaOutputFolder = FPaths::Combine(FSpatialGDKServicesModule::GetSpatialOSDirectory(), TEXT("Tests/"));
	SpatialGDKEditor::Schema::ResetSchemaGeneratorState();
	ResetID();

	// WHEN
	SpatialGDKEditor::Schema::SpatialGDKGenerateSchemaForClasses(Classes, SchemaOutputFolder);

	// THEN
	FString FileContent = LoadSchemaFileForClass(SchemaOutputFolder, CurrentClass);
	TestTrue("Generated Actor schema is valid", ValidateGeneratedSchemaForClass(FileContent, CurrentClass));

	// CLEANUP
	SpatialGDKEditor::Schema::DeleteGeneratedSchemaFiles(SchemaOutputFolder);

	return true;
}

SCHEMA_GENERATOR_TEST(GIVEN_multiple_schema_files_exist_WHEN_deleted_generated_files_THEN_no_schema_files_exist)
{
	// GIVEN
	TSet<UClass*> Classes;
	// TODO(Alex): Don't have that many classes?
	Classes.Add(USchemaGenObjectStub::StaticClass());
	Classes.Add(USpatialTypeObjectStub::StaticClass());
	Classes.Add(UChildOfSpatialTypeObjectStub::StaticClass());
	Classes.Add(UNotSpatialTypeObjectStub::StaticClass());
	Classes.Add(UChildOfNotSpatialTypeObjectStub::StaticClass());
	Classes.Add(UNoSpatialFlagsObjectStub::StaticClass());
	Classes.Add(UChildOfNoSpatialFlagsObjectStub::StaticClass());
	Classes.Add(ASpatialTypeActor::StaticClass());

	FString SchemaOutputFolder = FPaths::Combine(FSpatialGDKServicesModule::GetSpatialOSDirectory(), TEXT("Tests/"));
	SpatialGDKEditor::Schema::ResetSchemaGeneratorState();
	ResetID();
	SpatialGDKEditor::Schema::SpatialGDKGenerateSchemaForClasses(Classes, SchemaOutputFolder);

	// WHEN
	SpatialGDKEditor::Schema::DeleteGeneratedSchemaFiles(SchemaOutputFolder);

	// THEN
	IPlatformFile& PlatformFile = FPlatformFileManager::Get().GetPlatformFile();
	TestFalse("Schema directory does not exist", PlatformFile.DirectoryExists(*SchemaOutputFolder));

	return true;
}

SCHEMA_GENERATOR_TEST(GIVEN_no_schema_files_exist_WHEN_deleted_generated_files_THEN_no_schema_files_exist)
{
	// GIVEN
	FString SchemaOutputFolder = FPaths::Combine(FSpatialGDKServicesModule::GetSpatialOSDirectory(), TEXT("Tests/"));

	// WHEN
	SpatialGDKEditor::Schema::DeleteGeneratedSchemaFiles(SchemaOutputFolder);

	// THEN
	IPlatformFile& PlatformFile = FPlatformFileManager::Get().GetPlatformFile();
	TestFalse("Schema directory does not exist", PlatformFile.DirectoryExists(*SchemaOutputFolder));

	return true;
}

SCHEMA_GENERATOR_TEST(GIVEN_multiple_classes_with_schema_generated_WHEN_schema_database_saved_THEN_schema_database_exists)
{
	// GIVEN
	TSet<UClass*> Classes;
	// TODO(Alex): Don't have that many classes?
	Classes.Add(USchemaGenObjectStub::StaticClass());
	Classes.Add(USpatialTypeObjectStub::StaticClass());
	Classes.Add(UChildOfSpatialTypeObjectStub::StaticClass());
	Classes.Add(UNotSpatialTypeObjectStub::StaticClass());
	Classes.Add(UChildOfNotSpatialTypeObjectStub::StaticClass());
	Classes.Add(UNoSpatialFlagsObjectStub::StaticClass());
	Classes.Add(UChildOfNoSpatialFlagsObjectStub::StaticClass());
	Classes.Add(ASpatialTypeActor::StaticClass());

	const FString SchemaOutputFolder = FPaths::Combine(FSpatialGDKServicesModule::GetSpatialOSDirectory(), TEXT("Tests/"));
	const FString DatabaseOutputFile = TEXT("/Game/Spatial/Tests/SchemaDatabase");
	const FString SchemaDatabaseFileName = TEXT("Spatial/Tests/SchemaDatabase");
	SpatialGDKEditor::Schema::ResetSchemaGeneratorState();
	ResetID();
	SpatialGDKEditor::Schema::SpatialGDKGenerateSchemaForClasses(Classes, SchemaOutputFolder);

	// WHEN
	SpatialGDKEditor::Schema::SaveSchemaDatabase(DatabaseOutputFile);

	// THEN
	const FString SchemaDatabasePackagePath = FPaths::Combine(FPaths::ProjectContentDir(), SchemaDatabaseFileName);
	const FString ExpectedSchemaDatabaseFileName = FPaths::SetExtension(SchemaDatabasePackagePath, FPackageName::GetAssetPackageExtension());
	IPlatformFile& PlatformFile = FPlatformFileManager::Get().GetPlatformFile();
	TestTrue("Generated schema database exists", PlatformFile.FileExists(*ExpectedSchemaDatabaseFileName));

	// CLEANUP
	SpatialGDKEditor::Schema::DeleteGeneratedSchemaFiles(SchemaOutputFolder);
	SpatialGDKEditor::Schema::DeleteSchemaDatabase(SchemaDatabaseFileName );

	return true;
}

SCHEMA_GENERATOR_TEST(GIVEN_a_class_with_schema_generated_WHEN_schema_database_saved_THEN_valid_schema_database_exists)
{
	// GIVEN
	UClass* CurrentClass = ASpatialTypeActor::StaticClass();
	TSet<UClass*> Classes;
	Classes.Add(CurrentClass);

	const FString SchemaOutputFolder = FPaths::Combine(FSpatialGDKServicesModule::GetSpatialOSDirectory(), TEXT("Tests/"));
	const FString DatabaseOutputFile = TEXT("/Game/Spatial/Tests/SchemaDatabase");
	const FString SchemaDatabaseFileName = TEXT("Spatial/Tests/SchemaDatabase");
	SpatialGDKEditor::Schema::ResetSchemaGeneratorState();
	ResetID();
	SpatialGDKEditor::Schema::SpatialGDKGenerateSchemaForClasses(Classes, SchemaOutputFolder);

	// WHEN
	SpatialGDKEditor::Schema::SaveSchemaDatabase(DatabaseOutputFile);

	// THEN
	TestTrue("Generated schema database is valid", false);

	// CLEANUP
	SpatialGDKEditor::Schema::DeleteGeneratedSchemaFiles(SchemaOutputFolder);
	SpatialGDKEditor::Schema::DeleteSchemaDatabase(SchemaDatabaseFileName );

	return true;
}

SCHEMA_GENERATOR_TEST(GIVEN_multiple_classes_with_schema_generated_WHEN_schema_database_saved_THEN_valid_schema_database_exists)
{
	// GIVEN
	TSet<UClass*> Classes;
	// TODO(Alex): Don't have that many classes?
	Classes.Add(USchemaGenObjectStub::StaticClass());
	Classes.Add(USpatialTypeObjectStub::StaticClass());
	Classes.Add(UChildOfSpatialTypeObjectStub::StaticClass());
	Classes.Add(UNotSpatialTypeObjectStub::StaticClass());
	Classes.Add(UChildOfNotSpatialTypeObjectStub::StaticClass());
	Classes.Add(UNoSpatialFlagsObjectStub::StaticClass());
	Classes.Add(UChildOfNoSpatialFlagsObjectStub::StaticClass());
	Classes.Add(ASpatialTypeActor::StaticClass());

	const FString SchemaOutputFolder = FPaths::Combine(FSpatialGDKServicesModule::GetSpatialOSDirectory(), TEXT("Tests/"));
	const FString DatabaseOutputFile = TEXT("/Game/Spatial/Tests/SchemaDatabase");
	const FString SchemaDatabaseFileName = TEXT("Spatial/Tests/SchemaDatabase");
	SpatialGDKEditor::Schema::ResetSchemaGeneratorState();
	ResetID();
	SpatialGDKEditor::Schema::SpatialGDKGenerateSchemaForClasses(Classes, SchemaOutputFolder);

	// WHEN
	SpatialGDKEditor::Schema::SaveSchemaDatabase(DatabaseOutputFile);

	// THEN
	TestTrue("Generated schema database is valid", false);

	// CLEANUP
	SpatialGDKEditor::Schema::DeleteGeneratedSchemaFiles(SchemaOutputFolder);
	SpatialGDKEditor::Schema::DeleteSchemaDatabase(SchemaDatabaseFileName );

	return true;
}

SCHEMA_GENERATOR_TEST(GIVEN_schema_database_exists_WHEN_schema_database_deleted_THEN_no_schema_database_exists)
{
	// GIVEN
	UClass* CurrentClass = ASpatialTypeActor::StaticClass();
	TSet<UClass*> Classes;
	Classes.Add(CurrentClass);

	const FString SchemaOutputFolder = FPaths::Combine(FSpatialGDKServicesModule::GetSpatialOSDirectory(), TEXT("Tests/"));
	const FString DatabaseOutputFile = TEXT("/Game/Spatial/Tests/SchemaDatabase");
	const FString SchemaDatabaseFileName = TEXT("Spatial/Tests/SchemaDatabase");
	SpatialGDKEditor::Schema::ResetSchemaGeneratorState();
	ResetID();
	SpatialGDKEditor::Schema::SpatialGDKGenerateSchemaForClasses(Classes, SchemaOutputFolder);
	SpatialGDKEditor::Schema::SaveSchemaDatabase(DatabaseOutputFile);

	// WHEN
	SpatialGDKEditor::Schema::DeleteSchemaDatabase(SchemaDatabaseFileName );

	// THEN
	const FString SchemaDatabasePackagePath = FPaths::Combine(FPaths::ProjectContentDir(), SchemaDatabaseFileName);
	const FString ExpectedSchemaDatabaseFileName = FPaths::SetExtension(SchemaDatabasePackagePath, FPackageName::GetAssetPackageExtension());
	IPlatformFile& PlatformFile = FPlatformFileManager::Get().GetPlatformFile();
	TestFalse("Generated schema database does not exists", PlatformFile.FileExists(*ExpectedSchemaDatabaseFileName));

	// CLEANUP
	SpatialGDKEditor::Schema::DeleteGeneratedSchemaFiles(SchemaOutputFolder);

	return true;
}

SCHEMA_GENERATOR_TEST(GIVEN_schema_database_exists_WHEN_tried_to_load_THEN_loaded)
{
	// GIVEN
	UClass* CurrentClass = ASpatialTypeActor::StaticClass();
	TSet<UClass*> Classes;
	Classes.Add(CurrentClass);

	const FString SchemaOutputFolder = FPaths::Combine(FSpatialGDKServicesModule::GetSpatialOSDirectory(), TEXT("Tests/"));
	const FString DatabaseOutputFile = TEXT("/Game/Spatial/Tests/SchemaDatabase");
	const FString SchemaDatabaseFileName = TEXT("Spatial/Tests/SchemaDatabase");
	SpatialGDKEditor::Schema::ResetSchemaGeneratorState();
	ResetID();
	SpatialGDKEditor::Schema::SpatialGDKGenerateSchemaForClasses(Classes, SchemaOutputFolder);
	SpatialGDKEditor::Schema::SaveSchemaDatabase(DatabaseOutputFile);

	// WHEN
	const FString SchemaDatabasePackagePath = FPaths::Combine(FPaths::ProjectContentDir(), SchemaDatabaseFileName);
	const FString ExpectedSchemaDatabaseFileName = FPaths::SetExtension(SchemaDatabasePackagePath, FPackageName::GetAssetPackageExtension());
	bool bSuccess = SpatialGDKEditor::Schema::TryLoadExistingSchemaDatabase(ExpectedSchemaDatabaseFileName);

	// THEN
	TestTrue("Schema database loaded", bSuccess);

	// CLEANUP
	SpatialGDKEditor::Schema::DeleteGeneratedSchemaFiles(SchemaOutputFolder);
	SpatialGDKEditor::Schema::DeleteSchemaDatabase(SchemaDatabaseFileName );

	return true;
}

SCHEMA_GENERATOR_TEST(GIVEN_schema_database_does_not_exist_WHEN_tried_to_load_THEN_not_loaded)
{
	// GIVEN
	const FString SchemaOutputFolder = FPaths::Combine(FSpatialGDKServicesModule::GetSpatialOSDirectory(), TEXT("Tests/"));
	const FString SchemaDatabaseFileName = TEXT("Spatial/Tests/SchemaDatabase");
	SpatialGDKEditor::Schema::DeleteGeneratedSchemaFiles(SchemaOutputFolder);
	SpatialGDKEditor::Schema::DeleteSchemaDatabase(SchemaDatabaseFileName );

	// WHEN
	const FString SchemaDatabasePackagePath = FPaths::Combine(FPaths::ProjectContentDir(), SchemaDatabaseFileName);
	const FString ExpectedSchemaDatabaseFileName = FPaths::SetExtension(SchemaDatabasePackagePath, FPackageName::GetAssetPackageExtension());
	bool bSuccess = SpatialGDKEditor::Schema::TryLoadExistingSchemaDatabase(ExpectedSchemaDatabaseFileName);

	// THEN
	TestFalse("Schema database not loaded", bSuccess);

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
