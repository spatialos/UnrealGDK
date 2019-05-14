// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "TypeStructure.h"
#include "Utils/SchemaDatabase.h"

DECLARE_LOG_CATEGORY_EXTERN(LogSchemaGenerator, Log, All);

class FCodeWriter;
struct FComponentIdGenerator;

extern TArray<UClass*> SchemaGeneratedClasses;
extern TMap<FString, FSchemaData> ClassPathToSchemaData;
extern TMap<FString, uint32> LevelPathToComponentId;

// Generates a schema file, given an output code writer, component ID, Unreal type and type info.
void GenerateActorSchema(FComponentIdGenerator& IdGenerator, UClass* Class, TSharedPtr<FUnrealType> TypeInfo, FString SchemaPath);
void GenerateSubobjectSchema(UClass* Class, TSharedPtr<FUnrealType> TypeInfo, FString SchemaPath);
void GenerateSubobjectSchemaForActor(FComponentIdGenerator& IdGenerator, UClass* ActorClass, TSharedPtr<FUnrealType> TypeInfo, FString SchemaPath, FSchemaData& ActorSchemaData);
FSubobjectSchemaData GenerateSubobjectSpecificSchema(FCodeWriter& Writer, FComponentIdGenerator& IdGenerator, FString PropertyName, TSharedPtr<FUnrealType>& TypeInfo, UClass* ComponentClass);
void GenerateActorIncludes(FCodeWriter& Writer, TSharedPtr<FUnrealType>& TypeInfo);
