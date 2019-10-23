// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "TestDefinitions.h"

#include "ExpectedGeneratedSchemaFileContents.h"
#include "SchemaGenObjectStub.h"
#include "SpatialGDKEditorSchemaGenerator.h"
#include "SpatialGDKServicesModule.h"
#include "Utils/SchemaDatabase.h"

#include "CoreMinimal.h"
#include "HAL/PlatformFilemanager.h"
#include "Misc/FileHelper.h"

#define LOCTEXT_NAMESPACE "SpatialGDKEDitorSchemaGeneratorTest"

/*
// TODO(Alex): 
+1. GetAllSupportedClasses add if
2. move IsEmpty logic outside eventually
?3. Package to filepath
4. TryLoadExistingSchemaDatabase:
	- Check Readonly schemadatabase
	- Read data from schemadatabase

5. reuse readonly func in DeleteSchemaDatabase
?6. GetStatData should use filepath
+7. add constructor in Stub.cpp (defaultpawn.cpp)
*/

#define SCHEMA_GENERATOR_TEST(TestName) \
	TEST(SpatialGDKEditor, SchemaGenerator, TestName)

namespace
{
const FString gSchemaOutputFolder = FPaths::Combine(FSpatialGDKServicesModule::GetSpatialOSDirectory(), TEXT("Tests/"));
const FString gSchemaDatabaseFileName = TEXT("Spatial/Tests/SchemaDatabase");
const FString gDatabaseOutputFile = TEXT("/Game/Spatial/Tests/SchemaDatabase");

TArray<FString> LoadSchemaFileForClassToStringArray(const FString& SchemaOutputFolder, const UClass* CurrentClass)
{
	FString SchemaFileFolder = TEXT("");

	if (!CurrentClass->IsChildOf<AActor>())
	{
		SchemaFileFolder = TEXT("Subobjects");
	}

	TArray<FString> FileContent;
	FFileHelper::LoadFileToStringArray(FileContent, *FPaths::SetExtension(FPaths::Combine(FPaths::Combine(SchemaOutputFolder, SchemaFileFolder), CurrentClass->GetName()), TEXT(".schema")));

	return FileContent;
}

struct NamesAndIds
{
	TArray<FString> Names;
	TArray<FString> SubobjectNames;
	TArray<int32> Ids;
};

NamesAndIds ParseAvailableNamesAndIdsFromSchemaFile(const FString& SchemaOutputFolder, const UClass* CurrentClass)
{
	const TArray<FString> LoadedSchema = LoadSchemaFileForClassToStringArray(SchemaOutputFolder, CurrentClass);
	NamesAndIds ParsedNamesAndIds;

	for (const auto& SchemaLine : LoadedSchema)
	{
		FRegexPattern IdPattern = TEXT("(\tid = )([0-9]+)(;)");
		FRegexMatcher IdRegMatcher(IdPattern, SchemaLine);

		FRegexPattern NamePattern = TEXT("(component )(.+)( \\{)");
		FRegexMatcher NameRegMatcher(NamePattern, SchemaLine);

		FRegexPattern SubobjectNamePattern = TEXT("(\tUnrealObjectRef )(.+)( = )([0-9]+)(;)");
		FRegexMatcher SubobjectNameRegMatcher(SubobjectNamePattern, SchemaLine);

		if (IdRegMatcher.FindNext())
		{
			FString ParsedId = IdRegMatcher.GetCaptureGroup(2);

			if (ParsedId.IsNumeric())
			{
				ParsedNamesAndIds.Ids.Push(FCString::Atoi(*ParsedId));
			}
		}
		else if (NameRegMatcher.FindNext())
		{
			FString ComponentName = NameRegMatcher.GetCaptureGroup(2);
			ParsedNamesAndIds.Names.Push(ComponentName);
		}
		else if (SubobjectNameRegMatcher.FindNext())
		{
			FString ParsedSubobjectName = SubobjectNameRegMatcher.GetCaptureGroup(2);

			if (!ParsedSubobjectName.IsEmpty() &&
				ParsedSubobjectName.Compare(TEXT("attachmentreplication_attachparent")) != 0 &&
				ParsedSubobjectName.Compare(TEXT("attachmentreplication_attachcomponent")) != 0 &&
				ParsedSubobjectName.Compare(TEXT("owner")) != 0 &&
				ParsedSubobjectName.Compare(TEXT("instigator")) != 0)
			{
				ParsedNamesAndIds.SubobjectNames.Push(ParsedSubobjectName);
			}
		}
	}

	return ParsedNamesAndIds;
}

bool TestEqualDatabaseEntryAndSchemaFile(const UClass* CurrentClass, const FString& SchemaOutputFolder, const USchemaDatabase* SchemaDatabase)
{
	NamesAndIds ParsedNamesAndIds = ParseAvailableNamesAndIdsFromSchemaFile(SchemaOutputFolder, CurrentClass);

	if (CurrentClass->IsChildOf<AActor>())
	{
		const FActorSchemaData* ActorData = SchemaDatabase->ActorClassPathToSchema.Find(CurrentClass->GetPathName());
		if (ActorData == nullptr)
		{
			return false;
		}
		else
		{
			if (ParsedNamesAndIds.Names.Num() != 1)
			{
				return false;
			}

			if (ActorData->GeneratedSchemaName.Compare(ParsedNamesAndIds.Names[0]) != 0)
			{
				return false;
			}

			TArray<FActorSpecificSubobjectSchemaData> SubobjectNames;
			ActorData->SubobjectData.GenerateValueArray(SubobjectNames);
			if (SubobjectNames.Num() != ParsedNamesAndIds.SubobjectNames.Num())
			{
				return false;
			}

			// TODO: Fix it in the next PR
			//for (int i = 0; i < ParsedNamesAndIds.SubobjectNames.Num(); ++i)
			//{
			//	const auto& Predicate = [&SchemaName = ParsedNamesAndIds.SubobjectNames[i]](const FActorSpecificSubobjectSchemaData& Data)
			//	{
			//		return (Data.Name.ToString().Compare(SchemaName) == 0);
			//	};

			//	if (!SubobjectNames.ContainsByPredicate(Predicate))
			//	{
			//		return false;
			//	}
			//}

			for (int i = 0; i < ParsedNamesAndIds.Ids.Num(); ++i)
			{
				if (ActorData->SchemaComponents[i] != ParsedNamesAndIds.Ids[i])
				{
					return false;
				}
			}
		}
	}
	else
	{
		const FSubobjectSchemaData* SubobjectSchemaData = SchemaDatabase->SubobjectClassPathToSchema.Find(CurrentClass->GetPathName());
		if (SubobjectSchemaData == nullptr)
		{
			return false;
		}
		else
		{
			if (ParsedNamesAndIds.Names.Num() != ParsedNamesAndIds.Ids.Num())
			{
				return false;
			}

			for (int i = 0; i < ParsedNamesAndIds.Ids.Num(); ++i)
			{
				if (SubobjectSchemaData->DynamicSubobjectComponents[i].SchemaComponents[0] != ParsedNamesAndIds.Ids[i])
				{
					return false;
				}

				FString ExpectedComponentName = SubobjectSchemaData->GeneratedSchemaName;
				ExpectedComponentName += TEXT("Dynamic");
				ExpectedComponentName.AppendInt(i + 1);
				if (ParsedNamesAndIds.Names[i].Compare(ExpectedComponentName) != 0)
				{
					return false;
				}
			}
		}
	}

	return true;
}

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

TArray<UObject*> AllTestClassesArray =
{
	USchemaGenObjectStub::StaticClass(),
	USpatialTypeObjectStub::StaticClass(),
	UChildOfSpatialTypeObjectStub::StaticClass(),
	UNotSpatialTypeObjectStub::StaticClass(),
	UChildOfNotSpatialTypeObjectStub::StaticClass(),
	UNoSpatialFlagsObjectStub::StaticClass(),
	UChildOfNoSpatialFlagsObjectStub::StaticClass(),
	ASpatialTypeActor::StaticClass(),
	ANonSpatialTypeActor::StaticClass(),
	USpatialTypeActorComponent::StaticClass(),
	ASpatialTypeActorWithActorComponent::StaticClass(),
	ASpatialTypeActorWithMultipleActorComponents::StaticClass(),
	ASpatialTypeActorWithMultipleObjectComponents::StaticClass(),
	ASpatialTypeActorWithSubobject::StaticClass()
};

TSet<UClass*> AllTestClassesSet =
{
	USchemaGenObjectStub::StaticClass(),
	USpatialTypeObjectStub::StaticClass(),
	UChildOfSpatialTypeObjectStub::StaticClass(),
	UNotSpatialTypeObjectStub::StaticClass(),
	UChildOfNotSpatialTypeObjectStub::StaticClass(),
	UNoSpatialFlagsObjectStub::StaticClass(),
	UChildOfNoSpatialFlagsObjectStub::StaticClass(),
	ASpatialTypeActor::StaticClass(),
	ANonSpatialTypeActor::StaticClass(),
	USpatialTypeActorComponent::StaticClass(),
	ASpatialTypeActorWithActorComponent::StaticClass(),
	ASpatialTypeActorWithMultipleActorComponents::StaticClass(),
	ASpatialTypeActorWithMultipleObjectComponents::StaticClass(),
	ASpatialTypeActorWithSubobject::StaticClass()
};

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

class SchemaValidator
{
public:
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

private:
	int GetID()
	{
		return freeId++;
	}

