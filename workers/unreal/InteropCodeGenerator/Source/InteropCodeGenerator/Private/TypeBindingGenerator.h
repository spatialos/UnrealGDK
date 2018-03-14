// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "EngineMinimal.h"

struct FPropertyLayout;
class FCodeWriter;

// Generates code to copy an Unreal PropertyValue into a SpatialOS component update.
void GenerateUnrealToSchemaConversion(
	FCodeWriter& Writer,
	const FString& Update,
	TArray<UProperty*> PropertyChain,
	const FString& PropertyValue,
	const bool bIsUpdate,
	TFunction<void(const FString&)> ObjectResolveFailureGenerator);

// Given a 'chained' list of UProperties, this class will generate the code to extract values from flattened schema and apply them to the corresponding Unreal data structure 
void GeneratePropertyToUnrealConversion(
	FCodeWriter& Writer,
	const FString& Update,
	TArray<UProperty*> PropertyChain,
	const FString& PropertyValue,
	const bool bIsUpdate,
	const FString& PropertyType,
	TFunction<void(const FString&)> ObjectResolveFailureGenerator);

FString GeneratePropertyReader(UProperty* Property);


void GenerateFunction_SubobjectNameToOffsetMap(
	FCodeWriter& SourceWriter,
	FString SchemaFilename,
	FString InteropFilename,
	UClass* Class,
	const FPropertyLayout& Layout,
	FString& TypeBindingName);

void GenerateForwardingCodeFromLayout(
	FCodeWriter& HeaderWriter,
	FCodeWriter& SourceWriter,
	FString SchemaFilename,
	FString InteropFilename,
	UClass* Class,
	const FPropertyLayout& Layout);