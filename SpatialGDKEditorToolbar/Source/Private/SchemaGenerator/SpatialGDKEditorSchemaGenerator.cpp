// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "SpatialGDKEditorSchemaGenerator.h"

#include "AssetRegistryModule.h"
#include "Engine/LevelScriptActor.h"
#include "GeneralProjectSettings.h"
#include "GenericPlatform/GenericPlatformFile.h"
#include "GenericPlatform/GenericPlatformProcess.h"
#include "Misc/FileHelper.h"
#include "Misc/MonitoredProcess.h"
#include "SchemaGenerator.h"
#include "SharedPointer.h"
#include "TypeStructure.h"
#include "SpatialConstants.h"
#include "SpatialGDKEditorToolbarSettings.h"
#include "Utils/CodeWriter.h"
#include "Utils/ComponentIdGenerator.h"
#include "Utils/DataTypeUtilities.h"
#include "Utils/SchemaDatabase.h"

DEFINE_LOG_CATEGORY(LogSpatialGDKSchemaGenerator);

TArray<UClass*> SchemaGeneratedClasses;
TMap<UClass*, FSchemaData> ClassToSchema;

namespace
{

void OnStatusOutput(FString Message)
{
	UE_LOG(LogSpatialGDKSchemaGenerator, Log, TEXT("%s"), *Message);
}

int GenerateCompleteSchemaFromClass(FString SchemaPath, int ComponentId, UClass* Class)
{
	FString SchemaFilename = UnrealNameToSchemaTypeName(Class->GetName());

	// Parent and static array index start at 0 for checksum calculations.
	TSharedPtr<FUnrealType> TypeInfo = CreateUnrealTypeInfo(Class, 0, 0, false);

	int NumComponents = 0;
	if (Class->IsChildOf<AActor>())
	{
		NumComponents = GenerateActorSchema(ComponentId, Class, TypeInfo, SchemaPath);
	}
	else
	{
		GenerateSubobjectSchema(Class, TypeInfo, SchemaPath + TEXT("Subobjects/"));
	}

	return NumComponents;
}

bool CheckClassNameListValidity(const TArray<UClass*>& Classes)
{
	// Remove all underscores from the class names, check for duplicates.
	for (int i = 0; i < Classes.Num() - 1; ++i)
	{
		const FString& ClassA = Classes[i]->GetName();
		const FString SchemaTypeA = UnrealNameToSchemaTypeName(ClassA);

		for (int j = i + 1; j < Classes.Num(); ++j)
		{
			const FString& ClassB = Classes[j]->GetName();
			const FString SchemaTypeB = UnrealNameToSchemaTypeName(ClassB);

			if (SchemaTypeA.Equals(SchemaTypeB))
			{
				UE_LOG(LogSpatialGDKSchemaGenerator, Error, TEXT("Class name collision after removing underscores: '%s' and '%s' - schema not generated"), *ClassA, *ClassB);
				return false;
			}
		}
	}

	// Ensure class conforms to schema uppercase letter check
	for (const auto& Class : Classes)
	{
		FString ClassName = Class->GetName();
		if (FChar::IsLower(ClassName[0]))
		{
			UE_LOG(LogSpatialGDKSchemaGenerator, Error, TEXT("SpatialType class begins with lowercase letter: %s. Schema not generated"), *ClassName);
			return false;
		}
	}
	return true;
}
}// ::

void GenerateSchemaFromClasses(const TArray<UClass*>& Classes, const FString& CombinedSchemaPath)
{
	Worker_ComponentId ComponentId = SpatialConstants::STARTING_GENERATED_COMPONENT_ID;

	for (const auto& Class : Classes)
	{
		ComponentId += GenerateCompleteSchemaFromClass(CombinedSchemaPath, ComponentId, Class);
	}
}

FString GenerateIntermediateDirectory()
{
	const FString CombinedIntermediatePath = FPaths::Combine(*FPaths::GetPath(FPaths::GetProjectFilePath()), TEXT("Intermediate/Improbable/"), *FGuid::NewGuid().ToString(), TEXT("/"));
	FString AbsoluteCombinedIntermediatePath = FPaths::ConvertRelativePathToFull(CombinedIntermediatePath);
	FPlatformFileManager::Get().GetPlatformFile().CreateDirectoryTree(*AbsoluteCombinedIntermediatePath);

	return AbsoluteCombinedIntermediatePath;
}