	int freeId = 10000;
};

void DeleteTestFolders()
{
	IPlatformFile& PlatformFile = FPlatformFileManager::Get().GetPlatformFile();
	PlatformFile.DeleteDirectoryRecursively(*FPaths::Combine(FPaths::ProjectContentDir(), TEXT("Spatial/Tests/")));
	PlatformFile.DeleteDirectoryRecursively(*gSchemaOutputFolder);
}

} // anonymous namespace

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
	TestFalse("Class with Not Spatial Type flag is not supported", bIsSupported);
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
	TSet<UClass*> Classes =
	{
		USpatialTypeObjectStub::StaticClass(),
		ASpatialTypeActor::StaticClass()
	};

	SpatialGDKEditor::Schema::ResetSchemaGeneratorState();

	// WHEN
	SpatialGDKEditor::Schema::SpatialGDKGenerateSchemaForClasses(Classes, gSchemaOutputFolder);

	// THEN
	bool bExpectedFilesExist = true;
	for (const auto& CurrentClass : Classes)
	{
		if (LoadSchemaFileForClass(gSchemaOutputFolder, CurrentClass).IsEmpty())
		{
			bExpectedFilesExist = false;
			break;
		}
	}

	TestTrue("All expected schema files have been generated", bExpectedFilesExist);

	// CLEANUP
	DeleteTestFolders();

	return true;
}

