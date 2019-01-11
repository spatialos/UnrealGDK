// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "SpatialGDKEditorSchemaGenerator.h"

#include "AssetRegistryModule.h"
#include "Components/SceneComponent.h"
#include "Engine/LevelScriptActor.h"
#include "GeneralProjectSettings.h"
#include "GenericPlatform/GenericPlatformFile.h"
#include "GenericPlatform/GenericPlatformProcess.h"
#include "Misc/FileHelper.h"
#include "Misc/MonitoredProcess.h"
#include "Templates/SharedPointer.h"

#include "TypeStructure.h"
#include "SchemaGenerator.h"
#include "SpatialConstants.h"
#include "SpatialGDKEditorSettings.h"
#include "Utils/CodeWriter.h"
#include "Utils/ComponentIdGenerator.h"
#include "Utils/DataTypeUtilities.h"
#include "Utils/SchemaDatabase.h"

DEFINE_LOG_CATEGORY(LogSpatialGDKSchemaGenerator);

TArray<UClass*> SchemaGeneratedClasses;
TMap<FString, FSchemaData> ClassPathToSchema;

namespace
{

void OnStatusOutput(FString Message)
{
	UE_LOG(LogSpatialGDKSchemaGenerator, Log, TEXT("%s"), *Message);
}

int GenerateCompleteSchemaFromClass(FString SchemaPath, int ComponentId, TSharedPtr<FUnrealType> TypeInfo)
{
	UClass* Class = Cast<UClass>(TypeInfo->Type);
	FString SchemaFilename = UnrealNameToSchemaName(Class->GetName());

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

bool CheckIdentifierNameValidity(TSharedPtr<FUnrealType> TypeInfo)
{
	// Check Replicated Data
	FUnrealFlatRepData RepData = GetFlatRepData(TypeInfo);
	for (EReplicatedPropertyGroup Group : GetAllReplicatedPropertyGroups())
	{
		TMap<FString, TSharedPtr<FUnrealProperty>> SchemaReplicatedDataNames;
		for (auto& RepProp : RepData[Group])
		{
			FString NextSchemaReplicatedDataName = SchemaFieldName(RepProp.Value);
			TSharedPtr<FUnrealProperty>* ExistingReplicatedProperty = SchemaReplicatedDataNames.Find(NextSchemaReplicatedDataName);

			if (ExistingReplicatedProperty != nullptr)
			{
				UE_LOG(LogSpatialGDKSchemaGenerator, Error, TEXT("Replicated property name collision after removing non-alphanumeric characters, schema not generated. Name '%s' collides for '%s' and '%s'"),
					*NextSchemaReplicatedDataName, *ExistingReplicatedProperty->Get()->Property->GetPathName(), *RepProp.Value->Property->GetPathName());
				return false;
			}

			SchemaReplicatedDataNames.Add(NextSchemaReplicatedDataName, RepProp.Value);
		}
	}

	// Check Handover data
	FCmdHandlePropertyMap HandoverData = GetFlatHandoverData(TypeInfo);
	TMap<FString, TSharedPtr<FUnrealProperty>> SchemaHandoverDataNames;
	for (auto& Prop : HandoverData)
	{
		FString NextSchemaHandoverDataName = SchemaFieldName(Prop.Value);
		TSharedPtr<FUnrealProperty>* ExistingHandoverData = SchemaHandoverDataNames.Find(NextSchemaHandoverDataName);

		if (ExistingHandoverData != nullptr)
		{
			UE_LOG(LogSpatialGDKSchemaGenerator, Error, TEXT("Handover data name collision after removing non-alphanumeric characters, schema not generated. Name '%s' collides for '%s' and '%s'"),
				*NextSchemaHandoverDataName, *ExistingHandoverData->Get()->Property->GetPathName(), *Prop.Value->Property->GetPathName());
			return false;
		}

		SchemaHandoverDataNames.Add(NextSchemaHandoverDataName, Prop.Value);
	}

	// Check RPC name validity
	FUnrealRPCsByType RPCsByType = GetAllRPCsByType(TypeInfo);
	for (auto Group : GetRPCTypes())
	{
		TMap<FString, TSharedPtr<FUnrealRPC>> SchemaRPCNames;
		for (auto& RPC : RPCsByType[Group])
		{
			FString NextSchemaRPCName = SchemaRPCName(RPC->Function);
			TSharedPtr<FUnrealRPC>* ExistingRPC = SchemaRPCNames.Find(NextSchemaRPCName);

			if (ExistingRPC != nullptr)
			{
				UE_LOG(LogSpatialGDKSchemaGenerator, Error, TEXT("RPC name collision after removing non-alphanumeric characters, schema not generated. Name '%s' collides for '%s' and '%s'"),
					*NextSchemaRPCName, *ExistingRPC->Get()->Function->GetPathName(), *RPC->Function->GetPathName());
				return false;
			}

			SchemaRPCNames.Add(NextSchemaRPCName, RPC);
		}
	}

	return true;
}

bool ValidateIdentifierNames(TArray<TSharedPtr<FUnrealType>>& TypeInfos)
{
	// Remove all underscores from the class names, check for duplicates.
	for (int i = 0; i < TypeInfos.Num() - 1; ++i)
	{
		const FString& ClassA = TypeInfos[i]->Type->GetName();
		const FString SchemaTypeA = UnrealNameToSchemaName(ClassA);

		for (int j = i + 1; j < TypeInfos.Num(); ++j)
		{
			const FString& ClassB = TypeInfos[j]->Type->GetName();
			const FString SchemaTypeB = UnrealNameToSchemaName(ClassB);

			if (SchemaTypeA.Equals(SchemaTypeB))
			{
				UE_LOG(LogSpatialGDKSchemaGenerator, Error, TEXT("Class name collision after removing non-alphanumeric characters, schema not generated. Name '%s' collides for types '%s' and '%s'"),
					*SchemaTypeA, *TypeInfos[i]->Type->GetPathName(), *TypeInfos[j]->Type->GetPathName());
				return false;
			}
		}
	}

	// Check for duplicate names in the generated type info
	for (auto& TypeInfo : TypeInfos)
	{
		if (!CheckIdentifierNameValidity(TypeInfo))
		{
			return false;
		}
	}

	return true;
}
}// ::

void GenerateSchemaFromClasses(const TArray<TSharedPtr<FUnrealType>>& TypeInfos, const FString& CombinedSchemaPath)
{
	Worker_ComponentId ComponentId = SpatialConstants::STARTING_GENERATED_COMPONENT_ID;

	// Generate the actual schema
	for (auto& TypeInfo : TypeInfos)
	{
		ComponentId += GenerateCompleteSchemaFromClass(CombinedSchemaPath, ComponentId, TypeInfo);
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
		SchemaDatabase->ClassPathToSchema = ClassPathToSchema;

		FAssetRegistryModule::AssetCreated(SchemaDatabase);
		SchemaDatabase->MarkPackageDirty();

		// NOTE: UPackage::GetMetaData() has some code where it will auto-create the metadata if it's missing
		// UPackage::SavePackage() calls UPackage::GetMetaData() at some point, and will cause an exception to get thrown
		// if the metadata auto-creation branch needs to be taken. This is the case when generating the schema from the
		// command line, so we just pre-empt it here.
		Package->GetMetaData();

		FString FilePath = FString::Printf(TEXT("%s%s"), *PackagePath, *FPackageName::GetAssetPackageExtension());
		bool bSuccess = UPackage::SavePackage(Package, SchemaDatabase, EObjectFlags::RF_Public | EObjectFlags::RF_Standalone, *FPackageName::LongPackageNameToFilename(PackagePath, FPackageName::GetAssetPackageExtension()));

		if (!bSuccess)
		{
			FString FullPath = FPaths::ConvertRelativePathToFull(FilePath);
			FPaths::MakePlatformFilename(FullPath);
			FMessageDialog::Debugf(FText::FromString(FString::Printf(TEXT("Unable to save Schema Database to '%s'! Please make sure the file is writeable."), *FullPath)));
		}
	});
}

