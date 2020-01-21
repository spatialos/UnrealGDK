// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "TypeStructure.h"
#include "Utils/SchemaDatabase.h"

DECLARE_LOG_CATEGORY_EXTERN(LogSchemaGenerator, Log, All);

class FCodeWriter;
struct FComponentIdGenerator;

extern TArray<UClass*> SchemaGeneratedClasses;
extern TMap<FString, FActorSchemaData> ActorClassPathToSchema;
extern TMap<FString, FSubobjectSchemaData> SubobjectClassPathToSchema;
extern TMap<FString, uint32> LevelPathToComponentId;
extern TMap<ESchemaComponentType, TSet<uint32>> SchemaComponentTypeToComponentSet;

// Generates schema for an Actor
void GenerateActorSchema(FComponentIdGenerator& IdGenerator, UClass* Class, TSharedPtr<FUnrealType> TypeInfo, FString SchemaPath);
// Generates schema for a Subobject class - the schema type and the dynamic schema components
void GenerateSubobjectSchema(FComponentIdGenerator& IdGenerator, UClass* Class, TSharedPtr<FUnrealType> TypeInfo, FString SchemaPath);

// Generates schema for RPC endpoints.
void GenerateRPCEndpointsSchema(FString SchemaPath);

void AddComponentId(uint32 ComponentId, uint32 (&SchemaComponents)[ESchemaComponentType::SCHEMA_Count], ESchemaComponentType ComponentType);