SCHEMA_GENERATOR_TEST(GIVEN_an_Actor_class_WHEN_generated_schema_for_this_class_THEN_a_file_with_valid_schema_exists)
{
	// GIVEN
	SchemaValidator Validator;
	UClass* CurrentClass = ASpatialTypeActor::StaticClass();
	TSet<UClass*> Classes = { CurrentClass };

	SpatialGDKEditor::Schema::ResetSchemaGeneratorState();

	// WHEN
	SpatialGDKEditor::Schema::SpatialGDKGenerateSchemaForClasses(Classes, gSchemaOutputFolder);

	// THEN
	FString FileContent = LoadSchemaFileForClass(gSchemaOutputFolder, CurrentClass);
	TestTrue("Generated Actor schema is valid", Validator.ValidateGeneratedSchemaForClass(FileContent, CurrentClass));

	// CLEANUP
	DeleteTestFolders();

	return true;
}

SCHEMA_GENERATOR_TEST(GIVEN_multiple_Actor_classes_WHEN_generated_schema_for_these_classes_THEN_files_with_valid_schema_exist)
{
	// GIVEN
	SchemaValidator Validator;
	// TODO(Alex): should we have more? 
	TSet<UClass*> Classes =
	{
		ASpatialTypeActor::StaticClass(),
		ANonSpatialTypeActor::StaticClass()
	};

	// Classes need to be sorted to have proper ids
	Classes.Sort([](const UClass& A, const UClass& B)
	{
		return A.GetPathName() < B.GetPathName();
	});

	SpatialGDKEditor::Schema::ResetSchemaGeneratorState();

	// WHEN
	SpatialGDKEditor::Schema::SpatialGDKGenerateSchemaForClasses(Classes, gSchemaOutputFolder);

	// THEN
	bool bValidSchemaExists = true;
	for (const auto& CurrentClass : Classes)
	{
		FString FileContent = LoadSchemaFileForClass(gSchemaOutputFolder, CurrentClass);
		if(!Validator.ValidateGeneratedSchemaForClass(FileContent, CurrentClass))
		{
			bValidSchemaExists = false;
			break;
		}
	}

	TestTrue("Generated Actor schema is valid", bValidSchemaExists);

	// CLEANUP
	DeleteTestFolders();

	return true;
}

