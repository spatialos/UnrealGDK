// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "SpatialGDKEditorSchemaGenerator.h"

#include "Abilities/GameplayAbility.h"
#include "AssetRegistryModule.h"
#include "Async/Async.h"
#include "Components/SceneComponent.h"
#include "Editor.h"
#include "Engine/LevelScriptActor.h"
#include "Engine/LevelStreaming.h"
#include "Engine/World.h"
#include "GeneralProjectSettings.h"
#include "GenericPlatform/GenericPlatformFile.h"
#include "GenericPlatform/GenericPlatformProcess.h"
#include "HAL/PlatformFilemanager.h"
#include "Misc/ConfigCacheIni.h"
#include "Misc/FileHelper.h"
#include "Misc/MessageDialog.h"
#include "Misc/MonitoredProcess.h"
#include "Templates/SharedPointer.h"
#include "UObject/UObjectIterator.h"

#include "Engine/WorldComposition.h"
#include "Interop/SpatialClassInfoManager.h"
#include "Misc/ScopedSlowTask.h"
#include "SchemaGenerator.h"
#include "Settings/ProjectPackagingSettings.h"
#include "SpatialConstants.h"
#include "SpatialGDKEditorSettings.h"
#include "SpatialGDKServicesModule.h"
#include "TypeStructure.h"
#include "UObject/StrongObjectPtr.h"
#include "Utils/CodeWriter.h"
#include "Utils/ComponentIdGenerator.h"
#include "Utils/DataTypeUtilities.h"
#include "Utils/SchemaDatabase.h"

DEFINE_LOG_CATEGORY(LogSpatialGDKSchemaGenerator);
#define LOCTEXT_NAMESPACE "SpatialGDKSchemaGenerator"

TArray<UClass*> SchemaGeneratedClasses;
TMap<FString, FActorSchemaData> ActorClassPathToSchema;
TMap<FString, FSubobjectSchemaData> SubobjectClassPathToSchema;
uint32 NextAvailableComponentId = SpatialConstants::STARTING_GENERATED_COMPONENT_ID;

// LevelStreaming
TMap<FString, uint32> LevelPathToComponentId;
TSet<uint32> LevelComponentIds;

// Prevent name collisions.
TMap<FString, FString> ClassPathToSchemaName;
TMap<FString, FString> SchemaNameToClassPath;
TMap<FString, TSet<FString>> PotentialSchemaNameCollisions;

const FString RelativeSchemaDatabaseFilePath = FPaths::SetExtension(FPaths::Combine(FPaths::ProjectContentDir(), SpatialConstants::SCHEMA_DATABASE_FILE_PATH), FPackageName::GetAssetPackageExtension());

