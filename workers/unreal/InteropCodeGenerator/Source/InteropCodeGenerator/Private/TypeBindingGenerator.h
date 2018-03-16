// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "EngineMinimal.h"
#include "TypeStructure.h"

class FCodeWriter;

// Given an Unreal class, generates the name of the type binding class.
// For example: USpatialTypeBinding_Character.
FString TypeBindingName(UClass* Class);

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

void GenerateTypeBindingHeader(
	FCodeWriter& HeaderWriter,
	FString SchemaFilename,
	FString InteropFilename,
	UClass* Class,
	const FPropertyLayout_OLD& Layout);

void GenerateTypeBindingSource(
	FCodeWriter& SourceWriter,
	FString SchemaFilename,
	FString InteropFilename,
	UClass* Class,
	const FPropertyLayout_OLD& Layout);

void GenerateFunction_GetHandlePropertyMap(FCodeWriter& SourceWriter, UClass* Class, const FPropertyLayout_OLD& Layout);
void GenerateFunction_GetBoundClass(FCodeWriter& SourceWriter, UClass* Class);
void GenerateFunction_Init(FCodeWriter& SourceWriter, UClass* Class, const FPropertyLayout_OLD& Layout);
void GenerateFunction_BindToView(FCodeWriter& SourceWriter, UClass* Class, const FPropertyLayout_OLD& Layout);
void GenerateFunction_UnbindFromView(FCodeWriter& SourceWriter, UClass* Class);
void GenerateFunction_GetReplicatedGroupComponentId(FCodeWriter& SourceWriter, UClass* Class, const FPropertyLayout_OLD& Layout);
void GenerateFunction_CreateActorEntity(FCodeWriter& SourceWriter, UClass* Class, const FPropertyLayout_OLD& Layout);
void GenerateFunction_SendComponentUpdates(FCodeWriter& SourceWriter, UClass* Class);
void GenerateFunction_SendRPCCommand(FCodeWriter& SourceWriter, UClass* Class);
void GenerateFunction_ReceiveAddComponent(FCodeWriter& SourceWriter, UClass* Class);
void GenerateFunction_ApplyQueuedStateToChannel(FCodeWriter& SourceWriter, UClass* Class);
void GenerateFunction_BuildSpatialComponentUpdate(FCodeWriter& SourceWriter, UClass* Class);
void GenerateFunction_ServerSendUpdate(FCodeWriter& SourceWriter, UClass* Class, const FPropertyLayout_OLD& Layout, EReplicatedPropertyGroup Group);
void GenerateFunction_ReceiveUpdate(FCodeWriter& SourceWriter, UClass* Class, const FPropertyLayout_OLD& Layout, EReplicatedPropertyGroup Group);
void GenerateFunction_RPCSendCommand(FCodeWriter& SourceWriter, UClass* Class, const FRPCDefinition_OLD& RPC);
void GenerateFunction_RPCOnCommandRequest(FCodeWriter& SourceWriter, UClass* Class, const FRPCDefinition_OLD& RPC);
void GenerateFunction_RPCOnCommandResponse(FCodeWriter& SourceWriter, UClass* Class, const FRPCDefinition_OLD& RPC);