SCHEMA_GENERATOR_TEST(GIVEN_an_Actor_component_class_WHEN_generated_schema_for_this_class_THEN_a_file_with_valid_schema_exists)
{
	// GIVEN
	SchemaValidator Validator;
	UClass* CurrentClass = USpatialTypeActorComponent::StaticClass();
	TSet<UClass*> Classes = { CurrentClass };

	SpatialGDKEditor::Schema::ResetSchemaGeneratorState();

	// WHEN
	SpatialGDKEditor::Schema::SpatialGDKGenerateSchemaForClasses(Classes, gSchemaOutputFolder);

	// THEN
	FString FileContent = LoadSchemaFileForClass(gSchemaOutputFolder, CurrentClass);
	TestTrue("Generated Actor schema is valid", Validator.ValidateGeneratedSchemaForClass(FileContent, CurrentClass));

	// CLEANUP
	DeleteTestFolders();

	return true;
}

SCHEMA_GENERATOR_TEST(GIVEN_an_Actor_class_with_an_actor_component_WHEN_generated_schema_for_this_class_THEN_a_file_with_valid_schema_exists)
{
	// GIVEN
	SchemaValidator Validator;
	UClass* CurrentClass = ASpatialTypeActorWithActorComponent::StaticClass();
	TSet<UClass*> Classes = { CurrentClass };

	SpatialGDKEditor::Schema::ResetSchemaGeneratorState();

	// WHEN
	SpatialGDKEditor::Schema::SpatialGDKGenerateSchemaForClasses(Classes, gSchemaOutputFolder);

	// THEN
	FString FileContent = LoadSchemaFileForClass(gSchemaOutputFolder, CurrentClass);
	TestTrue("Generated Actor schema is valid", Validator.ValidateGeneratedSchemaForClass(FileContent, CurrentClass));

	// CLEANUP
	DeleteTestFolders();

	return true;
}

SCHEMA_GENERATOR_TEST(GIVEN_an_Actor_class_with_multiple_actor_components_WHEN_generated_schema_for_this_class_THEN_files_with_valid_schema_exist)
{
	// GIVEN
	SchemaValidator Validator;
	UClass* CurrentClass = ASpatialTypeActorWithMultipleActorComponents::StaticClass();
	TSet<UClass*> Classes = { CurrentClass };

	SpatialGDKEditor::Schema::ResetSchemaGeneratorState();

	// WHEN
	SpatialGDKEditor::Schema::SpatialGDKGenerateSchemaForClasses(Classes, gSchemaOutputFolder);

	// THEN
	FString FileContent = LoadSchemaFileForClass(gSchemaOutputFolder, CurrentClass);
	TestTrue("Generated Actor schema is valid", Validator.ValidateGeneratedSchemaForClass(FileContent, CurrentClass));

	// CLEANUP
	DeleteTestFolders();

	return true;
}

SCHEMA_GENERATOR_TEST(GIVEN_an_Actor_class_with_multiple_object_components_WHEN_generated_schema_for_this_class_THEN_files_with_valid_schema_exist)
{
	// GIVEN
	SchemaValidator Validator;
	UClass* CurrentClass = ASpatialTypeActorWithMultipleObjectComponents::StaticClass();
	TSet<UClass*> Classes = { CurrentClass };

	SpatialGDKEditor::Schema::ResetSchemaGeneratorState();

	// WHEN
	SpatialGDKEditor::Schema::SpatialGDKGenerateSchemaForClasses(Classes, gSchemaOutputFolder);

	// THEN
	FString FileContent = LoadSchemaFileForClass(gSchemaOutputFolder, CurrentClass);
	TestTrue("Generated Actor schema is valid", Validator.ValidateGeneratedSchemaForClass(FileContent, CurrentClass));

	// CLEANUP
	DeleteTestFolders();

	return true;
}

