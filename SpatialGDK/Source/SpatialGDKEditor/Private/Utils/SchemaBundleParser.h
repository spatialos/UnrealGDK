// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "SpatialCommonTypes.h"
#include "Utils/SchemaDatabase.h"

DECLARE_LOG_CATEGORY_EXTERN(LogSpatialSchemaBundleParser, Log, All);

namespace SpatialGDK
{
struct SchemaField
{
	FString Name;
	FString Type;
	int32 FieldId;
};

struct SchemaComponentIdentifiers
{
	FString Name;
	Worker_ComponentId ComponentId;
};

struct SchemaComponent
{
	SchemaComponentIdentifiers Id;
	TArray<SchemaField> Fields;
};

bool ExtractInformationFromSchemaJson(const FString& SchemaJsonPath, TMap<uint32, FComponentIDs>& OutComponentSetMap,
									  TMap<uint32, uint32>& OutComponentIdToFieldIdsIndex, TArray<FFieldIDs>& OutFieldIdsArray);

bool ExtractComponentsFromSchemaJson(const FString& SchemaJsonPath, TArray<SchemaComponentIdentifiers>& OutComponents,
									 TSet<FString> const& Files);

bool ExtractComponentsDetailsFromSchemaJson(const FString& SchemaJsonPath, TArray<SchemaComponent>& OutComponents,
											TSet<FString> const& Files);
} // namespace SpatialGDK
