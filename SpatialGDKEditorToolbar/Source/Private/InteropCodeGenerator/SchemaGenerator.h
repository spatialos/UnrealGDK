// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "EngineMinimal.h"

#include "TypeStructure.h"

class FCodeWriter;

// Generates a schema file, given an output code writer, component ID, Unreal type and type info.
int GenerateTypeBindingSchema(FCodeWriter& Writer, int ComponentId, UClass* Class, TSharedPtr<FUnrealType> TypeInfo, FString SchemaPath);
