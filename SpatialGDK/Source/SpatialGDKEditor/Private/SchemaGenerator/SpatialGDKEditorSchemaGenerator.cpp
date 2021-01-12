// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "SpatialGDKEditorSchemaGenerator.h"

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
#include "Hash/CityHash.h"
#include "Misc/ConfigCacheIni.h"
#include "Misc/FileHelper.h"
#include "Misc/MessageDialog.h"
#include "Misc/MonitoredProcess.h"
#include "Runtime/Launch/Resources/Version.h"
#include "Templates/SharedPointer.h"
#include "UObject/UObjectIterator.h"

#include "Engine/WorldComposition.h"
#include "Interop/SpatialClassInfoManager.h"
#include "Misc/ScopedSlowTask.h"
#include "SchemaGenerator.h"
#include "Settings/ProjectPackagingSettings.h"
#include "SpatialConstants.h"
#include "SpatialGDKEditorSettings.h"
#include "SpatialGDKServicesConstants.h"
#include "SpatialGDKServicesModule.h"
#include "SpatialGDKSettings.h"
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
Worker_ComponentId NextAvailableComponentId = SpatialConstants::STARTING_GENERATED_COMPONENT_ID;

// Sets of data/owner only/handover components
TMap<ESchemaComponentType, TSet<Worker_ComponentId>> SchemaComponentTypeToComponents;

// LevelStreaming
TMap<FString, Worker_ComponentId> LevelPathToComponentId;

// Prevent name collisions.
TMap<FString, FString> ClassPathToSchemaName;
TMap<FString, FString> SchemaNameToClassPath;
TMap<FString, TSet<FString>> PotentialSchemaNameCollisions;

// QBI
TMap<float, Worker_ComponentId> NetCullDistanceToComponentId;

const FString RelativeSchemaDatabaseFilePath = FPaths::SetExtension(
	FPaths::Combine(FPaths::ProjectContentDir(), SpatialConstants::SCHEMA_DATABASE_FILE_PATH), FPackageName::GetAssetPackageExtension());

