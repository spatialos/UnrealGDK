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
#include "SharedPointer.h"

#include "TypeStructure.h"
#include "SchemaGenerator.h"
#include "SpatialConstants.h"
#include "SpatialGDKEditorToolbarSettings.h"
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
				UE_LOG(LogSpatialGDKSchemaGenerator, Error, TEXT("Replicated property name collision after removing non-alphanumeric characters: '%s' and '%s' - schema not generated"), *ExistingReplicatedProperty->Get()->Property->GetName(), *RepProp.Value->Property->GetName());
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
			UE_LOG(LogSpatialGDKSchemaGenerator, Error, TEXT("Handover data name collision after removing non-alphanumeric characters: '%s' and '%s' - schema not generated"), *ExistingHandoverData->Get()->Property->GetName(), *Prop.Value->Property->GetName());
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
				UE_LOG(LogSpatialGDKSchemaGenerator, Error, TEXT("RPC name collision after removing non-alphanumeric characters: '%s' and '%s' - schema not generated"), *ExistingRPC->Get()->Function->GetName(), *RPC->Function->GetName());
				return false;
			}

			SchemaRPCNames.Add(NextSchemaRPCName, RPC);
		}
	}

	return true;
}

bool ValidateIdentifierNames(/*const TArray<UClass*>& Classes,*/ TArray<TSharedPtr<FUnrealType>>& TypeInfos)
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
				UE_LOG(LogSpatialGDKSchemaGenerator, Error, TEXT("Class name collision after removing non-alphanumeric characters: '%s' and '%s' - schema not generated"), *ClassA, *ClassB);
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
	TSet<UClass*> Classes;

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

	return Classes.Array();
}

TArray<UClass*> GetAllSupportedClasses()
{
	TSet<UClass*> Classes;

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

	return Classes.Array();
}

bool SpatialGDKGenerateSchema()
{
	ClassPathToSchema.Empty();

	const USpatialGDKEditorToolbarSettings* SpatialGDKToolbarSettings = GetDefault<USpatialGDKEditorToolbarSettings>();

	if(SpatialGDKToolbarSettings->bGenerateSchemaForAllSupportedClasses)
	{
		SchemaGeneratedClasses = GetAllSupportedClasses();	
	}
	else
	{
		SchemaGeneratedClasses = GetAllSpatialTypeClasses();
	}

	SchemaGeneratedClasses.Sort();

	// Generate Type Info structs for all classes
	TArray<TSharedPtr<FUnrealType>> TypeInfos;

	for (const auto& Class : SchemaGeneratedClasses)
	{
		// Parent and static array index start at 0 for checksum calculations.
		TypeInfos.Add(CreateUnrealTypeInfo(Class, 0, 0, false));
	}

	if (!ValidateIdentifierNames(/*SchemaGeneratedClasses,*/ TypeInfos))
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
	GenerateSchemaFromClasses(TypeInfos, SchemaOutputPath);

	SaveSchemaDatabase();

	return true;
}