namespace SpatialGDKEditor
{
namespace Schema
{

void AddPotentialNameCollision(const FString& DesiredSchemaName, const FString& ClassPath, const FString& GeneratedSchemaName)
{
	PotentialSchemaNameCollisions.FindOrAdd(DesiredSchemaName).Add(FString::Printf(TEXT("%s(%s)"), *ClassPath, *GeneratedSchemaName));
}

void OnStatusOutput(FString Message)
{
	UE_LOG(LogSpatialGDKSchemaGenerator, Log, TEXT("%s"), *Message);
}

void GenerateCompleteSchemaFromClass(FString SchemaPath, FComponentIdGenerator& IdGenerator, TSharedPtr<FUnrealType> TypeInfo)
{
	UClass* Class = Cast<UClass>(TypeInfo->Type);
	FString SchemaFilename = UnrealNameToSchemaName(Class->GetName());

	if (Class->IsChildOf<AActor>())
	{
		GenerateActorSchema(IdGenerator, Class, TypeInfo, SchemaPath);
	}
	else
	{
		GenerateSubobjectSchema(IdGenerator, Class, TypeInfo, SchemaPath + TEXT("Subobjects/"));
	}
}

bool CheckSchemaNameValidity(FString Name, FString Identifier, FString Category)
{
	if (Name.IsEmpty())
	{
		UE_LOG(LogSpatialGDKSchemaGenerator, Error, TEXT("%s %s is empty after removing non-alphanumeric characters, schema not generated."), *Category, *Identifier);
		return false;
	}

	if (FChar::IsDigit(Name[0]))
	{
		UE_LOG(LogSpatialGDKSchemaGenerator, Error, TEXT("%s names should not start with digits. %s %s (%s) has leading digits (potentially after removing non-alphanumeric characters), schema not generated."), *Category, *Category, *Name, *Identifier);
		return false;
	}

	return true;
}

void CheckIdentifierNameValidity(TSharedPtr<FUnrealType> TypeInfo, bool& bOutSuccess)
{
	// Check Replicated data.
	FUnrealFlatRepData RepData = GetFlatRepData(TypeInfo);
	for (EReplicatedPropertyGroup Group : GetAllReplicatedPropertyGroups())
	{
		TMap<FString, TSharedPtr<FUnrealProperty>> SchemaReplicatedDataNames;
		for (auto& RepProp : RepData[Group])
		{
			FString NextSchemaReplicatedDataName = SchemaFieldName(RepProp.Value);

			if (!CheckSchemaNameValidity(NextSchemaReplicatedDataName, RepProp.Value->Property->GetPathName(), TEXT("Replicated property")))
			{
				bOutSuccess = false;
			}

			if (TSharedPtr<FUnrealProperty>* ExistingReplicatedProperty = SchemaReplicatedDataNames.Find(NextSchemaReplicatedDataName))
			{
				UE_LOG(LogSpatialGDKSchemaGenerator, Error, TEXT("Replicated property name collision after removing non-alphanumeric characters, schema not generated. Name '%s' collides for '%s' and '%s'"),
					*NextSchemaReplicatedDataName, *ExistingReplicatedProperty->Get()->Property->GetPathName(), *RepProp.Value->Property->GetPathName());
				bOutSuccess = false;
			}
			else
			{
				SchemaReplicatedDataNames.Add(NextSchemaReplicatedDataName, RepProp.Value);
			}
		}
	}

	// Check Handover data.
	FCmdHandlePropertyMap HandoverData = GetFlatHandoverData(TypeInfo);
	TMap<FString, TSharedPtr<FUnrealProperty>> SchemaHandoverDataNames;
	for (auto& Prop : HandoverData)
	{
		FString NextSchemaHandoverDataName = SchemaFieldName(Prop.Value);

		if (!CheckSchemaNameValidity(NextSchemaHandoverDataName, Prop.Value->Property->GetPathName(), TEXT("Handover property")))
		{
			bOutSuccess = false;
		}

		if (TSharedPtr<FUnrealProperty>* ExistingHandoverData = SchemaHandoverDataNames.Find(NextSchemaHandoverDataName))
		{
			UE_LOG(LogSpatialGDKSchemaGenerator, Error, TEXT("Handover data name collision after removing non-alphanumeric characters, schema not generated. Name '%s' collides for '%s' and '%s'"),
				*NextSchemaHandoverDataName, *ExistingHandoverData->Get()->Property->GetPathName(), *Prop.Value->Property->GetPathName());
			bOutSuccess = false;
		}
		else
		{
			SchemaHandoverDataNames.Add(NextSchemaHandoverDataName, Prop.Value);
		}
	}

	// Check subobject name validity.
	FSubobjectMap Subobjects = GetAllSubobjects(TypeInfo);
	TMap<FString, TSharedPtr<FUnrealType>> SchemaSubobjectNames;
	for (auto& It : Subobjects)
	{
		TSharedPtr<FUnrealType>& SubobjectTypeInfo = It.Value;
		FString NextSchemaSubobjectName = UnrealNameToSchemaComponentName(SubobjectTypeInfo->Name.ToString());

		if (!CheckSchemaNameValidity(NextSchemaSubobjectName, SubobjectTypeInfo->Object->GetPathName(), TEXT("Subobject")))
		{
			bOutSuccess = false;
		}

		if (TSharedPtr<FUnrealType>* ExistingSubobject = SchemaSubobjectNames.Find(NextSchemaSubobjectName))
		{
			UE_LOG(LogSpatialGDKSchemaGenerator, Error, TEXT("Subobject name collision after removing non-alphanumeric characters, schema not generated. Name '%s' collides for '%s' and '%s'"),
				*NextSchemaSubobjectName, *ExistingSubobject->Get()->Object->GetPathName(), *SubobjectTypeInfo->Object->GetPathName());
			bOutSuccess = false;
		}
		else
		{
			SchemaSubobjectNames.Add(NextSchemaSubobjectName, SubobjectTypeInfo);
		}
	}
}

bool ValidateIdentifierNames(TArray<TSharedPtr<FUnrealType>>& TypeInfos)
{
	bool bSuccess = true;

	// Remove all underscores from the class names, check for duplicates or invalid schema names.
	for (const auto& TypeInfo : TypeInfos)
	{
		UClass* Class = Cast<UClass>(TypeInfo->Type);
		check(Class);
		const FString& ClassName = Class->GetName();
		const FString& ClassPath = Class->GetPathName();
		FString SchemaName = UnrealNameToSchemaName(ClassName);

		if (!CheckSchemaNameValidity(SchemaName, ClassPath, TEXT("Class")))
		{
			bSuccess = false;
		}

		FString DesiredSchemaName = SchemaName;

		if (ClassPathToSchemaName.Contains(ClassPath))
		{
			continue;
		}

		int Suffix = 0;
		while (SchemaNameToClassPath.Contains(SchemaName))
		{
			SchemaName = UnrealNameToSchemaName(ClassName) + FString::Printf(TEXT("%d"), ++Suffix);
		}

		ClassPathToSchemaName.Add(ClassPath, SchemaName);
		SchemaNameToClassPath.Add(SchemaName, ClassPath);

		if (DesiredSchemaName != SchemaName)
		{
			AddPotentialNameCollision(DesiredSchemaName, ClassPath, SchemaName);
		}
		AddPotentialNameCollision(SchemaName, ClassPath, SchemaName);
	}

	for (const auto& Collision : PotentialSchemaNameCollisions)
	{
		if (Collision.Value.Num() > 1)
		{
			UE_LOG(LogSpatialGDKSchemaGenerator, Display, TEXT("Class name collision after removing non-alphanumeric characters. Name '%s' collides for classes [%s]"),
				*Collision.Key, *FString::Join(Collision.Value, TEXT(", ")));
		}
	}

	// Check for invalid/duplicate names in the generated type info.
	for (auto& TypeInfo : TypeInfos)
	{
		CheckIdentifierNameValidity(TypeInfo, bSuccess);
	}

	return bSuccess;
}

void GenerateSchemaFromClasses(const TArray<TSharedPtr<FUnrealType>>& TypeInfos, const FString& CombinedSchemaPath, FComponentIdGenerator& IdGenerator)
{
	// Generate the actual schema.
	FScopedSlowTask Progress((float)TypeInfos.Num(), LOCTEXT("GenerateSchemaFromClasses", "Generating Schema..."));
	for (const auto& TypeInfo : TypeInfos)
	{
		Progress.EnterProgressFrame(1.f);
		GenerateCompleteSchemaFromClass(CombinedSchemaPath, IdGenerator, TypeInfo);
	}
}

void WriteLevelComponent(FCodeWriter& Writer, FString LevelName, uint32 ComponentId, FString ClassPath)
{
	Writer.PrintNewLine();
	Writer.Printf("// {0}", *ClassPath);
	Writer.Printf("component {0} {", *UnrealNameToSchemaComponentName(LevelName));
	Writer.Indent();
	Writer.Printf("id = {0};", ComponentId);
	Writer.Outdent().Print("}");
}

TMultiMap<FName, FName> GetLevelNamesToPathsMap()
{
	FAssetRegistryModule& AssetRegistryModule = FModuleManager::LoadModuleChecked<FAssetRegistryModule>("AssetRegistry");

	TArray<FAssetData> WorldAssets;
	AssetRegistryModule.Get().GetAllAssets(WorldAssets, true);

	// Filter assets to game maps.
	WorldAssets = WorldAssets.FilterByPredicate([](FAssetData Data) {
		return (Data.AssetClass == UWorld::StaticClass()->GetFName() && Data.PackagePath.ToString().StartsWith("/Game"));
	});

	TMultiMap<FName, FName> LevelNamesToPaths;

	for (FAssetData World : WorldAssets)
	{
		LevelNamesToPaths.Add(World.AssetName, World.PackageName);
	}

	return LevelNamesToPaths;
}

void GenerateSchemaForSublevels()
{
	const FString SchemaOutputPath = GetDefault<USpatialGDKEditorSettings>()->GetGeneratedSchemaOutputFolder();
	TMultiMap<FName, FName> LevelNamesToPaths = GetLevelNamesToPathsMap();
	GenerateSchemaForSublevels(SchemaOutputPath, LevelNamesToPaths);
}

SPATIALGDKEDITOR_API void GenerateSchemaForSublevels(const FString& SchemaOutputPath, const TMultiMap<FName, FName>& LevelNamesToPaths)
{
	FCodeWriter Writer;
	Writer.Printf(R"""(
		// Copyright (c) Improbable Worlds Ltd, All Rights Reserved
		// Note that this file has been generated automatically
		package unreal.sublevels;)""");

	FComponentIdGenerator IdGenerator = FComponentIdGenerator(NextAvailableComponentId);

	TArray<FName> Keys;
	LevelNamesToPaths.GetKeys(Keys);

	for (FName LevelName : Keys)
	{
		if (LevelNamesToPaths.Num(LevelName) > 1)
		{
			// Write multiple numbered components.
			TArray<FName> LevelPaths;
			LevelNamesToPaths.MultiFind(LevelName, LevelPaths);
			FString LevelNameString = LevelName.ToString();

			for (int i = 0; i < LevelPaths.Num(); i++)
			{
				uint32 ComponentId = LevelPathToComponentId.FindRef(LevelPaths[i].ToString());
				if (ComponentId == 0)
				{
					ComponentId = IdGenerator.Next();
					LevelPathToComponentId.Add(LevelPaths[i].ToString(), ComponentId);
					LevelComponentIds.Add(ComponentId);
				}
				WriteLevelComponent(Writer, FString::Printf(TEXT("%sInd%d"), *LevelNameString, i), ComponentId, LevelPaths[i].ToString());
				
			}
		}
		else
		{
			// Write a single component.
			FString LevelPath = LevelNamesToPaths.FindRef(LevelName).ToString();
			uint32 ComponentId = LevelPathToComponentId.FindRef(LevelPath);
			if (ComponentId == 0)
			{
				ComponentId = IdGenerator.Next();
				LevelPathToComponentId.Add(LevelPath, ComponentId);
				LevelComponentIds.Add(ComponentId);
			}
			WriteLevelComponent(Writer, LevelName.ToString(), ComponentId, LevelPath);
		}
	}

	NextAvailableComponentId = IdGenerator.Peek();

	Writer.WriteToFile(FString::Printf(TEXT("%sSublevels/sublevels.schema"), *SchemaOutputPath));
}

FString GenerateIntermediateDirectory()
{
	const FString CombinedIntermediatePath = FPaths::Combine(*FPaths::GetPath(FPaths::GetProjectFilePath()), TEXT("Intermediate/Improbable/"), *FGuid::NewGuid().ToString(), TEXT("/"));
	FString AbsoluteCombinedIntermediatePath = FPaths::ConvertRelativePathToFull(CombinedIntermediatePath);
	FPlatformFileManager::Get().GetPlatformFile().CreateDirectoryTree(*AbsoluteCombinedIntermediatePath);

	return AbsoluteCombinedIntermediatePath;
}

TMap<uint32, FString> CreateComponentIdToClassPathMap()
{
	TMap<uint32, FString> ComponentIdToClassPath;

	for (const auto& ActorSchemaData : ActorClassPathToSchema)
	{
		ForAllSchemaComponentTypes([&](ESchemaComponentType Type)
		{
			ComponentIdToClassPath.Add(ActorSchemaData.Value.SchemaComponents[Type], ActorSchemaData.Key);
		});

		for (const auto& SubobjectSchemaData : ActorSchemaData.Value.SubobjectData)
		{
			ForAllSchemaComponentTypes([&](ESchemaComponentType Type)
			{
				ComponentIdToClassPath.Add(SubobjectSchemaData.Value.SchemaComponents[Type], SubobjectSchemaData.Value.ClassPath);
			});
		}
	}

	for (const auto& SubobjectSchemaData : SubobjectClassPathToSchema)
	{
		for (const auto& DynamicSubobjectData : SubobjectSchemaData.Value.DynamicSubobjectComponents)
		{
			ForAllSchemaComponentTypes([&](ESchemaComponentType Type)
			{
				ComponentIdToClassPath.Add(DynamicSubobjectData.SchemaComponents[Type], SubobjectSchemaData.Key);
			});
		}
	}

	ComponentIdToClassPath.Remove(SpatialConstants::INVALID_COMPONENT_ID);

	return ComponentIdToClassPath;
}

bool SaveSchemaDatabase(const FString& PackagePath)
{
	UPackage *Package = CreatePackage(nullptr, *PackagePath);

	USchemaDatabase* SchemaDatabase = NewObject<USchemaDatabase>(Package, USchemaDatabase::StaticClass(), FName("SchemaDatabase"), EObjectFlags::RF_Public | EObjectFlags::RF_Standalone);
	SchemaDatabase->NextAvailableComponentId = NextAvailableComponentId;
	SchemaDatabase->ActorClassPathToSchema = ActorClassPathToSchema;
	SchemaDatabase->SubobjectClassPathToSchema = SubobjectClassPathToSchema;
	SchemaDatabase->LevelPathToComponentId = LevelPathToComponentId;
	SchemaDatabase->ComponentIdToClassPath = CreateComponentIdToClassPathMap();
	SchemaDatabase->LevelComponentIds = LevelComponentIds;

	FAssetRegistryModule::AssetCreated(SchemaDatabase);
	SchemaDatabase->MarkPackageDirty();

	// NOTE: UPackage::GetMetaData() has some code where it will auto-create the metadata if it's missing
	// UPackage::SavePackage() calls UPackage::GetMetaData() at some point, and will cause an exception to get thrown
	// if the metadata auto-creation branch needs to be taken. This is the case when generating the schema from the
	// command line, so we just pre-empt it here.
	Package->GetMetaData();

	FString FilePath = FString::Printf(TEXT("%s%s"), *PackagePath, *FPackageName::GetAssetPackageExtension());
	bool bSuccess = UPackage::SavePackage(Package, SchemaDatabase, EObjectFlags::RF_Public | EObjectFlags::RF_Standalone, *FPackageName::LongPackageNameToFilename(PackagePath, FPackageName::GetAssetPackageExtension()), GError, nullptr, false, true, SAVE_NoError);

	if (!bSuccess)
	{
		FString FullPath = FPaths::ConvertRelativePathToFull(FilePath);
		FPaths::MakePlatformFilename(FullPath);
		FMessageDialog::Debugf(FText::FromString(FString::Printf(TEXT("Unable to save Schema Database to '%s'! The file may be locked by another process."), *FullPath)));
		return false;
	}
	return true;
}

bool IsSupportedClass(const UClass* SupportedClass)
{
	if (!IsValid(SupportedClass))
	{
		UE_LOG(LogSpatialGDKSchemaGenerator, Verbose, TEXT("[%s] Invalid Class not supported for schema gen."), *GetPathNameSafe(SupportedClass));
		return false;
	}

	if (SupportedClass->IsEditorOnly())
	{
		UE_LOG(LogSpatialGDKSchemaGenerator, Verbose, TEXT("[%s] Editor-only Class not supported for schema gen."), *GetPathNameSafe(SupportedClass));
		return false;
	}

	if (!SupportedClass->HasAnySpatialClassFlags(SPATIALCLASS_SpatialType))
	{
		if (SupportedClass->HasAnySpatialClassFlags(SPATIALCLASS_NotSpatialType))
		{
			UE_LOG(LogSpatialGDKSchemaGenerator, Verbose, TEXT("[%s] Has NotSpatialType flag, not supported for schema gen."), *GetPathNameSafe(SupportedClass));
			return false;
		}

		// Need to check if super class is supported here because some blueprints don't appear to inherit SpatialFlags correctly until
		// recompiled and saved. See [UNR-2172].
		UClass* Class = SupportedClass->GetSuperClass();
		while (Class != nullptr)
		{
			if (Class->HasAnySpatialClassFlags(SPATIALCLASS_SpatialType | SPATIALCLASS_NotSpatialType))
			{
				break;
			}
			Class = Class->GetSuperClass();
		}

		if (Class == nullptr || !Class->HasAnySpatialClassFlags(SPATIALCLASS_SpatialType))
		{
			UE_LOG(LogSpatialGDKSchemaGenerator, Verbose, TEXT("[%s] No SpatialType flag, not supported for schema gen."), *GetPathNameSafe(SupportedClass));
			return false;
		}
	}

	if (SupportedClass->HasAnyClassFlags(CLASS_LayoutChanging))
	{
		UE_LOG(LogSpatialGDKSchemaGenerator, Verbose, TEXT("[%s] Layout changing, not supported"), *GetPathNameSafe(SupportedClass));
		return false;
	}

	// Ensure we don't process transient generated classes for BP
	if (SupportedClass->GetName().StartsWith(TEXT("SKEL_"), ESearchCase::CaseSensitive)
		|| SupportedClass->GetName().StartsWith(TEXT("REINST_"), ESearchCase::CaseSensitive)
		|| SupportedClass->GetName().StartsWith(TEXT("TRASHCLASS_"), ESearchCase::CaseSensitive)
		|| SupportedClass->GetName().StartsWith(TEXT("HOTRELOADED_"), ESearchCase::CaseSensitive)
		|| SupportedClass->GetName().StartsWith(TEXT("PROTO_BP_"), ESearchCase::CaseSensitive)
		|| SupportedClass->GetName().StartsWith(TEXT("PLACEHOLDER-CLASS_"), ESearchCase::CaseSensitive)
		|| SupportedClass->GetName().StartsWith(TEXT("ORPHANED_DATA_ONLY_"), ESearchCase::CaseSensitive))
	{
		UE_LOG(LogSpatialGDKSchemaGenerator, Verbose, TEXT("[%s] Transient Class not supported for schema gen"), *GetPathNameSafe(SupportedClass));
		return false;
	}

	const TArray<FDirectoryPath>& DirectoriesToNeverCook = GetDefault<UProjectPackagingSettings>()->DirectoriesToNeverCook;

	// Avoid processing classes contained in Directories to Never Cook
	const FString& ClassPath = SupportedClass->GetPathName();
	if (DirectoriesToNeverCook.ContainsByPredicate([&ClassPath](const FDirectoryPath& Directory)
	{
		return ClassPath.StartsWith(Directory.Path);
	}))
	{
		UE_LOG(LogSpatialGDKSchemaGenerator, Verbose, TEXT("[%s] Inside Directory to never cook for schema gen"), *GetPathNameSafe(SupportedClass));
		return false;
	}

	UE_LOG(LogSpatialGDKSchemaGenerator, Verbose, TEXT("[%s] Supported Class"), *GetPathNameSafe(SupportedClass));
	return true;
}


TSet<UClass*> GetAllSupportedClasses(const TArray<UObject*>& AllClasses)
{
	TSet<UClass*> Classes;

	for (const auto& ClassIt : AllClasses)
	{
		UClass* SupportedClass = Cast<UClass>(ClassIt);

		if (IsSupportedClass(SupportedClass))
		{
			Classes.Add(SupportedClass);
		}
	}

	return Classes;
}

void CopyWellKnownSchemaFiles(const FString& GDKSchemaCopyDir, const FString& CoreSDKSchemaCopyDir)
{
	FString PluginDir = FSpatialGDKServicesModule::GetSpatialGDKPluginDirectory();

	FString GDKSchemaDir = FPaths::Combine(PluginDir, TEXT("SpatialGDK/Extras/schema"));
	FString CoreSDKSchemaDir = FPaths::Combine(PluginDir, TEXT("SpatialGDK/Binaries/ThirdParty/Improbable/Programs/schema"));
	
	IPlatformFile& PlatformFile = FPlatformFileManager::Get().GetPlatformFile();

	if (!PlatformFile.DirectoryExists(*GDKSchemaCopyDir))
	{
		if (!PlatformFile.CreateDirectoryTree(*GDKSchemaCopyDir))
		{
			UE_LOG(LogSpatialGDKSchemaGenerator, Error, TEXT("Could not create gdk schema directory '%s'! Please make sure the parent directory is writeable."), *GDKSchemaCopyDir);
		}
	}

	if (!PlatformFile.DirectoryExists(*CoreSDKSchemaCopyDir))
	{
		if (!PlatformFile.CreateDirectoryTree(*CoreSDKSchemaCopyDir))
		{
			UE_LOG(LogSpatialGDKSchemaGenerator, Error, TEXT("Could not create standard library schema directory '%s'! Please make sure the parent directory is writeable."), *GDKSchemaCopyDir);
		}
	}

	if (!PlatformFile.CopyDirectoryTree(*GDKSchemaCopyDir, *GDKSchemaDir, true /*bOverwriteExisting*/))
	{
		UE_LOG(LogSpatialGDKSchemaGenerator, Error, TEXT("Could not copy gdk schema to '%s'! Please make sure the directory is writeable."), *GDKSchemaCopyDir);
	}

	if (!PlatformFile.CopyDirectoryTree(*CoreSDKSchemaCopyDir, *CoreSDKSchemaDir, true /*bOverwriteExisting*/))
	{
		UE_LOG(LogSpatialGDKSchemaGenerator, Error, TEXT("Could not copy standard library schema to '%s'! Please make sure the directory is writeable."), *CoreSDKSchemaCopyDir);
	}
}

void DeleteGeneratedSchemaFiles(const FString& SchemaOutputPath)
{
	IPlatformFile& PlatformFile = FPlatformFileManager::Get().GetPlatformFile();
	if (PlatformFile.DirectoryExists(*SchemaOutputPath))
	{
		if (!PlatformFile.DeleteDirectoryRecursively(*SchemaOutputPath))
		{
			UE_LOG(LogSpatialGDKSchemaGenerator, Error, TEXT("Could not clean the generated schema directory '%s'! Please make sure the directory and the files inside are writeable."), *SchemaOutputPath);
		}
	}
}

void CreateGeneratedSchemaFolder()
{
	IPlatformFile& PlatformFile = FPlatformFileManager::Get().GetPlatformFile();
	FString SchemaOutputPath = GetDefault<USpatialGDKEditorSettings>()->GetGeneratedSchemaOutputFolder();
	PlatformFile.CreateDirectoryTree(*SchemaOutputPath);
}

void ResetSchemaGeneratorState()
{
	ActorClassPathToSchema.Empty();
	SubobjectClassPathToSchema.Empty();
	LevelComponentIds.Empty();
	LevelPathToComponentId.Empty();
	NextAvailableComponentId = SpatialConstants::STARTING_GENERATED_COMPONENT_ID;
	SchemaGeneratedClasses.Empty();
}

