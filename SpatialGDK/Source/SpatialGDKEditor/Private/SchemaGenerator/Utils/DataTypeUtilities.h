// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "CoreMinimal.h"

#include "SchemaGenerator/TypeStructure.h"
#include "Utils/GDKPropertyMacros.h"

// Return the string representation of the underlying data type of an enum property
FString GetEnumDataType(const GDK_PROPERTY(EnumProperty) * EnumProperty);

FString PropertyToSchemaType(const GDK_PROPERTY(Property) * Property);

// Given a class or function name, generates the name used for naming schema components and types. Removes all non-alphanumeric characters.
FString UnrealNameToSchemaName(const FString& UnrealName, bool bWarnAboutRename = false);

// Given an object name, generates the name used for naming schema components. Removes all non-alphanumeric characters and capitalizes the
// first letter.
FString UnrealNameToSchemaComponentName(const FString& UnrealName);

// Given a replicated property group and Unreal type, generates the name of the corresponding schema component.
// For example: UnrealCharacterMultiClientRepData
FString SchemaReplicatedDataName(TMap<FString, FString> const& ClassPathToSchemaName, EReplicatedPropertyGroup Group,
								 FString const& ClassPath);

// Given an unreal type, generates the name of the component which stores server to server replication data.
// For example: UnrealCharacterHandoverData
FString SchemaHandoverDataName(TMap<FString, FString> const& ClassPathToSchemaName, FString const& ClassPath);

// Given a property node, generates the schema field name.
FString SchemaFieldName(const TSharedPtr<FUnrealOfflineProperty> Property);

FString AlphanumericSanitization(const FString& InString);
