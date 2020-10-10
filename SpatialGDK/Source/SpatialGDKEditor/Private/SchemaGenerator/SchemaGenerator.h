// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "TypeStructure.h"
#include "Utils/SchemaDatabase.h"

using ComponentIdPerType = FComponentId[ESchemaComponentType::SCHEMA_Count];

DECLARE_LOG_CATEGORY_EXTERN(LogSchemaGenerator, Log, All);

class FCodeWriter;
struct FComponentIdGenerator;

extern TArray<UClass*> SchemaGeneratedClasses;
extern TMap<FString, FActorSchemaData> ActorClassPathToSchema;
extern TMap<FString, FSubobjectSchemaData> SubobjectClassPathToSchema;
extern TMap<FString, FComponentId> LevelPathToComponentId;
extern TMap<ESchemaComponentType, TSet<FComponentId>> SchemaComponentTypeToComponents;
extern TMap<float, FComponentId> NetCullDistanceToComponentId;

// Generates schema for an Actor
void GenerateActorSchema(FComponentIdGenerator& IdGenerator, UClass* Class, TSharedPtr<FUnrealType> TypeInfo, FString SchemaPath);
// Generates schema for a Subobject class - the schema type and the dynamic schema components
void GenerateSubobjectSchema(FComponentIdGenerator& IdGenerator, UClass* Class, TSharedPtr<FUnrealType> TypeInfo, FString SchemaPath);

// Generates schema for RPC endpoints.
void GenerateRPCEndpointsSchema(FString SchemaPath);

void AddComponentId(const FComponentId ComponentId, ComponentIdPerType& SchemaComponents, const ESchemaComponentType ComponentType);