 void ResetSchemaGeneratorStateAndCleanupFolders()
{
	ResetSchemaGeneratorState();
	DeleteGeneratedSchemaFiles(GetDefault<USpatialGDKEditorSettings>()->GetGeneratedSchemaOutputFolder());
	CreateGeneratedSchemaFolder();
}

bool LoadGeneratorStateFromSchemaDatabase(const FString& FileName)
	{

	FString RelativeFileName = FPaths::Combine(FPaths::ProjectContentDir(), FileName);
	RelativeFileName = FPaths::SetExtension(RelativeFileName, FPackageName::GetAssetPackageExtension());

	if (IsAssetReadOnly(FileName))
	{
		FString AbsoluteFilePath = FPaths::ConvertRelativePathToFull(RelativeFileName);
		UE_LOG(LogSpatialGDKSchemaGenerator, Error, TEXT("Schema Generation failed: Schema Database at %s is read only. Make it writable before generating schema"), *AbsoluteFilePath);
		return false;
	}

	bool bResetSchema = false;

	FFileStatData StatData = FPlatformFileManager::Get().GetPlatformFile().GetStatData(*RelativeFileName);
	if (StatData.bIsValid)
	{
		const FString DatabaseAssetPath = FPaths::SetExtension(FPaths::Combine(TEXT("/Game/"), FileName), TEXT(".SchemaDatabase"));
		const USchemaDatabase* const SchemaDatabase = Cast<USchemaDatabase>(FSoftObjectPath(DatabaseAssetPath).TryLoad());

		if (SchemaDatabase == nullptr)
		{
			UE_LOG(LogSpatialGDKSchemaGenerator, Error, TEXT("Schema Generation failed: Failed to load existing schema database. If this continues, delete the schema database and try again."));
			return false;
		}

		ActorClassPathToSchema = SchemaDatabase->ActorClassPathToSchema;
		SubobjectClassPathToSchema = SchemaDatabase->SubobjectClassPathToSchema;
		LevelComponentIds = SchemaDatabase->LevelComponentIds;
		LevelPathToComponentId = SchemaDatabase->LevelPathToComponentId;
		NextAvailableComponentId = SchemaDatabase->NextAvailableComponentId;

		// Component Id generation was updated to be non-destructive, if we detect an old schema database, delete it.
		if (ActorClassPathToSchema.Num() > 0 && NextAvailableComponentId == SpatialConstants::STARTING_GENERATED_COMPONENT_ID)
		{
			return false;
		}
	}
	else
	{
		return false;
	}

	return true;
}

bool IsAssetReadOnly(FString FileName)
{
	FString RelativeFileName = FPaths::Combine(FPaths::ProjectContentDir(), FileName);
	RelativeFileName = FPaths::SetExtension(RelativeFileName, FPackageName::GetAssetPackageExtension());

	FFileStatData StatData = FPlatformFileManager::Get().GetPlatformFile().GetStatData(*RelativeFileName);

	if (StatData.bIsValid && StatData.bIsReadOnly)
	{
		return true;
	}

	return false;
}

bool GeneratedSchemaFolderExists()
{
	const FString SchemaOutputPath = GetDefault<USpatialGDKEditorSettings>()->GetGeneratedSchemaOutputFolder();
	IPlatformFile& PlatformFile = FPlatformFileManager::Get().GetPlatformFile();
	return PlatformFile.DirectoryExists(*SchemaOutputPath);
}

bool DeleteSchemaDatabase(const FString& PackagePath)
{
	FString DatabaseAssetPath = "";


	DatabaseAssetPath = FPaths::SetExtension(FPaths::Combine(FPaths::ProjectContentDir(), PackagePath), FPackageName::GetAssetPackageExtension());
	FFileStatData StatData = FPlatformFileManager::Get().GetPlatformFile().GetStatData(*DatabaseAssetPath);

	if (StatData.bIsValid)
	{
		if (IsAssetReadOnly(PackagePath))
		{
			UE_LOG(LogSpatialGDKSchemaGenerator, Error, TEXT("Unable to delete schema database at %s because it is read-only."), *DatabaseAssetPath);
			return false;
		}

		if (!FPlatformFileManager::Get().GetPlatformFile().DeleteFile(*DatabaseAssetPath))
		{
			// This should never run, since DeleteFile should only return false if the file does not exist which we have already checked for.
			UE_LOG(LogSpatialGDKSchemaGenerator, Error, TEXT("Unable to delete schema database at %s"), *DatabaseAssetPath);
			return false;
		}
	}

	return true;
}

bool GeneratedSchemaDatabaseExists()
{
	IPlatformFile& PlatformFile = FPlatformFileManager::Get().GetPlatformFile();
	
	return PlatformFile.FileExists(*RelativeSchemaDatabaseFilePath);
}

void ResolveClassPathToSchemaName(const FString& ClassPath, const FString& SchemaName)
{
	if (SchemaName.IsEmpty())
	{
		return;
	}

	ClassPathToSchemaName.Add(ClassPath, SchemaName);
	SchemaNameToClassPath.Add(SchemaName, ClassPath);
	FSoftObjectPath ObjPath = FSoftObjectPath(ClassPath);
	FString DesiredSchemaName = UnrealNameToSchemaName(ObjPath.GetAssetName());

	if (DesiredSchemaName != SchemaName)
	{
		AddPotentialNameCollision(DesiredSchemaName, ClassPath, SchemaName);
	}
	AddPotentialNameCollision(SchemaName, ClassPath, SchemaName);
}

void ResetUsedNames()
{
	ClassPathToSchemaName.Empty();
	SchemaNameToClassPath.Empty();
	PotentialSchemaNameCollisions.Empty();

	for (const TPair<FString, FActorSchemaData>& Entry : ActorClassPathToSchema)
	{
		ResolveClassPathToSchemaName(Entry.Key, Entry.Value.GeneratedSchemaName);
	}

 	for (const TPair< FString, FSubobjectSchemaData>& Entry : SubobjectClassPathToSchema)
 	{
		ResolveClassPathToSchemaName(Entry.Key, Entry.Value.GeneratedSchemaName);
 	}
}

bool RunSchemaCompiler()
{
	FString PluginDir = FSpatialGDKServicesModule::GetSpatialGDKPluginDirectory();

	// Get the schema_compiler path and arguments
	FString SchemaCompilerExe = FPaths::Combine(PluginDir, TEXT("SpatialGDK/Binaries/ThirdParty/Improbable/Programs/schema_compiler.exe"));

	FString SchemaDir = FPaths::Combine(FSpatialGDKServicesModule::GetSpatialOSDirectory(), TEXT("schema"));
	FString CoreSDKSchemaDir = FPaths::Combine(FSpatialGDKServicesModule::GetSpatialOSDirectory(), TEXT("build/dependencies/schema/standard_library"));
	FString SchemaDescriptorDir = FPaths::Combine(FSpatialGDKServicesModule::GetSpatialOSDirectory(), TEXT("build/assembly/schema"));
	FString SchemaDescriptorOutput = FPaths::Combine(SchemaDescriptorDir, TEXT("schema.descriptor"));

	// The schema_compiler cannot create folders.
	if (!FPaths::DirectoryExists(SchemaDescriptorDir))
	{
		IPlatformFile& PlatformFile = FPlatformFileManager::Get().GetPlatformFile();
		if (!PlatformFile.CreateDirectoryTree(*SchemaDescriptorDir))
		{
			UE_LOG(LogSpatialGDKSchemaGenerator, Error, TEXT("Could not create schema descriptor directory '%s'! Please make sure the parent directory is writeable."), *SchemaDescriptorDir);
			return false;
		}
	}

	FString SchemaCompilerArgs = FString::Printf(TEXT("--schema_path=\"%s\" --schema_path=\"%s\" --descriptor_set_out=\"%s\" --load_all_schema_on_schema_path"), *SchemaDir, *CoreSDKSchemaDir, *SchemaDescriptorOutput);

	UE_LOG(LogSpatialGDKSchemaGenerator, Log, TEXT("Starting '%s' with `%s` arguments."), *SchemaCompilerExe, *SchemaCompilerArgs);

	int32 ExitCode = 1;
	FString SchemaCompilerOut;
	FString SchemaCompilerErr;
	FPlatformProcess::ExecProcess(*SchemaCompilerExe, *SchemaCompilerArgs, &ExitCode, &SchemaCompilerOut, &SchemaCompilerErr);

	if (ExitCode == 0)
	{
		UE_LOG(LogSpatialGDKSchemaGenerator, Log, TEXT("schema_compiler successfully generated schema descriptor: %s"), *SchemaCompilerOut);
		return true;
	}
	else
	{
		UE_LOG(LogSpatialGDKSchemaGenerator, Error, TEXT("schema_compiler failed to generate schema descriptor: %s"), *SchemaCompilerErr);
		return false;
	}
}

bool SpatialGDKGenerateSchema()
{
	SchemaGeneratedClasses.Empty();

	// Generate Schema for classes loaded in memory.

	TArray<UObject*> AllClasses;
	GetObjectsOfClass(UClass::StaticClass(), AllClasses);
	if (!SpatialGDKGenerateSchemaForClasses(GetAllSupportedClasses(AllClasses), GetDefault<USpatialGDKEditorSettings>()->GetGeneratedSchemaOutputFolder()))
	{
		return false;
	}

	GenerateSchemaForSublevels();

	if (!SaveSchemaDatabase(SpatialConstants::SCHEMA_DATABASE_ASSET_PATH))
	{
		return false;
	}

	return RunSchemaCompiler();
}

bool SpatialGDKGenerateSchemaForClasses(TSet<UClass*> Classes, FString SchemaOutputPath /*= ""*/)
{
	ResetUsedNames();
	Classes.Sort([](const UClass& A, const UClass& B)
	{
		return A.GetPathName() < B.GetPathName();
	});
	
	// Generate Type Info structs for all classes
	TArray<TSharedPtr<FUnrealType>> TypeInfos;

	for (const auto& Class : Classes)
	{
		if (SchemaGeneratedClasses.Contains(Class))
		{
			continue;
		}

		SchemaGeneratedClasses.Add(Class);
		// Parent and static array index start at 0 for checksum calculations.
		TSharedPtr<FUnrealType> TypeInfo = CreateUnrealTypeInfo(Class, 0, 0);
		TypeInfos.Add(TypeInfo);
		VisitAllObjects(TypeInfo, [&](TSharedPtr<FUnrealType> TypeNode)
		{
			if (UClass* NestedClass = Cast<UClass>(TypeNode->Type))
			{
				if (!SchemaGeneratedClasses.Contains(NestedClass) && IsSupportedClass(NestedClass))
				{
					TypeInfos.Add(CreateUnrealTypeInfo(NestedClass, 0, 0));
					SchemaGeneratedClasses.Add(NestedClass);
				}
			}
			return true;
		});
	}

	if (!ValidateIdentifierNames(TypeInfos))
	{
		return false;
	}

	if (SchemaOutputPath.IsEmpty())
	{
		SchemaOutputPath = GetDefault<USpatialGDKEditorSettings>()->GetGeneratedSchemaOutputFolder();
	}

	UE_LOG(LogSpatialGDKSchemaGenerator, Display, TEXT("Schema path %s"), *SchemaOutputPath);

	// Check schema path is valid.
	if (!FPaths::CollapseRelativeDirectories(SchemaOutputPath))
	{
		UE_LOG(LogSpatialGDKSchemaGenerator, Error, TEXT("Invalid path: '%s'. Schema not generated."), *SchemaOutputPath);
		return false;
	}

#if ENGINE_MINOR_VERSION <= 22
	check(GetDefault<UGeneralProjectSettings>()->UsesSpatialNetworking());
#endif

	FComponentIdGenerator IdGenerator = FComponentIdGenerator(NextAvailableComponentId);

	GenerateSchemaFromClasses(TypeInfos, SchemaOutputPath, IdGenerator);

	NextAvailableComponentId = IdGenerator.Peek();

	return true;
}

} // Schema
} // SpatialGDKEditor

#undef LOCTEXT_NAMESPACE
