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

#define SCHEMA_GENERATOR_TEST(TestName) \
	GDK_TEST(SpatialGDKEditor, SchemaGenerator, TestName)

namespace
{
const FString SchemaOutputFolder = FPaths::Combine(FSpatialGDKServicesModule::GetSpatialOSDirectory(), TEXT("Tests/"));
const FString SchemaDatabaseFileName = TEXT("Spatial/Tests/SchemaDatabase");
const FString DatabaseOutputFile = TEXT("/Game/Spatial/Tests/SchemaDatabase");

TArray<FString> LoadSchemaFileForClassToStringArray(const FString& InSchemaOutputFolder, const UClass* CurrentClass)
{
	FString SchemaFileFolder = TEXT("");

	if (!CurrentClass->IsChildOf<AActor>())
	{
		SchemaFileFolder = TEXT("Subobjects");
	}

	TArray<FString> FileContent;
	FFileHelper::LoadFileToStringArray(FileContent, *FPaths::SetExtension(FPaths::Combine(FPaths::Combine(InSchemaOutputFolder, SchemaFileFolder), CurrentClass->GetName()), TEXT(".schema")));

	return FileContent;
}

struct ComponentNamesAndIds
{
	TArray<FString> Names;
	TArray<FString> SubobjectNames;
	TArray<int32> Ids;
};

ComponentNamesAndIds ParseAvailableNamesAndIdsFromSchemaFile(const TArray<FString>& LoadedSchema)
{
	ComponentNamesAndIds ParsedNamesAndIds;

	for (const auto& SchemaLine : LoadedSchema)
	{
		FRegexPattern IdPattern = TEXT("(\tid = )([0-9]+)(;)");
		FRegexMatcher IdRegMatcher(IdPattern, SchemaLine);

		FRegexPattern NamePattern = TEXT("(^component )(.+)( \\{)");
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

bool TestEqualDatabaseEntryAndSchemaFile(const UClass* CurrentClass, const FString& InSchemaOutputFolder, const USchemaDatabase* SchemaDatabase)
{
	const TArray<FString> LoadedSchema = LoadSchemaFileForClassToStringArray(InSchemaOutputFolder, CurrentClass);
	ComponentNamesAndIds ParsedNamesAndIds = ParseAvailableNamesAndIdsFromSchemaFile(LoadedSchema);

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

			// TODO: UNR-2298 - Uncomment and fix it
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
				if (SubobjectSchemaData->DynamicSubobjectComponents[i].SchemaComponents[SCHEMA_Data] != ParsedNamesAndIds.Ids[i])
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

FString LoadSchemaFileForClass(const FString& InSchemaOutputFolder, const UClass* CurrentClass)
{
	FString SchemaFileFolder = TEXT("");

	if (!CurrentClass->IsChildOf<AActor>())
	{
		SchemaFileFolder = TEXT("Subobjects");
	}

	FString FileContent;
	FFileHelper::LoadFileToString(FileContent, *FPaths::SetExtension(FPaths::Combine(FPaths::Combine(InSchemaOutputFolder, SchemaFileFolder), CurrentClass->GetName()), TEXT(".schema")));

	return FileContent;
}

const TArray<UObject*>& AllTestClassesArray()
{
	static TArray<UObject*> TestClassesArray = {
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
	return TestClassesArray;
};

const TSet<UClass*>& AllTestClassesSet()
{
	static TSet<UClass*> TestClassesSet = {
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
	return TestClassesSet;
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
			ExpectedContent.ReplaceInline(TEXT("{{id}}"), *FString::FromInt(GetNextFreeId()));
			return (FileContent.Compare(ExpectedContent) == 0);
		}
		else
		{
			return false;
		}
	}

private:
	int GetNextFreeId()
	{
		return FreeId++;
	}

	int FreeId = 10000;
};

class SchemaTestFixture
{
public:
	SchemaTestFixture()
	{
		SpatialGDKEditor::Schema::ResetSchemaGeneratorState();
		EnableSpatialNetworking();
	}
	~SchemaTestFixture()
	{
		DeleteTestFolders();
		ResetSpatialNetworking();
	}

private:

	void DeleteTestFolders()
	{
		IPlatformFile& PlatformFile = FPlatformFileManager::Get().GetPlatformFile();
		PlatformFile.DeleteDirectoryRecursively(*FPaths::Combine(FPaths::ProjectContentDir(), TEXT("Spatial/Tests/")));
		PlatformFile.DeleteDirectoryRecursively(*SchemaOutputFolder);
	}

	void EnableSpatialNetworking()
	{
		UGeneralProjectSettings* GeneralProjectSettings = GetMutableDefault<UGeneralProjectSettings>();
		bCachedSpatialNetworking = GeneralProjectSettings->UsesSpatialNetworking();
		GeneralProjectSettings->SetUsesSpatialNetworking(true);
	}

	void ResetSpatialNetworking()
	{
		UGeneralProjectSettings* GeneralProjectSettings = GetMutableDefault<UGeneralProjectSettings>();
		GetMutableDefault<UGeneralProjectSettings>()->SetUsesSpatialNetworking(bCachedSpatialNetworking);
		bCachedSpatialNetworking = true;
	}

	bool bCachedSpatialNetworking = true;
};

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
	SchemaTestFixture Fixture;

	// GIVEN
	TSet<UClass*> Classes =
	{
		USpatialTypeObjectStub::StaticClass(),
		ASpatialTypeActor::StaticClass()
	};

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

	return true;
}

SCHEMA_GENERATOR_TEST(GIVEN_an_Actor_class_WHEN_generated_schema_for_this_class_THEN_a_file_with_expected_schema_exists)
{
	SchemaTestFixture Fixture;

	// GIVEN
	SchemaValidator Validator;
	UClass* CurrentClass = ASpatialTypeActor::StaticClass();
	TSet<UClass*> Classes = { CurrentClass };

	// WHEN
	SpatialGDKEditor::Schema::SpatialGDKGenerateSchemaForClasses(Classes, SchemaOutputFolder);

	// THEN
	FString FileContent = LoadSchemaFileForClass(SchemaOutputFolder, CurrentClass);
	TestTrue("Generated Actor schema matches the expected schema", Validator.ValidateGeneratedSchemaForClass(FileContent, CurrentClass));

	return true;
}

SCHEMA_GENERATOR_TEST(GIVEN_multiple_Actor_classes_WHEN_generated_schema_for_these_classes_THEN_files_with_expected_schema_exist)
{
	SchemaTestFixture Fixture;

	// GIVEN
	SchemaValidator Validator;
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


	// WHEN
	SpatialGDKEditor::Schema::SpatialGDKGenerateSchemaForClasses(Classes, SchemaOutputFolder);

	// THEN
	bool bGeneratedSchemaMatchesExpected = true;
	for (const auto& CurrentClass : Classes)
	{
		FString FileContent = LoadSchemaFileForClass(SchemaOutputFolder, CurrentClass);
		if(!Validator.ValidateGeneratedSchemaForClass(FileContent, CurrentClass))
		{
			bGeneratedSchemaMatchesExpected = false;
			break;
		}
	}

	TestTrue("Generated Actor schema matches the expected schema", bGeneratedSchemaMatchesExpected);

	return true;
}

SCHEMA_GENERATOR_TEST(GIVEN_an_Actor_component_class_WHEN_generated_schema_for_this_class_THEN_a_file_with_expected_schema_exists)
{
	SchemaTestFixture Fixture;

	// GIVEN
	SchemaValidator Validator;
	UClass* CurrentClass = USpatialTypeActorComponent::StaticClass();
	TSet<UClass*> Classes = { CurrentClass };

	
	// WHEN
	SpatialGDKEditor::Schema::SpatialGDKGenerateSchemaForClasses(Classes, SchemaOutputFolder);

	// THEN
	FString FileContent = LoadSchemaFileForClass(SchemaOutputFolder, CurrentClass);
	TestTrue("Generated Actor schema matches the expected schema", Validator.ValidateGeneratedSchemaForClass(FileContent, CurrentClass));

	return true;
}

SCHEMA_GENERATOR_TEST(GIVEN_an_Actor_class_with_an_actor_component_WHEN_generated_schema_for_this_class_THEN_a_file_with_expected_schema_exists)
{
	SchemaTestFixture Fixture;

	// GIVEN
	SchemaValidator Validator;
	UClass* CurrentClass = ASpatialTypeActorWithActorComponent::StaticClass();
	TSet<UClass*> Classes = { CurrentClass };

	// WHEN
	SpatialGDKEditor::Schema::SpatialGDKGenerateSchemaForClasses(Classes, SchemaOutputFolder);

	// THEN
	FString FileContent = LoadSchemaFileForClass(SchemaOutputFolder, CurrentClass);
	TestTrue("Generated Actor schema matches the expected schema", Validator.ValidateGeneratedSchemaForClass(FileContent, CurrentClass));

	return true;
}

SCHEMA_GENERATOR_TEST(GIVEN_an_Actor_class_with_multiple_actor_components_WHEN_generated_schema_for_this_class_THEN_files_with_expected_schema_exist)
{
	SchemaTestFixture Fixture;

	// GIVEN
	SchemaValidator Validator;
	UClass* CurrentClass = ASpatialTypeActorWithMultipleActorComponents::StaticClass();
	TSet<UClass*> Classes = { CurrentClass };

	// WHEN
	SpatialGDKEditor::Schema::SpatialGDKGenerateSchemaForClasses(Classes, SchemaOutputFolder);

	// THEN
	FString FileContent = LoadSchemaFileForClass(SchemaOutputFolder, CurrentClass);
	TestTrue("Generated Actor schema matches the expected schema", Validator.ValidateGeneratedSchemaForClass(FileContent, CurrentClass));

	return true;
}

SCHEMA_GENERATOR_TEST(GIVEN_an_Actor_class_with_multiple_object_components_WHEN_generated_schema_for_this_class_THEN_files_with_expected_schema_exist)
{
	SchemaTestFixture Fixture;

	// GIVEN
	SchemaValidator Validator;
	UClass* CurrentClass = ASpatialTypeActorWithMultipleObjectComponents::StaticClass();
	TSet<UClass*> Classes = { CurrentClass };

	// WHEN
	SpatialGDKEditor::Schema::SpatialGDKGenerateSchemaForClasses(Classes, SchemaOutputFolder);

	// THEN
	FString FileContent = LoadSchemaFileForClass(SchemaOutputFolder, CurrentClass);
	TestTrue("Generated Actor schema matches the expected schema", Validator.ValidateGeneratedSchemaForClass(FileContent, CurrentClass));

	return true;
}

SCHEMA_GENERATOR_TEST(GIVEN_multiple_schema_files_exist_WHEN_deleted_generated_files_THEN_no_schema_files_exist)
{
	SchemaTestFixture Fixture;

	// GIVEN
	TSet<UClass*> Classes =
	{
		USpatialTypeObjectStub::StaticClass(),
		ASpatialTypeActor::StaticClass()
	};

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

	// WHEN
	SpatialGDKEditor::Schema::DeleteGeneratedSchemaFiles(SchemaOutputFolder);

	// THEN
	IPlatformFile& PlatformFile = FPlatformFileManager::Get().GetPlatformFile();
	TestFalse("Schema directory does not exist", PlatformFile.DirectoryExists(*SchemaOutputFolder));

	return true;
}

SCHEMA_GENERATOR_TEST(GIVEN_multiple_classes_with_schema_generated_WHEN_schema_database_saved_THEN_schema_database_exists)
{
	SchemaTestFixture Fixture;

	// GIVEN
	TSet<UClass*> Classes =
	{
		USpatialTypeObjectStub::StaticClass(),
		ASpatialTypeActor::StaticClass()
	};

	SpatialGDKEditor::Schema::SpatialGDKGenerateSchemaForClasses(Classes, SchemaOutputFolder);

	// WHEN
	SpatialGDKEditor::Schema::SaveSchemaDatabase(DatabaseOutputFile);

	// THEN
	const FString SchemaDatabasePackagePath = FPaths::Combine(FPaths::ProjectContentDir(), SchemaDatabaseFileName);
	const FString ExpectedSchemaDatabaseFileName = FPaths::SetExtension(SchemaDatabasePackagePath, FPackageName::GetAssetPackageExtension());
	IPlatformFile& PlatformFile = FPlatformFileManager::Get().GetPlatformFile();
	TestTrue("Generated schema database exists", PlatformFile.FileExists(*ExpectedSchemaDatabaseFileName));

	return true;
}

SCHEMA_GENERATOR_TEST(GIVEN_a_class_with_schema_generated_WHEN_schema_database_saved_THEN_expected_schema_database_exists)
{
	SchemaTestFixture Fixture;

	// GIVEN
	UClass* CurrentClass = ASpatialTypeActorWithSubobject::StaticClass();
	TSet<UClass*> Classes = { CurrentClass };

	SpatialGDKEditor::Schema::SpatialGDKGenerateSchemaForClasses(Classes, SchemaOutputFolder);

	// WHEN
	SpatialGDKEditor::Schema::SaveSchemaDatabase(DatabaseOutputFile);

	// THEN
	bool bDatabaseMatchesExpected = true;
	FSoftObjectPath SchemaDatabasePath = FSoftObjectPath(FPaths::SetExtension(DatabaseOutputFile, TEXT(".SchemaDatabase")));
	USchemaDatabase* SchemaDatabase = Cast<USchemaDatabase>(SchemaDatabasePath.TryLoad());
	if (SchemaDatabase == nullptr)
	{
		bDatabaseMatchesExpected = false;
	}
	else
	{
		if (!TestEqualDatabaseEntryAndSchemaFile(CurrentClass, SchemaOutputFolder, SchemaDatabase))
		{
			bDatabaseMatchesExpected = false;
		}
	}

	TestTrue("Generated schema database matches the expected database", bDatabaseMatchesExpected);

	return true;
}

SCHEMA_GENERATOR_TEST(GIVEN_multiple_classes_with_schema_generated_WHEN_schema_database_saved_THEN_expected_schema_database_exists)
{
	SchemaTestFixture Fixture;

	// GIVEN
	const TSet<UClass*>& Classes = AllTestClassesSet();

	SpatialGDKEditor::Schema::SpatialGDKGenerateSchemaForClasses(Classes, SchemaOutputFolder);

	// WHEN
	SpatialGDKEditor::Schema::SaveSchemaDatabase(DatabaseOutputFile);

	// THEN
	bool bDatabaseMatchesExpected = true;
	FSoftObjectPath SchemaDatabasePath = FSoftObjectPath(FPaths::SetExtension(DatabaseOutputFile, TEXT(".SchemaDatabase")));
	USchemaDatabase* SchemaDatabase = Cast<USchemaDatabase>(SchemaDatabasePath.TryLoad());
	if (SchemaDatabase == nullptr)
	{
		bDatabaseMatchesExpected = false;
	}
	else
	{
		for (const auto& CurrentClass : Classes)
		{
			if (!TestEqualDatabaseEntryAndSchemaFile(CurrentClass, SchemaOutputFolder, SchemaDatabase))
			{
				bDatabaseMatchesExpected = false;
				break;
			}
		}
	}

	TestTrue("Generated schema database matches the expected database", bDatabaseMatchesExpected);

	return true;
}

SCHEMA_GENERATOR_TEST(GIVEN_schema_database_exists_WHEN_schema_database_deleted_THEN_no_schema_database_exists)
{
	SchemaTestFixture Fixture;
	
	// GIVEN
	UClass* CurrentClass = ASpatialTypeActor::StaticClass();
	TSet<UClass*> Classes = { CurrentClass };

	SpatialGDKEditor::Schema::SpatialGDKGenerateSchemaForClasses(Classes, SchemaOutputFolder);
	SpatialGDKEditor::Schema::SaveSchemaDatabase(DatabaseOutputFile);

	IPlatformFile& PlatformFile = FPlatformFileManager::Get().GetPlatformFile();
	const FString SchemaDatabasePackagePath = FPaths::Combine(FPaths::ProjectContentDir(), SchemaDatabaseFileName);
	const FString ExpectedSchemaDatabaseFileName = FPaths::SetExtension(SchemaDatabasePackagePath, FPackageName::GetAssetPackageExtension());
	bool bFileCreated = PlatformFile.FileExists(*ExpectedSchemaDatabaseFileName);

	// WHEN
	SpatialGDKEditor::Schema::DeleteSchemaDatabase(SchemaDatabaseFileName );

	// THEN
	bool bResult = bFileCreated && !PlatformFile.FileExists(*ExpectedSchemaDatabaseFileName);
	TestTrue("Generated schema existed and is now deleted", bResult);

	return true;
}

SCHEMA_GENERATOR_TEST(GIVEN_schema_database_exists_WHEN_tried_to_load_THEN_loaded)
{
	SchemaTestFixture Fixture;

	// GIVEN
	UClass* CurrentClass = ASpatialTypeActor::StaticClass();
	TSet<UClass*> Classes = { CurrentClass };

	SpatialGDKEditor::Schema::SpatialGDKGenerateSchemaForClasses(Classes, SchemaOutputFolder);
	SpatialGDKEditor::Schema::SaveSchemaDatabase(DatabaseOutputFile);

	// WHEN
	bool bSuccess = SpatialGDKEditor::Schema::LoadGeneratorStateFromSchemaDatabase(SchemaDatabaseFileName);

	// THEN
	TestTrue("Schema database loaded", bSuccess);

	return true;
}

SCHEMA_GENERATOR_TEST(GIVEN_schema_database_does_not_exist_WHEN_tried_to_load_THEN_not_loaded)
{
	SchemaTestFixture Fixture;

	// GIVEN
	SpatialGDKEditor::Schema::DeleteSchemaDatabase(SchemaDatabaseFileName );

	// WHEN
	const FString SchemaDatabasePackagePath = FPaths::Combine(FPaths::ProjectContentDir(), SchemaDatabaseFileName);
	const FString ExpectedSchemaDatabaseFileName = FPaths::SetExtension(SchemaDatabasePackagePath, FPackageName::GetAssetPackageExtension());
	bool bSuccess = SpatialGDKEditor::Schema::LoadGeneratorStateFromSchemaDatabase(SchemaDatabaseFileName);

	// THEN
	TestFalse("Schema database not loaded", bSuccess);

	return true;
}

SCHEMA_GENERATOR_TEST(GIVEN_source_and_destination_of_well_known_schema_files_WHEN_copied_THEN_expected_files_exist)
{
	SchemaTestFixture Fixture;

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

	TArray<FString> FoundSchemaFiles;
	PlatformFile.FindFilesRecursively(FoundSchemaFiles, *GDKSchemaCopyDir, TEXT(""));
	if (FoundSchemaFiles.Num() != GDKSchemaFilePaths.Num())
	{
		bExpectedFilesCopied = false;
	}
	for(const auto& FilePath : GDKSchemaFilePaths)
	{
		if (!PlatformFile.FileExists(*FPaths::Combine(GDKSchemaCopyDir, FilePath)))
		{
			bExpectedFilesCopied = false;
			break;
		}
	}

	TArray<FString> FoundCoreSDKFiles;
	PlatformFile.FindFilesRecursively(FoundCoreSDKFiles, *CoreSDKSchemaCopyDir, TEXT(""));
	if (FoundCoreSDKFiles.Num() != CoreSDKFilePaths.Num())
	{
		bExpectedFilesCopied = false;
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

	return true;
}

SCHEMA_GENERATOR_TEST(GIVEN_multiple_classes_WHEN_getting_all_supported_classes_THEN_all_unsupported_classes_are_filtered)
{
	SchemaTestFixture Fixture;
	
	// GIVEN
	const TArray<UObject*>& Classes = AllTestClassesArray();

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

SCHEMA_GENERATOR_TEST(GIVEN_3_level_names_WHEN_generating_schema_for_sublevels_THEN_generated_schema_contains_3_components_with_unique_names)
{
	SchemaTestFixture Fixture;
	
	// GIVEN
	TMultiMap<FName, FName> LevelNamesToPaths;
	LevelNamesToPaths.Add(TEXT("TestLevel0"), TEXT("/Game/Maps/FirstTestLevel0"));
	LevelNamesToPaths.Add(TEXT("TestLevel0"), TEXT("/Game/Maps/SecondTestLevel0"));
	LevelNamesToPaths.Add(TEXT("TestLevel01"), TEXT("/Game/Maps/TestLevel01"));

	// WHEN
	SpatialGDKEditor::Schema::GenerateSchemaForSublevels(SchemaOutputFolder, LevelNamesToPaths);

	// THEN
	TArray<FString> LoadedSchema;
	FFileHelper::LoadFileToStringArray(LoadedSchema, *FPaths::Combine(SchemaOutputFolder, TEXT("Sublevels/sublevels.schema")));
	ComponentNamesAndIds ParsedNamesAndIds = ParseAvailableNamesAndIdsFromSchemaFile(LoadedSchema);

	bool bHasDuplicateNames = false;
	for (int i = 0; i < ParsedNamesAndIds.Names.Num() - 1; ++i)
	{
		for (int j = i + 1; j < ParsedNamesAndIds.Names.Num(); ++j)
		{
			if (ParsedNamesAndIds.Names[i].Compare(ParsedNamesAndIds.Names[j]) == 0)
			{
				bHasDuplicateNames = true;
				break;
			}
		}
	}

	TestFalse("No duplicate component names generated for equal sublevel map names", bHasDuplicateNames);

	return true;
}
