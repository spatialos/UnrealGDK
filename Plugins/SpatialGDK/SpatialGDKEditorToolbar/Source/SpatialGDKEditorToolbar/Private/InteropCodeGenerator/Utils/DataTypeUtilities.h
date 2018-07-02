// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "CoreMinimal.h"

#include "InteropCodeGenerator/TypeStructure.h"

// Return the string representation of the underlying data type of an enum property
FString GetEnumDataType(const UEnumProperty* EnumProperty);

// Given a class or function name, generates the name used for naming schema components and types. Currently, this just removes underscores.
FString UnrealNameToSchemaTypeName(const FString& UnrealName);

// Given a replicated property group and Unreal type, generates the name of the corresponding schema component.
// For example: UnrealCharacterMultiClientRepData
FString SchemaReplicatedDataName(EReplicatedPropertyGroup Group, UStruct* Type, bool bPrependNamespace = false);

// Given an unreal type, generates the name of the component which stores worker to worker replication data.
// For example: UnrealCharacterMigratableData
FString SchemaMigratableDataName(UStruct* Type, bool bPrependNamespace = false);

// Given an RPC type and Unreal type, generates the name of the corresponding RPC container component.
// For example: UnrealCharacterClientRPCs
FString SchemaRPCComponentName(ERPCType RpcType, UStruct* Type, bool bPrependNamespace = false);

// Given a UFunction, generates the command request data type.
// For example: ServerMove() -> UnrealServerMoveRequest.
FString SchemaRPCRequestType(UFunction* Function, bool bPrependNamespace = false);

// Given a UFunction, generates the command request data type.
// For example: ServerMove() -> UnrealServerMoveResponse.
FString SchemaRPCResponseType(UFunction* Function);

// Given a UFunction, generates the schema command name. Currently just returns the function name in lowercase.
FString SchemaRPCName(UClass* Class, UFunction* Function);

// Given a UFunction, generates the c++ command name. Identical to the schema name with the first letter being uppercase.
FString CPPCommandClassName(UClass* Class, UFunction* Function);

// Given a property node, generates the schema field name.
FString SchemaFieldName(const TSharedPtr<FUnrealProperty> Property);