void SaveSchemaDatabase()
{
	AsyncTask(ENamedThreads::GameThread, []{
		FString PackagePath = TEXT("/Game/Spatial/SchemaDatabase");
		UPackage *Package = CreatePackage(nullptr, *PackagePath);

		USchemaDatabase* SchemaDatabase = NewObject<USchemaDatabase>(Package, USchemaDatabase::StaticClass(), FName("SchemaDatabase"), EObjectFlags::RF_Public | EObjectFlags::RF_Standalone);
		SchemaDatabase->ClassToSchema = ClassToSchema;

		FAssetRegistryModule::AssetCreated(SchemaDatabase);
		SchemaDatabase->MarkPackageDirty();

		FString FilePath = FString::Printf(TEXT("%s%s"), *PackagePath, *FPackageName::GetAssetPackageExtension());
		bool bSuccess = UPackage::SavePackage(Package, SchemaDatabase, EObjectFlags::RF_Public | EObjectFlags::RF_Standalone, *FPackageName::LongPackageNameToFilename(PackagePath, FPackageName::GetAssetPackageExtension()));

		if (!bSuccess)
		{
			FMessageDialog::Debugf(FText::FromString(FString::Printf(TEXT("Unable to save Schema Database to %s!"), *PackagePath)));
		}
	});
}

TArray<UClass*> GetAllSpatialTypeClasses()
{
	TArray<UClass*> Classes;

	for (TObjectIterator<UClass> It; It; ++It)
	{
		if (It->HasAnySpatialClassFlags(SPATIALCLASS_GenerateTypeBindings) == false)
		{
			continue;
		}

		// Ensure we don't process skeleton or reinitialized classes
		if (It->GetName().StartsWith(TEXT("SKEL_"), ESearchCase::CaseSensitive)
			|| It->GetName().StartsWith(TEXT("REINST_"), ESearchCase::CaseSensitive))
		{
			continue;
		}

		Classes.Add(*It);
	}

	return Classes;
}

TArray<UClass*> GetAllSupportedClasses()
{
	TArray<UClass*> Classes;

	for (TObjectIterator<UClass> ClassIt; ClassIt; ++ClassIt)
	{
		UClass* SupportedClass = nullptr;
		for (TFieldIterator<UProperty> PropertyIt(*ClassIt); PropertyIt; ++PropertyIt)
		{
			if (PropertyIt->HasAnyPropertyFlags(CPF_Net | CPF_Handover))
			{
				SupportedClass = *ClassIt;
				break;
			}
		}

		// No replicated/handover properties found
		if (SupportedClass == nullptr) continue;

		// Doesn't let us save the schema database
		if (SupportedClass->IsChildOf<ALevelScriptActor>()) continue;

		if (SupportedClass->IsChildOf<USceneComponent>()) continue;

		// Ensure we don't process skeleton or reinitialized classes
		if (SupportedClass->GetName().StartsWith(TEXT("SKEL_"), ESearchCase::CaseSensitive)
			|| SupportedClass->GetName().StartsWith(TEXT("REINST_"), ESearchCase::CaseSensitive))
		{
			continue;
		}

		Classes.Add(SupportedClass);
	}

	return Classes;
}

bool SpatialGDKGenerateSchema()
{
	ClassToSchema.Empty();

	const USpatialGDKEditorToolbarSettings* SpatialGDKToolbarSettings = GetDefault<USpatialGDKEditorToolbarSettings>();

	if(SpatialGDKToolbarSettings->bGenerateSchemaForAllSupportedClasses)
	{
		SchemaGeneratedClasses = GetAllSupportedClasses();	
	}
	else
	{
		SchemaGeneratedClasses = GetAllSpatialTypeClasses();
	}

	if (!CheckClassNameListValidity(SchemaGeneratedClasses))
	{
		return false;
	}

	FString SchemaOutputPath = SpatialGDKToolbarSettings->GetGeneratedSchemaOutputFolder();

	UE_LOG(LogSpatialGDKSchemaGenerator, Display, TEXT("Schema path %s"), *SchemaOutputPath);

	// Check schema path is valid.
	if (!FPaths::CollapseRelativeDirectories(SchemaOutputPath))
	{
		UE_LOG(LogSpatialGDKSchemaGenerator, Error, TEXT("Invalid path: '%s'. Schema not generated."), *SchemaOutputPath);
		return false;
	}

	IPlatformFile& PlatformFile = FPlatformFileManager::Get().GetPlatformFile();
	if (PlatformFile.DirectoryExists(*SchemaOutputPath))
	{
		PlatformFile.DeleteDirectoryRecursively(*SchemaOutputPath);
		PlatformFile.CreateDirectory(*SchemaOutputPath);
	}

	check(GetDefault<UGeneralProjectSettings>()->bSpatialNetworking);
	GenerateSchemaFromClasses(SchemaGeneratedClasses, SchemaOutputPath);

	SaveSchemaDatabase();

	return true;
}
