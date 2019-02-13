// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "SpatialGDKEditorSchemaGenerator.h"

#include "AssetRegistryModule.h"
#include "Async/Async.h"
#include "Components/SceneComponent.h"
#include "Engine/LevelScriptActor.h"
#include "GeneralProjectSettings.h"
#include "GenericPlatform/GenericPlatformFile.h"
#include "GenericPlatform/GenericPlatformProcess.h"
#include "HAL/PlatformFilemanager.h"
#include "Misc/FileHelper.h"
#include "Misc/MessageDialog.h"
#include "Misc/MonitoredProcess.h"
#include "Templates/SharedPointer.h"
#include "UObject/UObjectIterator.h"

#include "TypeStructure.h"
#include "SchemaGenerator.h"
#include "SpatialConstants.h"
#include "SpatialGDKEditorSettings.h"
#include "Utils/CodeWriter.h"
#include "Utils/ComponentIdGenerator.h"
#include "Utils/DataTypeUtilities.h"
#include "Utils/SchemaDatabase.h"
#include "Editor/EditorEngine.h"

DEFINE_LOG_CATEGORY(LogSpatialGDKSchemaGenerator);

TArray<UClass*> SchemaGeneratedClasses;
TArray<UClass*> AdditionalSchemaGeneratedClasses; //Used to keep UClasses in memory whilst generating schema for them.
TMap<FString, FSchemaData> ClassPathToSchema;
uint32 NextAvailableComponentId;

