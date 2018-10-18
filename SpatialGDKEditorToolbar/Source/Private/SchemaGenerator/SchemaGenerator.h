// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "EngineMinimal.h"

#include "TypeStructure.h"

class FCodeWriter;
struct FComponentIdGenerator;

//static USchemaDatabase* SchemaDatabase;

// Generates a schema file, given an output code writer, component ID, Unreal type and type info.
int GenerateActorSchema(int ComponentId, UClass* Class, TSharedPtr<FUnrealType> TypeInfo, FString SchemaPath);
void GenerateActorComponentSchema(UClass* Class, TSharedPtr<FUnrealType> TypeInfo, FString SchemaPath);
void GenerateActorComponentSchemaForActor(FComponentIdGenerator& IdGenerator, UClass* ActorClass, TSharedPtr<FUnrealType> TypeInfo, FString SchemaPath);
void GenerateActorComponentSpecificSchema(FCodeWriter& Writer, FComponentIdGenerator& IdGenerator,  FString PropertyName, TSharedPtr<FUnrealType>& TypeInfo);
void GenerateActorIncludes(FCodeWriter& Writer, TSharedPtr<FUnrealType>& TypeInfo);
