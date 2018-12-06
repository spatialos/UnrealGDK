// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "CoreMinimal.h"

#include "SchemaGenerator/TypeStructure.h"

// Return the string representation of the underlying data type of an enum property
FString GetEnumDataType(const UEnumProperty* EnumProperty);

// Given a function name, generates the name used for naming schema commands.
// Converts all non-alphanumeric characters to hex code strings.
FString UnrealFunctionNameToSchemaName(const FString& UnrealName);

// Given a class name, generates the name used for naming schema classes.
// Removes all underscores. Any other non-alphanumeric characters are converted to hex code strings.
FString UnrealClassNameToSchemaName(const FString& UnrealName);

// Given an object name, generates the name used for naming schema components.
// Removes all underscores. Any other non-alphanumeric characters are converted to hex code strings and capitalizes the first letter.
FString UnrealNameToSchemaComponentName(const FString& UnrealName);

// Given a replicated property group and Unreal type, generates the name of the corresponding schema component.
// For example: UnrealCharacterMultiClientRepData
FString SchemaReplicatedDataName(EReplicatedPropertyGroup Group, UStruct* Type, bool bPrependNamespace = false);

// Given an unreal type, generates the name of the component which stores server to server replication data.
// For example: UnrealCharacterHandoverData
FString SchemaHandoverDataName(UStruct* Type, bool bPrependNamespace = false);

// Given an RPC type and Unreal type, generates the name of the corresponding RPC container component.
// For example: UnrealCharacterClientRPCs
FString SchemaRPCComponentName(ERPCType RpcType, UStruct* Type, bool bPrependNamespace = false);

// Given a UFunction, generates the schema command name. Currently just returns the function name in lowercase.
FString SchemaRPCName(UFunction* Function);

// Given a property node, generates the schema field name.
FString SchemaFieldName(const TSharedPtr<FUnrealProperty> Property);

// Converts all non-alphanumeric characters in the InString to hex code strings.
FString NonAlphanumericAsciiCharacterConverter(const FString& InString);

// Takes an ASCII character code and returns a hex code string.
FString ConvertAsciiCodeToHexString(int ASCIICode);

// Returns a string with all non-alphanumeric characters removed from the InString.
FString AlphanumericSanitization(const FString& InString);