namespace SpatialGDKEditor
{
namespace Schema
{
void AddPotentialNameCollision(const FString& DesiredSchemaName, const FString& ClassPath, const FString& GeneratedSchemaName)
{
	PotentialSchemaNameCollisions.FindOrAdd(DesiredSchemaName).Add(FString::Printf(TEXT("%s(%s)"), *ClassPath, *GeneratedSchemaName));
}

void OnStatusOutput(const FString& Message)
{
	UE_LOG(LogSpatialGDKSchemaGenerator, Log, TEXT("%s"), *Message);
}

void GenerateCompleteSchemaFromClass(const FString& SchemaPath, FComponentIdGenerator& IdGenerator, TSharedPtr<FUnrealType> TypeInfo)
{
	UClass* Class = Cast<UClass>(TypeInfo->Type);

	if (Class->IsChildOf<AActor>())
	{
		GenerateActorSchema(IdGenerator, Class, TypeInfo, SchemaPath);
	}
	else
	{
		GenerateSubobjectSchema(IdGenerator, Class, TypeInfo, SchemaPath + TEXT("Subobjects/"));
	}
}

bool CheckSchemaNameValidity(const FString& Name, const FString& Identifier, const FString& Category)
{
	if (Name.IsEmpty())
	{
		UE_LOG(LogSpatialGDKSchemaGenerator, Error,
			   TEXT("%s %s is empty after removing non-alphanumeric characters, schema not generated."), *Category, *Identifier);
		return false;
	}

	if (FChar::IsDigit(Name[0]))
	{
		UE_LOG(LogSpatialGDKSchemaGenerator, Error,
			   TEXT("%s names should not start with digits. %s %s (%s) has leading digits (potentially after removing non-alphanumeric "
					"characters), schema not generated."),
			   *Category, *Category, *Name, *Identifier);
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
				UE_LOG(LogSpatialGDKSchemaGenerator, Error,
					   TEXT("Replicated property name collision after removing non-alphanumeric characters, schema not generated. Name "
							"'%s' collides for '%s' and '%s'"),
					   *NextSchemaReplicatedDataName, *ExistingReplicatedProperty->Get()->Property->GetPathName(),
					   *RepProp.Value->Property->GetPathName());
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
			UE_LOG(LogSpatialGDKSchemaGenerator, Error,
				   TEXT("Handover data name collision after removing non-alphanumeric characters, schema not generated. Name '%s' collides "
						"for '%s' and '%s'"),
				   *NextSchemaHandoverDataName, *ExistingHandoverData->Get()->Property->GetPathName(),
				   *Prop.Value->Property->GetPathName());
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
			UE_LOG(LogSpatialGDKSchemaGenerator, Error,
				   TEXT("Subobject name collision after removing non-alphanumeric characters, schema not generated. Name '%s' collides for "
						"'%s' and '%s'"),
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
		FString SchemaName = UnrealNameToSchemaName(ClassName, true);

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
			UE_LOG(LogSpatialGDKSchemaGenerator, Display,
				   TEXT("Class name collision after removing non-alphanumeric characters. Name '%s' collides for classes [%s]"),
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

void GenerateSchemaFromClasses(const TArray<TSharedPtr<FUnrealType>>& TypeInfos, const FString& CombinedSchemaPath,
							   FComponentIdGenerator& IdGenerator)
{
	// Generate the actual schema.
	FScopedSlowTask Progress((float)TypeInfos.Num(), LOCTEXT("GenerateSchemaFromClasses", "Generating Schema..."));
	for (const auto& TypeInfo : TypeInfos)
	{
		Progress.EnterProgressFrame(1.f);
		GenerateCompleteSchemaFromClass(CombinedSchemaPath, IdGenerator, TypeInfo);
	}
}

void WriteLevelComponent(FCodeWriter& Writer, const FString& LevelName, Worker_ComponentId ComponentId, const FString& ClassPath)
{
	FString ComponentName = UnrealNameToSchemaComponentName(LevelName);
	Writer.PrintNewLine();
	Writer.Printf("// {0}", *ClassPath);
	Writer.Printf("component {0} {", *ComponentName);
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

void GenerateSchemaForSublevels(const FString& SchemaOutputPath, const TMultiMap<FName, FName>& LevelNamesToPaths)
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
				Worker_ComponentId ComponentId = LevelPathToComponentId.FindRef(LevelPaths[i].ToString());
				if (ComponentId == 0)
				{
					ComponentId = IdGenerator.Next();
					LevelPathToComponentId.Add(LevelPaths[i].ToString(), ComponentId);
				}
				WriteLevelComponent(Writer, FString::Printf(TEXT("%sInd%d"), *LevelNameString, i), ComponentId, LevelPaths[i].ToString());
			}
		}
		else
		{
			// Write a single component.
			FString LevelPath = LevelNamesToPaths.FindRef(LevelName).ToString();
			Worker_ComponentId ComponentId = LevelPathToComponentId.FindRef(LevelPath);
			if (ComponentId == 0)
			{
				ComponentId = IdGenerator.Next();
				LevelPathToComponentId.Add(LevelPath, ComponentId);
			}
			WriteLevelComponent(Writer, LevelName.ToString(), ComponentId, LevelPath);
		}
	}

	NextAvailableComponentId = IdGenerator.Peek();

	Writer.WriteToFile(FString::Printf(TEXT("%sSublevels/sublevels.schema"), *SchemaOutputPath));
}

void GenerateSchemaForRPCEndpoints()
{
	GenerateSchemaForRPCEndpoints(GetDefault<USpatialGDKEditorSettings>()->GetGeneratedSchemaOutputFolder());
}

void GenerateSchemaForRPCEndpoints(const FString& SchemaOutputPath)
{
	GenerateRPCEndpointsSchema(SchemaOutputPath);
}

void GenerateSchemaForNCDs()
{
	GenerateSchemaForNCDs(GetDefault<USpatialGDKEditorSettings>()->GetGeneratedSchemaOutputFolder());
}

void GenerateSchemaForNCDs(const FString& SchemaOutputPath)
{
	FCodeWriter Writer;
	Writer.Printf(R"""(
		// Copyright (c) Improbable Worlds Ltd, All Rights Reserved
		// Note that this file has been generated automatically
		package unreal.ncdcomponents;)""");

	FComponentIdGenerator IdGenerator = FComponentIdGenerator(NextAvailableComponentId);

	for (auto& NCDComponent : NetCullDistanceToComponentId)
	{
		const FString ComponentName = FString::Printf(TEXT("NetCullDistanceSquared%lld"), static_cast<uint64>(NCDComponent.Key));
		if (NCDComponent.Value == 0)
		{
			NCDComponent.Value = IdGenerator.Next();
		}

		FString SchemaComponentName = UnrealNameToSchemaComponentName(ComponentName);
		Worker_ComponentId ComponentId = NCDComponent.Value;

		Writer.PrintNewLine();
		Writer.Printf("// distance {0}", NCDComponent.Key);
		Writer.Printf("component {0} {", *SchemaComponentName);
		Writer.Indent();
		Writer.Printf("id = {0};", ComponentId);
		Writer.Outdent().Print("}");
	}

	NextAvailableComponentId = IdGenerator.Peek();

	Writer.WriteToFile(FString::Printf(TEXT("%sNetCullDistance/ncdcomponents.schema"), *SchemaOutputPath));
}

FString GenerateIntermediateDirectory()
{
	const FString CombinedIntermediatePath = FPaths::Combine(*FPaths::GetPath(FPaths::GetProjectFilePath()),
															 TEXT("Intermediate/Improbable/"), *FGuid::NewGuid().ToString(), TEXT("/"));
	FString AbsoluteCombinedIntermediatePath = FPaths::ConvertRelativePathToFull(CombinedIntermediatePath);
	FPlatformFileManager::Get().GetPlatformFile().CreateDirectoryTree(*AbsoluteCombinedIntermediatePath);

	return AbsoluteCombinedIntermediatePath;
}

TMap<Worker_ComponentId, FString> CreateComponentIdToClassPathMap()
{
	TMap<Worker_ComponentId, FString> ComponentIdToClassPath;

	for (const auto& ActorSchemaData : ActorClassPathToSchema)
	{
		ForAllSchemaComponentTypes([&](ESchemaComponentType Type) {
			ComponentIdToClassPath.Add(ActorSchemaData.Value.SchemaComponents[Type], ActorSchemaData.Key);
		});

		for (const auto& SubobjectSchemaData : ActorSchemaData.Value.SubobjectData)
		{
			ForAllSchemaComponentTypes([&](ESchemaComponentType Type) {
				ComponentIdToClassPath.Add(SubobjectSchemaData.Value.SchemaComponents[Type], SubobjectSchemaData.Value.ClassPath);
			});
		}
	}

	for (const auto& SubobjectSchemaData : SubobjectClassPathToSchema)
	{
		for (const auto& DynamicSubobjectData : SubobjectSchemaData.Value.DynamicSubobjectComponents)
		{
			ForAllSchemaComponentTypes([&](ESchemaComponentType Type) {
				ComponentIdToClassPath.Add(DynamicSubobjectData.SchemaComponents[Type], SubobjectSchemaData.Key);
			});
		}
	}

	ComponentIdToClassPath.Remove(SpatialConstants::INVALID_COMPONENT_ID);

	return ComponentIdToClassPath;
}

FString GetComponentSetNameBySchemaType(ESchemaComponentType SchemaType)
{
	switch (SchemaType)
	{
	case SCHEMA_Data:
		return SpatialConstants::DATA_COMPONENT_SET_NAME;
	case SCHEMA_OwnerOnly:
		return SpatialConstants::OWNER_ONLY_COMPONENT_SET_NAME;
	case SCHEMA_Handover:
		return SpatialConstants::HANDOVER_COMPONENT_SET_NAME;
	default:
		UE_LOG(LogSpatialGDKSchemaGenerator, Error, TEXT("Could not return component set name. Schema component type was invalid: %d"),
			   SchemaType);
		return FString();
	}
}

Worker_ComponentId GetComponentSetIdBySchemaType(ESchemaComponentType SchemaType)
{
	switch (SchemaType)
	{
	case SCHEMA_Data:
		return SpatialConstants::DATA_COMPONENT_SET_ID;
	case SCHEMA_OwnerOnly:
		return SpatialConstants::OWNER_ONLY_COMPONENT_SET_ID;
	case SCHEMA_Handover:
		return SpatialConstants::HANDOVER_COMPONENT_SET_ID;
	default:
		UE_LOG(LogSpatialGDKSchemaGenerator, Error, TEXT("Could not return component set ID. Schema component type was invalid: %d"),
			   SchemaType);
		return SpatialConstants::INVALID_COMPONENT_ID;
	}
}

FString GetComponentSetOutputPathBySchemaType(ESchemaComponentType SchemaType)
{
	const FString ComponentSetName = GetComponentSetNameBySchemaType(SchemaType);
	return FString::Printf(TEXT("%sComponentSets/%s.schema"), *GetDefault<USpatialGDKEditorSettings>()->GetGeneratedSchemaOutputFolder(),
						   *ComponentSetName);
}

void WriteServerAuthorityComponentSet(const USchemaDatabase* SchemaDatabase, TArray<Worker_ComponentId>& ServerAuthoritativeComponentIds)
{
	const FString SchemaOutputPath = GetDefault<USpatialGDKEditorSettings>()->GetGeneratedSchemaOutputFolder();

	FCodeWriter Writer;
	Writer.Printf(R"""(
		// Copyright (c) Improbable Worlds Ltd, All Rights Reserved
		// Note that this file has been generated automatically
		package unreal.generated;)""");
	Writer.PrintNewLine();

	// Write all import statements.
	{
		// Well-known SpatialOS and handwritten GDK schema files.
		for (const auto& WellKnownSchemaImport : SpatialConstants::ServerAuthorityWellKnownSchemaImports)
		{
			Writer.Printf("import \"{0}\";", WellKnownSchemaImport);
		}

		const FString IncludePath = TEXT("unreal/generated");
		for (const auto& GeneratedActorClass : SchemaDatabase->ActorClassPathToSchema)
		{
			const FString ActorClassName = UnrealNameToSchemaName(GeneratedActorClass.Value.GeneratedSchemaName);
			Writer.Printf("import \"{0}/{1}.schema\";", IncludePath, ActorClassName);
			if (GeneratedActorClass.Value.SubobjectData.Num() > 0)
			{
				Writer.Printf("import \"{0}/{1}Components.schema\";", IncludePath, ActorClassName);
			}
		}

		for (const auto& GeneratedSubObjectClass : SchemaDatabase->SubobjectClassPathToSchema)
		{
			const FString SubObjectClassName = UnrealNameToSchemaName(GeneratedSubObjectClass.Value.GeneratedSchemaName);
			Writer.Printf("import \"{0}/Subobjects/{1}.schema\";", IncludePath, SubObjectClassName);
		}
	}

	Writer.PrintNewLine();
	Writer.Printf("component_set {0} {", SpatialConstants::SERVER_AUTH_COMPONENT_SET_NAME).Indent();
	Writer.Printf("id = {0};", SpatialConstants::SERVER_AUTH_COMPONENT_SET_ID);
	Writer.Printf("components = [").Indent();

	// Write all components.
	{
		// Well-known SpatialOS and handwritten GDK components.
		for (const auto& WellKnownComponent : SpatialConstants::ServerAuthorityWellKnownComponents)
		{
			Writer.Printf("{0},", WellKnownComponent.Value);
		}

		// NCDs.
		for (auto& NCDComponent : NetCullDistanceToComponentId)
		{
			const FString NcdComponentName = FString::Printf(TEXT("NetCullDistanceSquared%lld"), static_cast<uint64>(NCDComponent.Key));
			Writer.Printf("unreal.ncdcomponents.{0},", NcdComponentName);
		}

		for (const auto& GeneratedActorClass : SchemaDatabase->ActorClassPathToSchema)
		{
			// Actor components.
			const FString& ActorClassName = UnrealNameToSchemaComponentName(GeneratedActorClass.Value.GeneratedSchemaName);
			ForAllSchemaComponentTypes([&](ESchemaComponentType SchemaType) {
				const Worker_ComponentId ComponentId = GeneratedActorClass.Value.SchemaComponents[SchemaType];
				ServerAuthoritativeComponentIds.Push(ComponentId);
				if (ComponentId != 0)
				{
					switch (SchemaType)
					{
					case SCHEMA_Data:
						Writer.Printf("unreal.generated.{0}.{1},", ActorClassName.ToLower(), ActorClassName);
						break;
					case SCHEMA_OwnerOnly:
						Writer.Printf("unreal.generated.{0}.{1}OwnerOnly,", ActorClassName.ToLower(), ActorClassName);
						break;
					case SCHEMA_Handover:
						Writer.Printf("unreal.generated.{0}.{1}Handover,", ActorClassName.ToLower(), ActorClassName);
						break;
					default:
						break;
					}
				}
			});

			// Actor static subobjects.
			for (const auto& ActorSubObjectData : GeneratedActorClass.Value.SubobjectData)
			{
				const FString ActorSubObjectName = UnrealNameToSchemaComponentName(ActorSubObjectData.Value.Name.ToString());
				ForAllSchemaComponentTypes([&](ESchemaComponentType SchemaType) {
					const Worker_ComponentId& ComponentId = ActorSubObjectData.Value.SchemaComponents[SchemaType];
					ServerAuthoritativeComponentIds.Push(ComponentId);
					if (ComponentId != 0)
					{
						switch (SchemaType)
						{
						case SCHEMA_Data:
							Writer.Printf("unreal.generated.{0}.subobjects.{1},", ActorClassName.ToLower(), ActorSubObjectName);
							break;
						case SCHEMA_OwnerOnly:
							Writer.Printf("unreal.generated.{0}.subobjects.{1}OwnerOnly,", ActorClassName.ToLower(), ActorSubObjectName);
							break;
						case SCHEMA_Handover:
							Writer.Printf("unreal.generated.{0}.subobjects.{1}Handover,", ActorClassName.ToLower(), ActorSubObjectName);
							break;
						default:
							break;
						}
					}
				});
			}
		}

		// Dynamic subobjects.
		for (const auto& GeneratedSubObjectClass : SchemaDatabase->SubobjectClassPathToSchema)
		{
			const FString& SubObjectClassName = UnrealNameToSchemaComponentName(GeneratedSubObjectClass.Value.GeneratedSchemaName);
			for (auto SubObjectNumber = 0; SubObjectNumber < GeneratedSubObjectClass.Value.DynamicSubobjectComponents.Num();
				 ++SubObjectNumber)
			{
				const FDynamicSubobjectSchemaData& SubObjectSchemaData =
					GeneratedSubObjectClass.Value.DynamicSubobjectComponents[SubObjectNumber];
				ForAllSchemaComponentTypes([&](ESchemaComponentType SchemaType) {
					const Worker_ComponentId& ComponentId = SubObjectSchemaData.SchemaComponents[SchemaType];
					ServerAuthoritativeComponentIds.Push(ComponentId);
					if (ComponentId != 0)
					{
						switch (SchemaType)
						{
						case SCHEMA_Data:
							Writer.Printf("unreal.generated.{0}Dynamic{1},", SubObjectClassName, SubObjectNumber + 1);
							break;
						case SCHEMA_OwnerOnly:
							Writer.Printf("unreal.generated.{0}OwnerOnlyDynamic{1},", SubObjectClassName, SubObjectNumber + 1);
							break;
						case SCHEMA_Handover:
							Writer.Printf("unreal.generated.{0}HandoverDynamic{1},", SubObjectClassName, SubObjectNumber + 1);
							break;
						default:
							break;
						}
					}
				});
			}
		}
	}

	Writer.RemoveTrailingComma();

	Writer.Outdent().Print("];");
	Writer.Outdent().Print("}");

	Writer.WriteToFile(FString::Printf(TEXT("%sComponentSets/ServerAuthoritativeComponentSet.schema"), *SchemaOutputPath));
}

void WriteClientAuthorityComponentSet()
{
	const FString SchemaOutputPath = GetDefault<USpatialGDKEditorSettings>()->GetGeneratedSchemaOutputFolder();

	FCodeWriter Writer;
	Writer.Printf(R"""(
		// Copyright (c) Improbable Worlds Ltd, All Rights Reserved
		// Note that this file has been generated automatically
		package unreal.generated;)""");
	Writer.PrintNewLine();

	// Write all import statements.
	for (const auto& WellKnownSchemaImport : SpatialConstants::ClientAuthorityWellKnownSchemaImports)
	{
		Writer.Printf("import \"{0}\";", WellKnownSchemaImport);
	}

	Writer.PrintNewLine();
	Writer.Printf("component_set {0} {", SpatialConstants::CLIENT_AUTH_COMPONENT_SET_NAME).Indent();
	Writer.Printf("id = {0};", SpatialConstants::CLIENT_AUTH_COMPONENT_SET_ID);
	Writer.Printf("components = [").Indent();

	// Write all import components.
	for (const auto& WellKnownComponent : SpatialConstants::ClientAuthorityWellKnownComponents)
	{
		Writer.Printf("{0},", WellKnownComponent.Value);
	}

	Writer.RemoveTrailingComma();

	Writer.Outdent().Print("];");
	Writer.Outdent().Print("}");

	Writer.WriteToFile(FString::Printf(TEXT("%sComponentSets/ClientAuthoritativeComponentSet.schema"), *SchemaOutputPath));
}

void WriteComponentSetBySchemaType(const USchemaDatabase* SchemaDatabase, ESchemaComponentType SchemaType)
{
	FCodeWriter Writer;
	Writer.Printf(R"""(
		// Copyright (c) Improbable Worlds Ltd, All Rights Reserved
		// Note that this file has been generated automatically
		package unreal.generated;)""");
	Writer.PrintNewLine();

	// Write all import statements.
	{
		const FString IncludePath = TEXT("unreal/generated");
		for (const auto& GeneratedActorClass : SchemaDatabase->ActorClassPathToSchema)
		{
			const FString ActorClassName = UnrealNameToSchemaName(GeneratedActorClass.Value.GeneratedSchemaName);
			if (GeneratedActorClass.Value.SchemaComponents[SchemaType] != 0)
			{
				Writer.Printf("import \"{0}/{1}.schema\";", IncludePath, ActorClassName);
			}
			for (const auto& SubObjectData : GeneratedActorClass.Value.SubobjectData)
			{
				if (SubObjectData.Value.SchemaComponents[SchemaType] != 0)
				{
					Writer.Printf("import \"{0}/{1}Components.schema\";", IncludePath, ActorClassName);
					break;
				}
			}
		}
		for (const auto& GeneratedSubObjectClass : SchemaDatabase->SubobjectClassPathToSchema)
		{
			const FString SubObjectClassName = UnrealNameToSchemaName(GeneratedSubObjectClass.Value.GeneratedSchemaName);
			for (const auto& SubObjectData : GeneratedSubObjectClass.Value.DynamicSubobjectComponents)
			{
				if (SubObjectData.SchemaComponents[SchemaType] != 0)
				{
					Writer.Printf("import \"{0}/Subobjects/{1}.schema\";", IncludePath, SubObjectClassName);
					break;
				}
			}
		}
	}

	Writer.PrintNewLine();
	Writer.Printf("component_set {0} {", GetComponentSetNameBySchemaType(SchemaType)).Indent();
	Writer.Printf("id = {0};", GetComponentSetIdBySchemaType(SchemaType));
	Writer.Printf("components = [").Indent();

	// Write all components.
	{
		for (const auto& GeneratedActorClass : SchemaDatabase->ActorClassPathToSchema)
		{
			// Actor components.
			const FString& ActorClassName = UnrealNameToSchemaComponentName(GeneratedActorClass.Value.GeneratedSchemaName);
			if (GeneratedActorClass.Value.SchemaComponents[SchemaType] != 0)
			{
				switch (SchemaType)
				{
				case SCHEMA_Data:
					Writer.Printf("unreal.generated.{0}.{1},", ActorClassName.ToLower(), ActorClassName);
					break;
				case SCHEMA_OwnerOnly:
					Writer.Printf("unreal.generated.{0}.{1}OwnerOnly,", ActorClassName.ToLower(), ActorClassName);
					break;
				case SCHEMA_Handover:
					Writer.Printf("unreal.generated.{0}.{1}Handover,", ActorClassName.ToLower(), ActorClassName);
					break;
				default:
					break;
				}
			}
			// Actor static subobjects.
			for (const auto& ActorSubObjectData : GeneratedActorClass.Value.SubobjectData)
			{
				const FString ActorSubObjectName = UnrealNameToSchemaComponentName(ActorSubObjectData.Value.Name.ToString());
				if (ActorSubObjectData.Value.SchemaComponents[SchemaType] != 0)
				{
					switch (SchemaType)
					{
					case SCHEMA_Data:
						Writer.Printf("unreal.generated.{0}.subobjects.{1},", ActorClassName.ToLower(), ActorSubObjectName);
						break;
					case SCHEMA_OwnerOnly:
						Writer.Printf("unreal.generated.{0}.subobjects.{1}OwnerOnly,", ActorClassName.ToLower(), ActorSubObjectName);
						break;
					case SCHEMA_Handover:
						Writer.Printf("unreal.generated.{0}.subobjects.{1}Handover,", ActorClassName.ToLower(), ActorSubObjectName);
						break;
					default:
						break;
					}
				}
			}
		}
		// Dynamic subobjects.
		for (const auto& GeneratedSubObjectClass : SchemaDatabase->SubobjectClassPathToSchema)
		{
			const FString& SubObjectClassName = UnrealNameToSchemaComponentName(GeneratedSubObjectClass.Value.GeneratedSchemaName);
			for (auto SubObjectNumber = 0; SubObjectNumber < GeneratedSubObjectClass.Value.DynamicSubobjectComponents.Num();
				 ++SubObjectNumber)
			{
				const FDynamicSubobjectSchemaData& SubObjectSchemaData =
					GeneratedSubObjectClass.Value.DynamicSubobjectComponents[SubObjectNumber];
				if (SubObjectSchemaData.SchemaComponents[SchemaType] != 0)
				{
					switch (SchemaType)
					{
					case SCHEMA_Data:
						Writer.Printf("unreal.generated.{0}Dynamic{1},", SubObjectClassName, SubObjectNumber + 1);
						break;
					case SCHEMA_OwnerOnly:
						Writer.Printf("unreal.generated.{0}OwnerOnlyDynamic{1},", SubObjectClassName, SubObjectNumber + 1);
						break;
					case SCHEMA_Handover:
						Writer.Printf("unreal.generated.{0}HandoverDynamic{1},", SubObjectClassName, SubObjectNumber + 1);
						break;
					default:
						break;
					}
				}
			}
		}
	}

	Writer.RemoveTrailingComma();

	Writer.Outdent().Print("];");
	Writer.Outdent().Print("}");

	const FString OutputPath = GetComponentSetOutputPathBySchemaType(SchemaType);
	Writer.WriteToFile(OutputPath);
}

USchemaDatabase* InitialiseSchemaDatabase(const FString& PackagePath)
{
#if ENGINE_MINOR_VERSION >= 26
	UPackage* Package = CreatePackage(*PackagePath);
#else
	UPackage* Package = CreatePackage(nullptr, *PackagePath);
#endif

	ActorClassPathToSchema.KeySort([](const FString& LHS, const FString& RHS) {
		return LHS < RHS;
	});
	SubobjectClassPathToSchema.KeySort([](const FString& LHS, const FString& RHS) {
		return LHS < RHS;
	});
	LevelPathToComponentId.KeySort([](const FString& LHS, const FString& RHS) {
		return LHS < RHS;
	});

	USchemaDatabase* SchemaDatabase = NewObject<USchemaDatabase>(Package, USchemaDatabase::StaticClass(), FName("SchemaDatabase"),
																 EObjectFlags::RF_Public | EObjectFlags::RF_Standalone);
	SchemaDatabase->NextAvailableComponentId = NextAvailableComponentId;
	SchemaDatabase->ActorClassPathToSchema = ActorClassPathToSchema;
	SchemaDatabase->SubobjectClassPathToSchema = SubobjectClassPathToSchema;
	SchemaDatabase->LevelPathToComponentId = LevelPathToComponentId;
	SchemaDatabase->NetCullDistanceToComponentId = NetCullDistanceToComponentId;
	SchemaDatabase->ComponentIdToClassPath = CreateComponentIdToClassPathMap();
	SchemaDatabase->DataComponentIds = SchemaComponentTypeToComponents[ESchemaComponentType::SCHEMA_Data].Array();
	SchemaDatabase->OwnerOnlyComponentIds = SchemaComponentTypeToComponents[ESchemaComponentType::SCHEMA_OwnerOnly].Array();
	SchemaDatabase->HandoverComponentIds = SchemaComponentTypeToComponents[ESchemaComponentType::SCHEMA_Handover].Array();

	SchemaDatabase->NetCullDistanceComponentIds.Reset();
	TArray<Worker_ComponentId> NetCullDistanceComponentIds;
	NetCullDistanceComponentIds.Reserve(NetCullDistanceToComponentId.Num());
	NetCullDistanceToComponentId.GenerateValueArray(NetCullDistanceComponentIds);
	SchemaDatabase->NetCullDistanceComponentIds.Append(NetCullDistanceComponentIds);

	SchemaDatabase->LevelComponentIds.Reset(LevelPathToComponentId.Num());
	LevelPathToComponentId.GenerateValueArray(SchemaDatabase->LevelComponentIds);

	SchemaDatabase->ComponentSetIdToComponentIds.Reset();
	for (const auto& WellKnownComponent : SpatialConstants::ServerAuthorityWellKnownComponents)
	{
		SchemaDatabase->ComponentSetIdToComponentIds.FindOrAdd(SpatialConstants::SERVER_AUTH_COMPONENT_SET_ID)
			.ComponentIDs.Push(WellKnownComponent.Key);
	}
	for (const auto& WellKnownComponent : SpatialConstants::ClientAuthorityWellKnownComponents)
	{
		SchemaDatabase->ComponentSetIdToComponentIds.FindOrAdd(SpatialConstants::CLIENT_AUTH_COMPONENT_SET_ID)
			.ComponentIDs.Push(WellKnownComponent.Key);
	}
	SchemaDatabase->ComponentSetIdToComponentIds.FindOrAdd(SpatialConstants::GDK_KNOWN_ENTITY_AUTH_COMPONENT_SET_ID)
		.ComponentIDs.Append(SpatialConstants::KnownEntityAuthorityComponents);
	SchemaDatabase->ComponentSetIdToComponentIds.FindOrAdd(SpatialConstants::SERVER_AUTH_COMPONENT_SET_ID)
		.ComponentIDs.Append(NetCullDistanceComponentIds);

	SchemaDatabase->SchemaDatabaseVersion = ESchemaDatabaseVersion::LatestVersion;

	return SchemaDatabase;
}

bool SaveSchemaDatabase(USchemaDatabase* SchemaDatabase)
{
	FString CompiledSchemaDir = FPaths::Combine(SpatialGDKServicesConstants::SpatialOSDirectory, TEXT("build/assembly/schema"));

	// Generate hash
	{
		SchemaDatabase->SchemaBundleHash = 0;
		FString SchemaBundlePath = FPaths::Combine(CompiledSchemaDir, TEXT("schema.sb"));
		IPlatformFile& PlatformFile = FPlatformFileManager::Get().GetPlatformFile();

		TUniquePtr<IFileHandle> FileHandle(PlatformFile.OpenRead(SchemaBundlePath.GetCharArray().GetData()));
		if (FileHandle)
		{
			// Create our byte buffer
			int64 FileSize = FileHandle->Size();
			TUniquePtr<uint8[]> ByteArray(new uint8[FileSize]);
			bool Result = FileHandle->Read(ByteArray.Get(), FileSize);
			if (Result)
			{
				SchemaDatabase->SchemaBundleHash = CityHash32(reinterpret_cast<const char*>(ByteArray.Get()), FileSize);
				UE_LOG(LogSpatialGDKSchemaGenerator, Display, TEXT("Generated schema bundle hash for database %u"),
					   SchemaDatabase->SchemaBundleHash);
			}
			else
			{
				UE_LOG(LogSpatialGDKSchemaGenerator, Warning, TEXT("Failed to fully read schema.sb. Schema not saved. Location: %s"),
					   *SchemaBundlePath);
			}
		}
		else
		{
			UE_LOG(LogSpatialGDKSchemaGenerator, Warning, TEXT("Failed to open schema.sb generated by the schema compiler! Location: %s"),
				   *SchemaBundlePath);
		}
	}

	FAssetRegistryModule::AssetCreated(SchemaDatabase);
	SchemaDatabase->MarkPackageDirty();

	// NOTE: UPackage::GetMetaData() has some code where it will auto-create the metadata if it's missing
	// UPackage::SavePackage() calls UPackage::GetMetaData() at some point, and will cause an exception to get thrown
	// if the metadata auto-creation branch needs to be taken. This is the case when generating the schema from the
	// command line, so we just preempt it here.
	UPackage* Package = SchemaDatabase->GetOutermost();
	const FString& PackagePath = Package->GetPathName();
	Package->GetMetaData();

	FString FilePath = FString::Printf(TEXT("%s%s"), *PackagePath, *FPackageName::GetAssetPackageExtension());
	bool bSuccess = UPackage::SavePackage(Package, SchemaDatabase, EObjectFlags::RF_Public | EObjectFlags::RF_Standalone,
										  *FPackageName::LongPackageNameToFilename(PackagePath, FPackageName::GetAssetPackageExtension()),
										  GError, nullptr, false, true, SAVE_NoError);

	if (!bSuccess)
	{
		FString FullPath = FPaths::ConvertRelativePathToFull(FilePath);
		FPaths::MakePlatformFilename(FullPath);
		FMessageDialog::Debugf(FText::Format(
			LOCTEXT("SchemaDatabaseLocked_Error", "Unable to save Schema Database to '{0}'! The file may be locked by another process."),
			FText::FromString(FullPath)));
		return false;
	}

	return true;
}

bool IsSupportedClass(const UClass* SupportedClass)
{
	if (!IsValid(SupportedClass))
	{
		UE_LOG(LogSpatialGDKSchemaGenerator, Verbose, TEXT("[%s] Invalid Class not supported for schema gen."),
			   *GetPathNameSafe(SupportedClass));
		return false;
	}

	if (SupportedClass->IsEditorOnly())
	{
		UE_LOG(LogSpatialGDKSchemaGenerator, Verbose, TEXT("[%s] Editor-only Class not supported for schema gen."),
			   *GetPathNameSafe(SupportedClass));
		return false;
	}

	if (!SupportedClass->HasAnySpatialClassFlags(SPATIALCLASS_SpatialType))
	{
		if (SupportedClass->HasAnySpatialClassFlags(SPATIALCLASS_NotSpatialType))
		{
			UE_LOG(LogSpatialGDKSchemaGenerator, Verbose, TEXT("[%s] Has NotSpatialType flag, not supported for schema gen."),
				   *GetPathNameSafe(SupportedClass));
		}
		else
		{
			UE_LOG(LogSpatialGDKSchemaGenerator, Verbose, TEXT("[%s] Has neither a SpatialType or NotSpatialType flag."),
				   *GetPathNameSafe(SupportedClass));
		}

		return false;
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
		UE_LOG(LogSpatialGDKSchemaGenerator, Verbose, TEXT("[%s] Transient Class not supported for schema gen"),
			   *GetPathNameSafe(SupportedClass));
		return false;
	}

	const TArray<FDirectoryPath>& DirectoriesToNeverCook = GetDefault<UProjectPackagingSettings>()->DirectoriesToNeverCook;

	// Avoid processing classes contained in Directories to Never Cook
	const FString& ClassPath = SupportedClass->GetPathName();
	if (DirectoriesToNeverCook.ContainsByPredicate([&ClassPath](const FDirectoryPath& Directory) {
			return ClassPath.StartsWith(Directory.Path);
		}))
	{
		UE_LOG(LogSpatialGDKSchemaGenerator, Verbose, TEXT("[%s] Inside Directory to never cook for schema gen"),
			   *GetPathNameSafe(SupportedClass));
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

	RefreshSchemaFiles(*GDKSchemaCopyDir);
	if (!PlatformFile.CopyDirectoryTree(*GDKSchemaCopyDir, *GDKSchemaDir, true /*bOverwriteExisting*/))
	{
		UE_LOG(LogSpatialGDKSchemaGenerator, Error, TEXT("Could not copy gdk schema to '%s'! Please make sure the directory is writeable."),
			   *GDKSchemaCopyDir);
	}

	RefreshSchemaFiles(*CoreSDKSchemaCopyDir);
	if (!PlatformFile.CopyDirectoryTree(*CoreSDKSchemaCopyDir, *CoreSDKSchemaDir, true /*bOverwriteExisting*/))
	{
		UE_LOG(LogSpatialGDKSchemaGenerator, Error,
			   TEXT("Could not copy standard library schema to '%s'! Please make sure the directory is writeable."), *CoreSDKSchemaCopyDir);
	}
}

bool RefreshSchemaFiles(const FString& SchemaOutputPath, const bool bDeleteExistingSchema /*= true*/,
						const bool bCreateDirectoryTree /*= true*/)
{
	IPlatformFile& PlatformFile = FPlatformFileManager::Get().GetPlatformFile();
	if (bDeleteExistingSchema && PlatformFile.DirectoryExists(*SchemaOutputPath))
	{
		if (!PlatformFile.DeleteDirectoryRecursively(*SchemaOutputPath))
		{
			UE_LOG(LogSpatialGDKSchemaGenerator, Error,
				   TEXT("Could not clean the schema directory '%s'! Please make sure the directory and the files inside are writeable."),
				   *SchemaOutputPath);
			return false;
		}
	}

	if (bCreateDirectoryTree && !PlatformFile.CreateDirectoryTree(*SchemaOutputPath))
	{
		UE_LOG(LogSpatialGDKSchemaGenerator, Error,
			   TEXT("Could not create schema directory '%s'! Please make sure the parent directory is writeable."), *SchemaOutputPath);
		return false;
	}
	return true;
}

void ResetSchemaGeneratorState()
{
	ActorClassPathToSchema.Empty();
	SubobjectClassPathToSchema.Empty();
	SchemaComponentTypeToComponents.Empty();
	ForAllSchemaComponentTypes([&](ESchemaComponentType Type) {
		SchemaComponentTypeToComponents.Add(Type, TSet<Worker_ComponentId>());
	});
	LevelPathToComponentId.Empty();
	NextAvailableComponentId = SpatialConstants::STARTING_GENERATED_COMPONENT_ID;
	SchemaGeneratedClasses.Empty();
	NetCullDistanceToComponentId.Empty();
}

void ResetSchemaGeneratorStateAndCleanupFolders()
{
	ResetSchemaGeneratorState();
	RefreshSchemaFiles(GetDefault<USpatialGDKEditorSettings>()->GetGeneratedSchemaOutputFolder());
}

bool LoadGeneratorStateFromSchemaDatabase(const FString& FileName)
{
	FString RelativeFileName = FPaths::Combine(FPaths::ProjectContentDir(), FileName);
	RelativeFileName = FPaths::SetExtension(RelativeFileName, FPackageName::GetAssetPackageExtension());

	if (IsAssetReadOnly(FileName))
	{
		FString AbsoluteFilePath = FPaths::ConvertRelativePathToFull(RelativeFileName);
		UE_LOG(LogSpatialGDKSchemaGenerator, Error,
			   TEXT("Schema Generation failed: Schema Database at %s is read only. Make it writable before generating schema"),
			   *AbsoluteFilePath);
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
			UE_LOG(LogSpatialGDKSchemaGenerator, Error,
				   TEXT("Schema Generation failed: Failed to load existing schema database. If this continues, delete the schema database "
						"and try again."));
			return false;
		}

		ActorClassPathToSchema = SchemaDatabase->ActorClassPathToSchema;
		SubobjectClassPathToSchema = SchemaDatabase->SubobjectClassPathToSchema;
		SchemaComponentTypeToComponents.Empty();
		SchemaComponentTypeToComponents.Add(ESchemaComponentType::SCHEMA_Data, TSet<Worker_ComponentId>(SchemaDatabase->DataComponentIds));
		SchemaComponentTypeToComponents.Add(ESchemaComponentType::SCHEMA_OwnerOnly,
											TSet<Worker_ComponentId>(SchemaDatabase->OwnerOnlyComponentIds));
		SchemaComponentTypeToComponents.Add(ESchemaComponentType::SCHEMA_Handover,
											TSet<Worker_ComponentId>(SchemaDatabase->HandoverComponentIds));
		LevelPathToComponentId = SchemaDatabase->LevelPathToComponentId;
		NextAvailableComponentId = SchemaDatabase->NextAvailableComponentId;
		NetCullDistanceToComponentId = SchemaDatabase->NetCullDistanceToComponentId;

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

bool IsAssetReadOnly(const FString& FileName)
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

	DatabaseAssetPath =
		FPaths::SetExtension(FPaths::Combine(FPaths::ProjectContentDir(), PackagePath), FPackageName::GetAssetPackageExtension());
	FFileStatData StatData = FPlatformFileManager::Get().GetPlatformFile().GetStatData(*DatabaseAssetPath);

	if (StatData.bIsValid)
	{
		if (IsAssetReadOnly(PackagePath))
		{
			UE_LOG(LogSpatialGDKSchemaGenerator, Error, TEXT("Unable to delete schema database at %s because it is read-only."),
				   *DatabaseAssetPath);
			return false;
		}

		if (!FPlatformFileManager::Get().GetPlatformFile().DeleteFile(*DatabaseAssetPath))
		{
			// This should never run, since DeleteFile should only return false if the file does not exist which we have already checked
			// for.
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

FSpatialGDKEditor::ESchemaDatabaseValidationResult ValidateSchemaDatabase()
{
	FFileStatData StatData = FPlatformFileManager::Get().GetPlatformFile().GetStatData(*RelativeSchemaDatabaseFilePath);
	if (!StatData.bIsValid)
	{
		return FSpatialGDKEditor::NotFound;
	}

	const FString DatabaseAssetPath = FPaths::SetExtension(SpatialConstants::SCHEMA_DATABASE_ASSET_PATH, TEXT(".SchemaDatabase"));
	const USchemaDatabase* const SchemaDatabase = Cast<USchemaDatabase>(FSoftObjectPath(DatabaseAssetPath).TryLoad());

	if (SchemaDatabase == nullptr)
	{
		return FSpatialGDKEditor::NotFound;
	}

	if (SchemaDatabase->SchemaDatabaseVersion < ESchemaDatabaseVersion::LatestVersion)
	{
		return FSpatialGDKEditor::OldVersion;
	}

	return FSpatialGDKEditor::Ok;
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

	for (const TPair<FString, FSubobjectSchemaData>& Entry : SubobjectClassPathToSchema)
	{
		ResolveClassPathToSchemaName(Entry.Key, Entry.Value.GeneratedSchemaName);
	}
}

bool RunSchemaCompiler()
{
	FString PluginDir = FSpatialGDKServicesModule::GetSpatialGDKPluginDirectory();

	// Get the schema_compiler path and arguments
	FString SchemaCompilerExe = FPaths::Combine(PluginDir, TEXT("SpatialGDK/Binaries/ThirdParty/Improbable/Programs/schema_compiler.exe"));

	FString SchemaDir = FPaths::Combine(SpatialGDKServicesConstants::SpatialOSDirectory, TEXT("schema"));
	FString CoreSDKSchemaDir =
		FPaths::Combine(SpatialGDKServicesConstants::SpatialOSDirectory, TEXT("build/dependencies/schema/standard_library"));
	FString CompiledSchemaDir = FPaths::Combine(SpatialGDKServicesConstants::SpatialOSDirectory, TEXT("build/assembly/schema"));
	FString CompiledSchemaASTDir = FPaths::Combine(CompiledSchemaDir, TEXT("ast"));
	FString SchemaBundleOutput = FPaths::Combine(CompiledSchemaDir, TEXT("schema.sb"));
	FString SchemaBundleJsonOutput = FPaths::Combine(CompiledSchemaDir, TEXT("schema.json"));

	IPlatformFile& PlatformFile = FPlatformFileManager::Get().GetPlatformFile();

	const FString& SchemaCompilerBaseArgs = FString::Printf(TEXT("--schema_path=\"%s\" --schema_path=\"%s\" --bundle_out=\"%s\" "
																 "--bundle_json_out=\"%s\" --load_all_schema_on_schema_path "),
															*SchemaDir, *CoreSDKSchemaDir, *SchemaBundleOutput, *SchemaBundleJsonOutput);

	// If there's already a compiled schema dir, blow it away so we don't have lingering artifacts from previous generation runs.
	if (FPaths::DirectoryExists(CompiledSchemaDir))
	{
		if (!PlatformFile.DeleteDirectoryRecursively(*CompiledSchemaDir))
		{
			UE_LOG(LogSpatialGDKSchemaGenerator, Error,
				   TEXT("Could not delete pre-existing compiled schema directory '%s'! Please make sure the directory is writeable."),
				   *CompiledSchemaDir);
			return false;
		}
	}

	// schema_compiler cannot create folders, so we need to set them up beforehand.
	if (!PlatformFile.CreateDirectoryTree(*CompiledSchemaDir))
	{
		UE_LOG(LogSpatialGDKSchemaGenerator, Error,
			   TEXT("Could not create compiled schema directory '%s'! Please make sure the parent directory is writeable."),
			   *CompiledSchemaDir);
		return false;
	}

	FString AdditionalSchemaCompilerArgs;

	TArray<FString> Tokens;
	TArray<FString> Switches;
	FCommandLine::Parse(FCommandLine::Get(), Tokens, Switches);

	if (const FString* SchemaCompileArgsCLSwitchPtr = Switches.FindByPredicate([](const FString& ClSwitch) {
			return ClSwitch.StartsWith(FString{ TEXT("AdditionalSchemaCompilerArgs") });
		}))
	{
		FString SwitchName;
		SchemaCompileArgsCLSwitchPtr->Split(FString{ TEXT("=") }, &SwitchName, &AdditionalSchemaCompilerArgs);
		if (AdditionalSchemaCompilerArgs.Contains(FString{ TEXT("ast_proto_out") })
			|| AdditionalSchemaCompilerArgs.Contains(FString{ TEXT("ast_json_out") }))
		{
			if (!PlatformFile.CreateDirectoryTree(*CompiledSchemaASTDir))
			{
				UE_LOG(LogSpatialGDKSchemaGenerator, Error,
					   TEXT("Could not create compiled schema AST directory '%s'! Please make sure the parent directory is writeable."),
					   *CompiledSchemaASTDir);
				return false;
			}
		}
	}

	FString SchemaCompilerArgs = FString::Printf(TEXT("%s %s"), *SchemaCompilerBaseArgs, *AdditionalSchemaCompilerArgs.TrimQuotes());

	UE_LOG(LogSpatialGDKSchemaGenerator, Log, TEXT("Starting '%s' with `%s` arguments."), *SpatialGDKServicesConstants::SchemaCompilerExe,
		   *SchemaCompilerArgs);

	int32 ExitCode = 1;
	FString SchemaCompilerOut;
	FString SchemaCompilerErr;
	FPlatformProcess::ExecProcess(*SpatialGDKServicesConstants::SchemaCompilerExe, *SchemaCompilerArgs, &ExitCode, &SchemaCompilerOut,
								  &SchemaCompilerErr);

	if (ExitCode == 0)
	{
		UE_LOG(LogSpatialGDKSchemaGenerator, Log, TEXT("schema_compiler successfully generated compiled schema with arguments `%s`: %s"),
			   *SchemaCompilerArgs, *SchemaCompilerOut);
		return true;
	}
	else
	{
		UE_LOG(LogSpatialGDKSchemaGenerator, Error, TEXT("schema_compiler failed to generate compiled schema for arguments `%s`: %s"),
			   *SchemaCompilerArgs, *SchemaCompilerErr);
		return false;
	}
}

bool SpatialGDKGenerateSchema()
{
	SchemaGeneratedClasses.Empty();

	// Generate Schema for classes loaded in memory.

	TArray<UObject*> AllClasses;
	GetObjectsOfClass(UClass::StaticClass(), AllClasses);
	if (!SpatialGDKGenerateSchemaForClasses(GetAllSupportedClasses(AllClasses)))
	{
		return false;
	}
	SpatialGDKSanitizeGeneratedSchema();

	GenerateSchemaForSublevels();
	GenerateSchemaForRPCEndpoints();
	GenerateSchemaForNCDs();

	USchemaDatabase* SchemaDatabase = InitialiseSchemaDatabase(SpatialConstants::SCHEMA_DATABASE_ASSET_PATH);

	// Needs to happen before RunSchemaCompiler
	// We construct the list of all server authoritative components while writing the file.
	TArray<Worker_ComponentId> GeneratedServerAuthoritativeComponentIds{};
	WriteServerAuthorityComponentSet(SchemaDatabase, GeneratedServerAuthoritativeComponentIds);
	WriteClientAuthorityComponentSet();
	WriteComponentSetBySchemaType(SchemaDatabase, SCHEMA_Data);
	WriteComponentSetBySchemaType(SchemaDatabase, SCHEMA_OwnerOnly);
	WriteComponentSetBySchemaType(SchemaDatabase, SCHEMA_Handover);

	// Finish initializing the schema database through updating the server authoritative component set.
	for (const auto& ComponentId : GeneratedServerAuthoritativeComponentIds)
	{
		SchemaDatabase->ComponentSetIdToComponentIds.FindOrAdd(SpatialConstants::SERVER_AUTH_COMPONENT_SET_ID)
			.ComponentIDs.Push(ComponentId);
	}

	if (!RunSchemaCompiler())
	{
		return false;
	}

	if (!SaveSchemaDatabase(SchemaDatabase)) // This requires RunSchemaCompiler to run first
	{
		return false;
	}

	return true;
}

bool SpatialGDKGenerateSchemaForClasses(TSet<UClass*> Classes, FString SchemaOutputPath /*= ""*/)
{
	ResetUsedNames();
	Classes.Sort([](const UClass& A, const UClass& B) {
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
		VisitAllObjects(TypeInfo, [&](TSharedPtr<FUnrealType> TypeNode) {
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

	FComponentIdGenerator IdGenerator = FComponentIdGenerator(NextAvailableComponentId);

	GenerateSchemaFromClasses(TypeInfos, SchemaOutputPath, IdGenerator);

	NextAvailableComponentId = IdGenerator.Peek();

	return true;
}

template <class T>
void SanitizeClassMap(TMap<FString, T>& Map, const TSet<FName>& ValidClassNames)
{
	for (auto Item = Map.CreateIterator(); Item; ++Item)
	{
		FString SanitizeName = Item->Key;
		SanitizeName.RemoveFromEnd(TEXT("_C"));
		if (!ValidClassNames.Contains(FName(*SanitizeName)))
		{
			UE_LOG(LogSpatialGDKSchemaGenerator, Log, TEXT("Found stale class (%s), removing from schema database."), *Item->Key);
			Item.RemoveCurrent();
		}
	}
}

void SpatialGDKSanitizeGeneratedSchema()
{
	// Sanitize schema database, removing assets that no longer exist
	FAssetRegistryModule& AssetRegistryModule = FModuleManager::LoadModuleChecked<FAssetRegistryModule>("AssetRegistry");

	TArray<FAssetData> Assets;
	AssetRegistryModule.Get().GetAllAssets(Assets, false);
	TSet<FName> ValidClassNames;
	for (const auto& Asset : Assets)
	{
		ValidClassNames.Add(FName(*Asset.ObjectPath.ToString()));
	}

	TArray<UObject*> AllClasses;
	GetObjectsOfClass(UClass::StaticClass(), AllClasses);
	for (const auto& SupportedClass : GetAllSupportedClasses(AllClasses))
	{
		ValidClassNames.Add(FName(*SupportedClass->GetPathName()));
	}

	SanitizeClassMap(ActorClassPathToSchema, ValidClassNames);
	SanitizeClassMap(SubobjectClassPathToSchema, ValidClassNames);
}

} // namespace Schema
} // namespace SpatialGDKEditor

#undef LOCTEXT_NAMESPACE