// Prevent name collisions
TMap<UClass*, FString> ClassToSchemaName;
TMap<FString, UClass*> UsedSchemaNames;

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
	for (const auto& TypeInfo : TypeInfos)
	{
		UClass* Class = Cast<UClass>(TypeInfo->Type);
		check(Class);
		const FString& ClassName = Class->GetName();
		FString SchemaName = UnrealNameToSchemaName(ClassName);

		int Suffix = 0;
		while (UsedSchemaNames.Contains(SchemaName))
		{
			UE_LOG(LogSpatialGDKSchemaGenerator, Warning, TEXT("Class name collision after removing non-alphanumeric characters. Name '%s' collides for types '%s' and '%s'. Will add a suffix to resolve the collision."),
				*SchemaName, *Class->GetPathName(), *UsedSchemaNames[SchemaName]->GetPathName());

			SchemaName = UnrealNameToSchemaName(ClassName) + FString::Printf(TEXT("%d"), ++Suffix);
		}

		ClassToSchemaName.Add(Class, SchemaName);
		UsedSchemaNames.Add(SchemaName, Class);
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


void  GenerateSchemaFromClasses(const TArray<TSharedPtr<FUnrealType>>& TypeInfos, const FString& CombinedSchemaPath)
{
	// Generate the actual schema
	for (const auto& TypeInfo : TypeInfos)
	{
		NextAvailableComponentId += GenerateCompleteSchemaFromClass(CombinedSchemaPath, NextAvailableComponentId, TypeInfo);
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
		SchemaDatabase->NextAvailableComponentId = NextAvailableComponentId;
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

void DeleteGeneratedSchemaFiles()
{
	const FString SchemaOutputPath = GetDefault<USpatialGDKEditorSettings>()->GetGeneratedSchemaOutputFolder();
	IPlatformFile& PlatformFile = FPlatformFileManager::Get().GetPlatformFile();
	if (PlatformFile.DirectoryExists(*SchemaOutputPath))
	{
		if (!PlatformFile.DeleteDirectoryRecursively(*SchemaOutputPath))
		{
			UE_LOG(LogSpatialGDKSchemaGenerator, Error, TEXT("Could not clean the generated schema directory '%s'! Please make sure the directory and the files inside are writeable."), *SchemaOutputPath);
		}
	}
	PlatformFile.CreateDirectory(*SchemaOutputPath);
}

void InitClassPathToSchemaMap()
{
	TSoftObjectPtr<USchemaDatabase> SchemaDatabasePtr(FSoftObjectPath(TEXT("/Game/Spatial/SchemaDatabase.SchemaDatabase")));
	SchemaDatabasePtr.LoadSynchronous();
	const USchemaDatabase* const SchemaDatabase = SchemaDatabasePtr.Get();

	if (SchemaDatabase)
	{
		ClassPathToSchema = SchemaDatabase->ClassPathToSchema;
		NextAvailableComponentId = SchemaDatabase->NextAvailableComponentId;

		// Component Id generation was updated to be non-destructive, if we detect an old schema database, delete it.
		if (ClassPathToSchema.Num() > 0 && NextAvailableComponentId == SpatialConstants::STARTING_GENERATED_COMPONENT_ID)
		{
			UE_LOG(LogSpatialGDKSchemaGenerator, Warning, TEXT("Detected an old schema database, it'll be reset."));
			ClassPathToSchema.Empty();
			DeleteGeneratedSchemaFiles();
		}
	}
	else
	{
		UE_LOG(LogSpatialGDKSchemaGenerator, Log, TEXT("SchemaDatabase not found on Engine startup so the generated schema directory will be cleared out if it exists."));
		NextAvailableComponentId = SpatialConstants::STARTING_GENERATED_COMPONENT_ID;
		// As a safety precaution, if the SchemaDatabase.uasset doesn't exist then make sure the schema generated folder is cleared as well. 
		DeleteGeneratedSchemaFiles();
	}
}

void PreProcessSchemaMap()
{
	TArray<FString> EntriesToRemove;
	for (const auto& EntryIn : ClassPathToSchema)
	{
		const FString ClassPath = EntryIn.Key;
		const FSoftObjectPath ItemToReference(ClassPath);
		
		// First check if the object is already loaded into memory.
		UObject* const ResolvedObject = ItemToReference.ResolveObject();
		UClass*  const LoadedClass    = ResolvedObject ? nullptr : Cast<UClass>(ItemToReference.TryLoad());

		// only store classes that weren't currently loaded into memory
		if (LoadedClass)
		{
			// don't allow the Garbage Collector to delete these objects until we are done generating schema
			LoadedClass->AddToRoot();
			AdditionalSchemaGeneratedClasses.Add(LoadedClass);
		}
		// if the class isn't loaded then mark the entry for removal from the map
		else if(!ResolvedObject && !LoadedClass)
		{
			EntriesToRemove.Add(ClassPath);
		}
	}

	// this will prevent any garbage/unused classes from sticking around in the SchemaDatabase as clutter
	for (const auto& EntryIn : EntriesToRemove)
	{
		ClassPathToSchema.Remove(EntryIn);
	}
}

bool SpatialGDKGenerateSchema()
{
	// compile all changed blueprints
	TArray<UBlueprint*> ErroredBlueprints;
	bool PromptForCompilation = false;
	FInternalPlayLevelUtils::ResolveDirtyBlueprints(PromptForCompilation, ErroredBlueprints);

	if (ErroredBlueprints.Num() != 0)
	{
		UE_LOG(LogSpatialGDKSchemaGenerator, Error, TEXT("There were errors when compiling blueprints. Schema has not been generated."));
		for (auto& Blueprint : ErroredBlueprints)
		{
			UE_LOG(LogSpatialGDKSchemaGenerator, Error, TEXT("Could not compile %s"), *Blueprint->GetName());
		}
		return false;
	}

	ClassToSchemaName.Empty();
	UsedSchemaNames.Empty();

	// gets the classes currently loaded into memory
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

	check(GetDefault<UGeneralProjectSettings>()->bSpatialNetworking);

	DeleteGeneratedSchemaFiles();

	GenerateSchemaFromClasses(TypeInfos, SchemaOutputPath);

	SaveSchemaDatabase();

	//Allow the garbage collector to clean up classes that were manually loaded and forced to keep alive for the Schema Generator process.
	for (const auto& EntryIn : AdditionalSchemaGeneratedClasses)
	{
		if (EntryIn)
		{
			EntryIn->RemoveFromRoot();
		}
	}

	AdditionalSchemaGeneratedClasses.Empty();

	return true;
}
