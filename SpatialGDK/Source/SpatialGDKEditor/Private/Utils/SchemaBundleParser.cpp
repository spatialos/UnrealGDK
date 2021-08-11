// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "Utils/SchemaBundleParser.h"

#include "Serialization/JsonReader.h"
#include "Serialization/JsonSerializer.h"

DEFINE_LOG_CATEGORY(LogSpatialSchemaBundleParser)

// clang-format off
#define SAFE_TRYGET(Value, Type, OutParam)                                                                                                 \
	do                                                                                                                                     \
	{                                                                                                                                      \
		if (!Value->TryGet##Type(OutParam))                                                                                                \
		{                                                                                                                                  \
			UE_LOG(LogSpatialSchemaBundleParser, Error, TEXT("Failed to get %s as type %s"), TEXT(#Value), TEXT(#Type));                   \
			return false;                                                                                                                  \
		}                                                                                                                                  \
	} while (false)

#define SAFE_TRYGETFIELD(Value, Type, FieldName, OutParam)                                                                                 \
	do                                                                                                                                     \
	{                                                                                                                                      \
		if (!Value->TryGet##Type##Field(TEXT(FieldName), OutParam))                                                                        \
		{                                                                                                                                  \
			UE_LOG(LogSpatialSchemaBundleParser, Error, TEXT("Failed to get field %s of type %s from %s"), TEXT(FieldName), TEXT(#Type), TEXT(#Value)); \
			return false;                                                                                                                  \
		}                                                                                                                                  \
	} while (false)

#define COND_SCHEMA_GEN_ERROR_AND_RETURN(Condition, Format, ...)                                                                           \
	if (UNLIKELY(Condition))                                                                                                               \
	{                                                                                                                                      \
		UE_LOG(LogSpatialSchemaBundleParser, Error, Format, ##__VA_ARGS__);                                                                \
		return false;                                                                                                                      \
	}
// clang-format on

namespace
{
TSharedPtr<FJsonObject> OpenJsonFile(const FString& SchemaJsonPath)
{
	TUniquePtr<FArchive> SchemaFile(IFileManager::Get().CreateFileReader(*SchemaJsonPath));
	if (!SchemaFile)
	{
		UE_LOG(LogSpatialSchemaBundleParser, Error, TEXT("Could not open schema bundle file %s"), *SchemaJsonPath);
		return TSharedPtr<FJsonObject>();
	}

	TSharedPtr<FJsonValue> SchemaBundleJson;
	{
		TSharedRef<TJsonReader<char>> JsonReader = TJsonReader<char>::Create(SchemaFile.Get());
		FJsonSerializer::Deserialize(*JsonReader, SchemaBundleJson);
	}

	const TSharedPtr<FJsonObject>* RootObject;
	if (!SchemaBundleJson || !SchemaBundleJson->TryGetObject(RootObject))
	{
		UE_LOG(LogSpatialSchemaBundleParser, Error, TEXT("%s is not a valid Json file"), *SchemaJsonPath);
		return TSharedPtr<FJsonObject>();
	}

	return *RootObject;
}
} // namespace

namespace SpatialGDK
{
void AddToListIfClearable(const TSharedPtr<FJsonObject>* ArrayObject, int32 FieldId, TArray<uint32>& OutIDs)
{
	const TSharedPtr<FJsonObject>* JsonObject;
	// List, options and maps are all clearable, but we can only generate
	// listTypes, for the moment at least.
	if ((*ArrayObject)->TryGetObjectField(TEXT("listType"), JsonObject))
	{
		OutIDs.Add(FieldId);
	}
}

bool ExtractInformationFromSchemaJson(const FString& SchemaJsonPath, TMap<uint32, FComponentIDs>& OutComponentSetMap,
									  TMap<uint32, uint32>& OutComponentIdToFieldIdsIndex, TArray<FFieldIDs>& OutFieldIdsArray,
									  TArray<FFieldIDs>& OutListIdsArray)
{
	TSharedPtr<FJsonObject> RootObject = OpenJsonFile(SchemaJsonPath);

	const TArray<TSharedPtr<FJsonValue>>* SchemaFiles;
	SAFE_TRYGETFIELD(RootObject, Array, "schemaFiles", SchemaFiles);

	TMap<FString, uint32> ComponentMap;
	TMap<uint32, TSet<FString>> ComponentRefSetMap;

	TMap<FString, uint32> DataDefinitionNameToFieldIdsIndex;
	TMap<uint32, FString> ComponentIdToDataDefinitionName;

	for (const auto& FileValue : *SchemaFiles)
	{
		const TSharedPtr<FJsonObject>* FileObject;
		SAFE_TRYGET(FileValue, Object, FileObject);

		const TArray<TSharedPtr<FJsonValue>>* TypesDecl;
		SAFE_TRYGETFIELD((*FileObject), Array, "types", TypesDecl);

		for (const auto& TypeValue : *TypesDecl)
		{
			const TSharedPtr<FJsonObject>* TypeObject;
			SAFE_TRYGET(TypeValue, Object, TypeObject);

			FString ComponentName;
			SAFE_TRYGETFIELD((*TypeObject), String, "qualifiedName", ComponentName);

			COND_SCHEMA_GEN_ERROR_AND_RETURN(DataDefinitionNameToFieldIdsIndex.Contains(ComponentName),
											 TEXT("The schema bundle contains duplicate data definitions for %s."), *ComponentName);
			DataDefinitionNameToFieldIdsIndex.Add(ComponentName, OutFieldIdsArray.Num());
			TArray<uint32>& FieldIDs = OutFieldIdsArray.AddDefaulted_GetRef().FieldIds;
			TArray<uint32>& ListIDs = OutListIdsArray.AddDefaulted_GetRef().FieldIds;

			const TArray<TSharedPtr<FJsonValue>>* FieldArray;
			SAFE_TRYGETFIELD((*TypeObject), Array, "fields", FieldArray);

			for (const auto& ArrayValue : *FieldArray)
			{
				const TSharedPtr<FJsonObject>* ArrayObject;
				SAFE_TRYGET(ArrayValue, Object, ArrayObject);

				int32 FieldId;
				SAFE_TRYGETFIELD((*ArrayObject), Number, "fieldId", FieldId);

				COND_SCHEMA_GEN_ERROR_AND_RETURN(FieldIDs.Contains(FieldId),
												 TEXT("The schema bundle contains duplicate fieldId: %d, component name: %s."), FieldId,
												 *ComponentName);
				FieldIDs.Add(FieldId);

				AddToListIfClearable(ArrayObject, FieldId, ListIDs);
			}
		}

		const TArray<TSharedPtr<FJsonValue>>* ComponentsDecl;
		SAFE_TRYGETFIELD((*FileObject), Array, "components", ComponentsDecl);

		for (const auto& CompValue : *ComponentsDecl)
		{
			const TSharedPtr<FJsonObject>* CompObject;
			SAFE_TRYGET(CompValue, Object, CompObject);

			FString ComponentName;
			SAFE_TRYGETFIELD((*CompObject), String, "qualifiedName", ComponentName);

			int32 ComponentId;
			SAFE_TRYGETFIELD((*CompObject), Number, "componentId", ComponentId);

			ComponentMap.Add(ComponentName, ComponentId);

			const TArray<TSharedPtr<FJsonValue>>* FieldArray;
			SAFE_TRYGETFIELD((*CompObject), Array, "fields", FieldArray);

			if (FieldArray->Num() > 0)
			{
				COND_SCHEMA_GEN_ERROR_AND_RETURN(OutComponentIdToFieldIdsIndex.Contains(ComponentId),
												 TEXT("The schema bundle contains duplicate component IDs with component %s."),
												 *ComponentName);
				OutComponentIdToFieldIdsIndex.Add(ComponentId, OutFieldIdsArray.Num());
				TArray<uint32>& FieldIDs = OutFieldIdsArray.AddDefaulted_GetRef().FieldIds;
				TArray<uint32>& ListIDs = OutListIdsArray.AddDefaulted_GetRef().FieldIds;

				for (const auto& ArrayValue : *FieldArray)
				{
					const TSharedPtr<FJsonObject>* ArrayObject;
					SAFE_TRYGET(ArrayValue, Object, ArrayObject);

					int32 FieldId;
					SAFE_TRYGETFIELD((*ArrayObject), Number, "fieldId", FieldId);

					COND_SCHEMA_GEN_ERROR_AND_RETURN(FieldIDs.Contains(FieldId),
													 TEXT("The schema bundle contains duplicate fieldId: %d, component name: %s."), FieldId,
													 *ComponentName);
					FieldIDs.Add(FieldId);

					AddToListIfClearable(ArrayObject, FieldId, ListIDs);
				}
			}

			FString DataDefinition;
			SAFE_TRYGETFIELD((*CompObject), String, "dataDefinition", DataDefinition);

			if (!DataDefinition.IsEmpty())
			{
				COND_SCHEMA_GEN_ERROR_AND_RETURN(
					FieldArray->Num() != 0,
					TEXT("The schema bundle supplied both a data definition and field IDs - this is unexpected, component name: %s."),
					*ComponentName);
				ComponentIdToDataDefinitionName.Add(ComponentId, DataDefinition);
			}
		}

		const TArray<TSharedPtr<FJsonValue>>* ComponentSets;
		SAFE_TRYGETFIELD((*FileObject), Array, "componentSets", ComponentSets);

		for (const auto& CompSetValue : *ComponentSets)
		{
			const TSharedPtr<FJsonObject>* CompSetObject;
			SAFE_TRYGET(CompSetValue, Object, CompSetObject);

			int32 ComponentSetId;
			SAFE_TRYGETFIELD((*CompSetObject), Number, "componentSetId", ComponentSetId);

			const TSharedPtr<FJsonObject>* CompListObject;
			SAFE_TRYGETFIELD((*CompSetObject), Object, "componentList", CompListObject);

			const TArray<TSharedPtr<FJsonValue>>* RefComponents;
			SAFE_TRYGETFIELD((*CompListObject), Array, "components", RefComponents);

			TSet<FString> Components;

			for (const auto& CompRefValue : *RefComponents)
			{
				const TSharedPtr<FJsonObject>* CompRefObject;
				SAFE_TRYGET(CompRefValue, Object, CompRefObject);

				FString ComponentName;
				SAFE_TRYGETFIELD((*CompRefObject), String, "component", ComponentName);

				Components.Add(ComponentName);
			}

			ComponentRefSetMap.Add(ComponentSetId, MoveTemp(Components));
		}
	}

	TMap<uint32, FComponentIDs> FinalMap;

	for (const auto& SetEntry : ComponentRefSetMap)
	{
		const TSet<FString>& ComponentRefs = SetEntry.Value;

		FComponentIDs SetIds;
		for (const auto& CompRef : ComponentRefs)
		{
			uint32* FoundId = ComponentMap.Find(CompRef);
			COND_SCHEMA_GEN_ERROR_AND_RETURN(FoundId == nullptr, TEXT("Schema file %s is missing a component entry for %s"),
											 *SchemaJsonPath, *CompRef);
			SetIds.ComponentIDs.Add(*FoundId);
		}

		FinalMap.Add(SetEntry.Key, MoveTemp(SetIds));
	}

	for (const auto& Pair : ComponentIdToDataDefinitionName)
	{
		COND_SCHEMA_GEN_ERROR_AND_RETURN(
			!DataDefinitionNameToFieldIdsIndex.Contains(Pair.Value),
			TEXT("The schema bundle did not contain a data definition for component ID %d, data definition name: %s."), Pair.Key,
			*Pair.Value);
		OutComponentIdToFieldIdsIndex.Add(Pair.Key, DataDefinitionNameToFieldIdsIndex[Pair.Value]);
	}

	OutComponentSetMap = MoveTemp(FinalMap);

	return true;
}

bool ExtractComponentsFromSchemaJson(const FString& SchemaJsonPath, TArray<SchemaComponentIdentifiers>& OutComponents,
									 const TSet<FString>& Files)
{
	TSharedPtr<FJsonObject> RootObject = OpenJsonFile(SchemaJsonPath);

	const TArray<TSharedPtr<FJsonValue>>* SchemaFiles;
	SAFE_TRYGETFIELD(RootObject, Array, "schemaFiles", SchemaFiles);

	for (const auto& FileValue : *SchemaFiles)
	{
		const TSharedPtr<FJsonObject>* FileObject;
		SAFE_TRYGET(FileValue, Object, FileObject);

		FString CanonicalPath;
		SAFE_TRYGETFIELD((*FileObject), String, "canonicalPath", CanonicalPath);
		if (!Files.Contains(CanonicalPath))
		{
			continue;
		}

		const TArray<TSharedPtr<FJsonValue>>* ComponentsDecl;
		SAFE_TRYGETFIELD((*FileObject), Array, "components", ComponentsDecl);

		for (const auto& CompValue : *ComponentsDecl)
		{
			const TSharedPtr<FJsonObject>* CompObject;
			SAFE_TRYGET(CompValue, Object, CompObject);

			SchemaComponentIdentifiers Ids;

			SAFE_TRYGETFIELD((*CompObject), String, "qualifiedName", Ids.Name);
			SAFE_TRYGETFIELD((*CompObject), Number, "componentId", Ids.ComponentId);

			OutComponents.Add(Ids);
		}
	}

	return true;
}

} // namespace SpatialGDK
