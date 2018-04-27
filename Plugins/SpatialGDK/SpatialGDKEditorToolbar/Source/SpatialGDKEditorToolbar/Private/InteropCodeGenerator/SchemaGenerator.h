// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "EngineMinimal.h"
#include "Net/RepLayout.h"

#include "TypeStructure.h"

class FCodeWriter;

// Given a class or function name, generates the name used for naming schema
// components and types.
// Currently, this just removes underscores.
FString UnrealNameToSchemaTypeName(const FString& UnrealName);

// Given a replicated property group and Unreal type, generates the name of the
// corresponding
// component.
// For example: UnrealCharacterMultiClientRepData
FString SchemaReplicatedDataName(EReplicatedPropertyGroup Group, UStruct* Type);

// Given an unreal type, generates the name of the component which stores worker
// to worker
// replication data.
// For example: UnrealCharacterWorkerRepData
FString SchemaMigratableDataName(UStruct* Type);

// Given an RPC type and Unreal type, generates the name of the corresponding
// RPC container
// component.
// For example: UnrealCharacterClientRPCs
FString SchemaRPCComponentName(ERPCType RpcType, UStruct* Type);

// Given a UFunction, generates the command request data type.
// For example: UnrealServerMoveRequest.
FString SchemaRPCRequestType(UFunction* Function);

// Given a UFunction, generates the command request data type.
// For example: UnrealServerMoveResponse.
FString SchemaRPCResponseType(UFunction* Function);

// Given a property node, generates the schema field name.
FString SchemaFieldName(const TSharedPtr<FUnrealProperty> Property);

// Given a UFunction, generates the schema command name. Currently just returns
// the function name in
// lowercase.
FString SchemaCommandName(UFunction* Function);

// Given a UFunction, generates the c++ command name. Identical to the schema
// name with the first
// letter being uppercase.
FString CPPCommandClassName(UFunction* Function);

// Given a RepLayout cmd type (a data type supported by the replication system).
// Generates the
// corresponding
// type used in schema.
FString PropertyToSchemaType(UProperty* Property);

// Generates a schema file, given an output code writer, component ID, Unreal
// type and type info.
int GenerateTypeBindingSchema(FCodeWriter& Writer, int ComponentId, UClass* Class, TSharedPtr<FUnrealType> TypeInfo, FString SchemaPath);
