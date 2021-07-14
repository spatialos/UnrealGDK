// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "SpatialCommonTypes.h"
#include "Utils/SchemaDatabase.h"

DECLARE_LOG_CATEGORY_EXTERN(LogSpatialSchemaBundleParser, Log, All);

namespace SpatialGDK
{
struct SchemaComponentIdentifiers
{
	FString Name;
	Worker_ComponentId ComponentId;
};

bool ExtractInformationFromSchemaJson(const FString& SchemaJsonPath, TMap<uint32, FComponentIDs>& OutComponentSetMap,
									  TMap<uint32, uint32>& OutComponentIdToFieldIdsIndex, TArray<FFieldIDs>& OutFieldIdsArray, TArray<FFieldIDs>& OutListIdsArray);

bool ExtractComponentsFromSchemaJson(const FString& SchemaJsonPath, TArray<SchemaComponentIdentifiers>& OutComponents,
									 const TSet<FString>& Files);
} // namespace SpatialGDK