SCHEMA_GENERATOR_TEST(GIVEN_multiple_schema_files_exist_WHEN_deleted_generated_files_THEN_no_schema_files_exist)
{
	// GIVEN
	TSet<UClass*> Classes =
	{
		USpatialTypeObjectStub::StaticClass(),
		ASpatialTypeActor::StaticClass()
	};

	SpatialGDKEditor::Schema::ResetSchemaGeneratorState();
	SpatialGDKEditor::Schema::SpatialGDKGenerateSchemaForClasses(Classes, gSchemaOutputFolder);

	// WHEN
	SpatialGDKEditor::Schema::DeleteGeneratedSchemaFiles(gSchemaOutputFolder);

	// THEN
	IPlatformFile& PlatformFile = FPlatformFileManager::Get().GetPlatformFile();
	TestFalse("Schema directory does not exist", PlatformFile.DirectoryExists(*gSchemaOutputFolder));

	return true;
}

SCHEMA_GENERATOR_TEST(GIVEN_no_schema_files_exist_WHEN_deleted_generated_files_THEN_no_schema_files_exist)
{
	// GIVEN

	// WHEN
	SpatialGDKEditor::Schema::DeleteGeneratedSchemaFiles(gSchemaOutputFolder);

	// THEN
	IPlatformFile& PlatformFile = FPlatformFileManager::Get().GetPlatformFile();
	TestFalse("Schema directory does not exist", PlatformFile.DirectoryExists(*gSchemaOutputFolder));

	return true;
}

SCHEMA_GENERATOR_TEST(GIVEN_multiple_classes_with_schema_generated_WHEN_schema_database_saved_THEN_schema_database_exists)
{
	// GIVEN
	TSet<UClass*> Classes =
	{
		USpatialTypeObjectStub::StaticClass(),
		ASpatialTypeActor::StaticClass()
	};

	SpatialGDKEditor::Schema::ResetSchemaGeneratorState();
	SpatialGDKEditor::Schema::SpatialGDKGenerateSchemaForClasses(Classes, gSchemaOutputFolder);

	// WHEN
	SpatialGDKEditor::Schema::SaveSchemaDatabase(gDatabaseOutputFile);

	// THEN
	const FString SchemaDatabasePackagePath = FPaths::Combine(FPaths::ProjectContentDir(), gSchemaDatabaseFileName);
	const FString ExpectedgSchemaDatabaseFileName = FPaths::SetExtension(SchemaDatabasePackagePath, FPackageName::GetAssetPackageExtension());
	IPlatformFile& PlatformFile = FPlatformFileManager::Get().GetPlatformFile();
	TestTrue("Generated schema database exists", PlatformFile.FileExists(*ExpectedgSchemaDatabaseFileName));

	// CLEANUP
	DeleteTestFolders();

	return true;
}

SCHEMA_GENERATOR_TEST(GIVEN_a_class_with_schema_generated_WHEN_schema_database_saved_THEN_valid_schema_database_exists)
{
	// GIVEN
	UClass* CurrentClass = ASpatialTypeActorWithSubobject::StaticClass();
	TSet<UClass*> Classes = { CurrentClass };

	SpatialGDKEditor::Schema::ResetSchemaGeneratorState();
	SpatialGDKEditor::Schema::SpatialGDKGenerateSchemaForClasses(Classes, gSchemaOutputFolder);

	// WHEN
	SpatialGDKEditor::Schema::SaveSchemaDatabase(gDatabaseOutputFile);

	// THEN
	bool bDatabaseIsValid = true;
	FSoftObjectPath SchemaDatabasePath = FSoftObjectPath(FPaths::SetExtension(gDatabaseOutputFile, TEXT(".SchemaDatabase")));
	USchemaDatabase* SchemaDatabase = Cast<USchemaDatabase>(SchemaDatabasePath.TryLoad());
	if (SchemaDatabase == nullptr)
	{
		bDatabaseIsValid = false;
	}
	else
	{
		if (!TestEqualDatabaseEntryAndSchemaFile(CurrentClass, gSchemaOutputFolder, SchemaDatabase))
		{
			bDatabaseIsValid = false;
		}
	}

	TestTrue("Generated schema database is valid", bDatabaseIsValid);

	// CLEANUP
	DeleteTestFolders();

	return true;
}

