// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "CoreMinimal.h"

#include "SchemaGenerator/TypeStructure.h"

extern TMap<FString, FString> ClassPathToSchemaName;

// Return the string representation of the underlying data type of an enum property
FString GetEnumDataType(const UEnumProperty* EnumProperty);

// Given a class or function name, generates the name used for naming schema components and types. Removes all non-alphanumeric characters.
FString UnrealNameToSchemaName(const FString& UnrealName);

// Given an object name, generates the name used for naming schema components. Removes all non-alphanumeric characters and capitalizes the first letter.
FString UnrealNameToSchemaComponentName(const FString& UnrealName);

// Given a replicated property group and Unreal type, generates the name of the corresponding schema component.
// For example: UnrealCharacterMultiClientRepData
FString SchemaReplicatedDataName(EReplicatedPropertyGroup Group, UClass* Class);

// Given an unreal type, generates the name of the component which stores server to server replication data.
// For example: UnrealCharacterHandoverData
FString SchemaHandoverDataName(UClass* Class);

// Given a UFunction, generates the schema command name. Currently just returns the function name in lowercase.
FString SchemaRPCName(UFunction* Function);

// Given a property node, generates the schema field name.
FString SchemaFieldName(const TSharedPtr<FUnrealProperty> Property);

FString AlphanumericSanitization(const FString& InString);
