// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "TypeStructure.h"
#include "Utils/SchemaDatabase.h"

using ComponentIdPerType = Worker_ComponentId[ESchemaComponentType::SCHEMA_Count];

DECLARE_LOG_CATEGORY_EXTERN(LogSchemaGenerator, Log, All);

class FCodeWriter;
struct FComponentIdGenerator;

extern TArray<UClass*> SchemaGeneratedClasses;
extern TMap<FString, FActorSchemaData> ActorClassPathToSchema;
extern TMap<FString, FSubobjectSchemaData> SubobjectClassPathToSchema;
extern TMap<FString, Worker_ComponentId> LevelPathToComponentId;
extern TMap<ESchemaComponentType, TSet<Worker_ComponentId>> SchemaComponentTypeToComponents;
extern TMap<float, Worker_ComponentId> NetCullDistanceToComponentId;
extern TMap<Worker_ComponentId, FComponentSchemaData> ComponentIdToFieldIds;

extern TMap<UClass*, TMap<EReplicatedPropertyGroup, TSet<Schema_FieldId>>> FieldIdsForComponentClassAndGroup;
extern TMap<UClass*, TSet<Schema_FieldId>> FieldIdsForHandoverComponentClass;
extern TMap<TPair<UClass*, EReplicatedPropertyGroup>, TArray<Worker_ComponentId>> ComponentIdsWaitingForSubobjects;
extern TMap<UClass*, TArray<Worker_ComponentId>> ComponentIdsWaitingForHandoverSubobjects;

// Generates schema for an Actor
void GenerateActorSchema(FComponentIdGenerator& IdGenerator, UClass* Class, TSharedPtr<FUnrealType> TypeInfo, FString SchemaPath);
// Generates schema for a Subobject class - the schema type and the dynamic schema components
void GenerateSubobjectSchema(FComponentIdGenerator& IdGenerator, UClass* Class, TSharedPtr<FUnrealType> TypeInfo, FString SchemaPath);

// Generates schema for RPC endpoints.
void GenerateRPCEndpointsSchema(FString SchemaPath);

void AddComponentId(const Worker_ComponentId ComponentId, ComponentIdPerType& SchemaComponents, const ESchemaComponentType ComponentType);