SCHEMA_GENERATOR_TEST(GIVEN_multiple_classes_with_schema_generated_WHEN_schema_database_saved_THEN_valid_schema_database_exists)
{
	// GIVEN
	TSet<UClass*> Classes = AllTestClassesSet;

	SpatialGDKEditor::Schema::ResetSchemaGeneratorState();
	SpatialGDKEditor::Schema::SpatialGDKGenerateSchemaForClasses(Classes, gSchemaOutputFolder);

	// WHEN
	SpatialGDKEditor::Schema::SaveSchemaDatabase(gDatabaseOutputFile);

	// THEN
	bool bDatabaseIsValid = true;
	FSoftObjectPath SchemaDatabasePath = FSoftObjectPath(FPaths::SetExtension(gDatabaseOutputFile, TEXT(".SchemaDatabase")));
	USchemaDatabase* SchemaDatabase = Cast<USchemaDatabase>(SchemaDatabasePath.TryLoad());
	if (SchemaDatabase == nullptr)
	{
		bDatabaseIsValid = false;
	}
	else
	{
		for (const auto& CurrentClass : Classes)
		{
			if (!TestEqualDatabaseEntryAndSchemaFile(CurrentClass, gSchemaOutputFolder, SchemaDatabase))
			{
				bDatabaseIsValid = false;
				break;
			}
		}
	}

	TestTrue("Generated schema database is valid", bDatabaseIsValid);

	// CLEANUP
	DeleteTestFolders();

	return true;
}

SCHEMA_GENERATOR_TEST(GIVEN_schema_database_exists_WHEN_schema_database_deleted_THEN_no_schema_database_exists)
{
	// GIVEN
	UClass* CurrentClass = ASpatialTypeActor::StaticClass();
	TSet<UClass*> Classes = { CurrentClass };

	SpatialGDKEditor::Schema::ResetSchemaGeneratorState();
	SpatialGDKEditor::Schema::SpatialGDKGenerateSchemaForClasses(Classes, gSchemaOutputFolder);
	SpatialGDKEditor::Schema::SaveSchemaDatabase(gDatabaseOutputFile);

	// WHEN
	SpatialGDKEditor::Schema::DeleteSchemaDatabase(gSchemaDatabaseFileName );

	// THEN
	const FString SchemaDatabasePackagePath = FPaths::Combine(FPaths::ProjectContentDir(), gSchemaDatabaseFileName);
	const FString ExpectedgSchemaDatabaseFileName = FPaths::SetExtension(SchemaDatabasePackagePath, FPackageName::GetAssetPackageExtension());
	IPlatformFile& PlatformFile = FPlatformFileManager::Get().GetPlatformFile();
	TestFalse("Generated schema database does not exists", PlatformFile.FileExists(*ExpectedgSchemaDatabaseFileName));

	// CLEANUP
	DeleteTestFolders();

	return true;
}

SCHEMA_GENERATOR_TEST(GIVEN_schema_database_exists_WHEN_tried_to_load_THEN_loaded)
{
	// GIVEN
	UClass* CurrentClass = ASpatialTypeActor::StaticClass();
	TSet<UClass*> Classes = { CurrentClass };

	SpatialGDKEditor::Schema::ResetSchemaGeneratorState();
	SpatialGDKEditor::Schema::SpatialGDKGenerateSchemaForClasses(Classes, gSchemaOutputFolder);
	SpatialGDKEditor::Schema::SaveSchemaDatabase(gDatabaseOutputFile);

	// WHEN
	bool bSuccess = SpatialGDKEditor::Schema::LoadGeneratorStateFromSchemaDatabase(gSchemaDatabaseFileName);

	// THEN
	TestTrue("Schema database loaded", bSuccess);

	// CLEANUP
	DeleteTestFolders();

	return true;
}

SCHEMA_GENERATOR_TEST(GIVEN_schema_database_does_not_exist_WHEN_tried_to_load_THEN_not_loaded)
{
	// GIVEN
	AddExpectedError(TEXT("Attempted to delete schema database"), EAutomationExpectedErrorFlags::Contains, 1);
	SpatialGDKEditor::Schema::DeleteSchemaDatabase(gSchemaDatabaseFileName );

	// WHEN
	const FString SchemaDatabasePackagePath = FPaths::Combine(FPaths::ProjectContentDir(), gSchemaDatabaseFileName);
	const FString ExpectedgSchemaDatabaseFileName = FPaths::SetExtension(SchemaDatabasePackagePath, FPackageName::GetAssetPackageExtension());
	bool bSuccess = SpatialGDKEditor::Schema::LoadGeneratorStateFromSchemaDatabase(gSchemaDatabaseFileName);

	// THEN
	TestFalse("Schema database not loaded", bSuccess);

	return true;
}