TArray<UClass*> GetAllSupportedClasses()
{
	TSet<UClass*> Classes;

	for (TObjectIterator<UClass> ClassIt; ClassIt; ++ClassIt)
	{
		// User told us to ignore this class
		if (ClassIt->HasAnySpatialClassFlags(SPATIALCLASS_NotSpatialType))
		{
			continue;
		}

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

		// Ensure we don't process skeleton, reinitialized or classes that have since been hot reloaded
		if (SupportedClass->GetName().StartsWith(TEXT("SKEL_"), ESearchCase::CaseSensitive)
			|| SupportedClass->GetName().StartsWith(TEXT("REINST_"), ESearchCase::CaseSensitive)
			|| SupportedClass->GetName().StartsWith(TEXT("TRASHCLASS_"), ESearchCase::CaseSensitive)
			|| SupportedClass->GetName().StartsWith(TEXT("HOTRELOADED_"), ESearchCase::CaseSensitive))
		{
			continue;
		}

		Classes.Add(SupportedClass);
	}

	return Classes.Array();
}

bool SpatialGDKGenerateSchema()
{
	ClassPathToSchema.Empty();

	SchemaGeneratedClasses = GetAllSupportedClasses();

	SchemaGeneratedClasses.Sort();

	// Generate Type Info structs for all classes
	TArray<TSharedPtr<FUnrealType>> TypeInfos;

	for (const auto& Class : SchemaGeneratedClasses)
	{
		// Parent and static array index start at 0 for checksum calculations.
		TypeInfos.Add(CreateUnrealTypeInfo(Class, 0, 0, false));
	}

	if (!ValidateIdentifierNames(TypeInfos))
	{
		return false;
	}

	FString SchemaOutputPath = GetDefault<USpatialGDKEditorSettings>()->GetGeneratedSchemaOutputFolder();

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
		if (!PlatformFile.DeleteDirectoryRecursively(*SchemaOutputPath))
		{
			UE_LOG(LogSpatialGDKSchemaGenerator, Error, TEXT("Could not clean the generated schema directory '%s'! Please make sure the directory and the files inside are writeable."), *SchemaOutputPath);
			return false;
		}
		PlatformFile.CreateDirectory(*SchemaOutputPath);
	}

	check(GetDefault<UGeneralProjectSettings>()->bSpatialNetworking);
	GenerateSchemaFromClasses(TypeInfos, SchemaOutputPath);

	SaveSchemaDatabase();

	return true;
}