SCHEMA_GENERATOR_TEST(GIVEN_source_and_destination_of_well_known_schema_files_WHEN_copied_THEN_valid_files_exist)
{
	// GIVEN
	FString GDKSchemaCopyDir = FPaths::Combine(FSpatialGDKServicesModule::GetSpatialOSDirectory(), TEXT("/Tests/schema/unreal/gdk"));
	FString CoreSDKSchemaCopyDir = FPaths::Combine(FSpatialGDKServicesModule::GetSpatialOSDirectory(), TEXT("/Tests/build/dependencies/schema/standard_library"));
	TArray<FString> GDKSchemaFilePaths =
	{
		"authority_intent.schema",
		"core_types.schema",
		"debug_metrics.schema",
		"global_state_manager.schema",
		"heartbeat.schema",
		"not_streamed.schema",
		"relevant.schema",
		"rpc_components.schema",
		"singleton.schema",
		"spawndata.schema",
		"spawner.schema",
		"tombstone.schema",
		"unreal_metadata.schema",
		"virtual_worker_translation.schema"
	};
	TArray<FString> CoreSDKFilePaths =
	{
		"improbable\\restricted\\system_components.schema",
		"improbable\\standard_library.schema"
	};

	// WHEN
	SpatialGDKEditor::Schema::CopyWellKnownSchemaFiles(GDKSchemaCopyDir, CoreSDKSchemaCopyDir);

	// THEN
	bool bExpectedFilesCopied = true;

	IPlatformFile& PlatformFile = FPlatformFileManager::Get().GetPlatformFile();
	for(const auto& FilePath : GDKSchemaFilePaths)
	{
		if (!PlatformFile.FileExists(*FPaths::Combine(GDKSchemaCopyDir, FilePath)))
		{
			bExpectedFilesCopied = false;
			break;
		}
	}

	for(const auto& FilePath : CoreSDKFilePaths)
	{
		if (!PlatformFile.FileExists(*FPaths::Combine(CoreSDKSchemaCopyDir, FilePath)))
		{
			bExpectedFilesCopied = false;
			break;
		}
	}

	TestTrue("Expected files have been copied", bExpectedFilesCopied);

	// CLEANUP
	DeleteTestFolders();

	return true;
}

SCHEMA_GENERATOR_TEST(GIVEN_multiple_classes_WHEN_getting_all_supported_classes_THEN_all_unsupported_classes_are_filtered)
{
	// GIVEN
	TArray<UObject*> Classes = AllTestClassesArray;

	// WHEN
	TSet<UClass*> FilteredClasses = SpatialGDKEditor::Schema::GetAllSupportedClasses(Classes);

	// THEN
	TSet<UClass*> ExpectedClasses =
	{
		USpatialTypeObjectStub::StaticClass(),
		UChildOfSpatialTypeObjectStub::StaticClass(),
		ASpatialTypeActor::StaticClass(),
		ANonSpatialTypeActor::StaticClass(),
		USpatialTypeActorComponent::StaticClass(),
		ASpatialTypeActorWithActorComponent::StaticClass(),
		ASpatialTypeActorWithMultipleActorComponents::StaticClass(),
		ASpatialTypeActorWithMultipleObjectComponents::StaticClass(),
		ASpatialTypeActorWithSubobject::StaticClass()
	};

	bool bClassesFilteredCorrectly = true;
	if (FilteredClasses.Num() == ExpectedClasses.Num())
	{
		for (const auto& ExpectedClass : ExpectedClasses)
		{
			if (!FilteredClasses.Contains(ExpectedClass))
			{
				bClassesFilteredCorrectly = false;
				break;
			}
		}
	}
	else
	{
		bClassesFilteredCorrectly = false;
	}

	TestTrue("Supported classes have been filtered correctly", bClassesFilteredCorrectly);

	return true;
}
