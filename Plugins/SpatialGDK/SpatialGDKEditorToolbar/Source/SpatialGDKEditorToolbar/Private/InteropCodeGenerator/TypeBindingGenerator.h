// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "EngineMinimal.h"
#include "TypeStructure.h"

class FCodeWriter;

// Given an Unreal class, generates the name of the type binding class.
// For example: USpatialTypeBinding_Character.
FString TypeBindingName(UClass* Class);

// Generates code to copy an Unreal 'PropertyValue' and write it to a SpatialOS
// component update
// object 'Update'.
void GenerateUnrealToSchemaConversion(FCodeWriter& Writer, const FString& Update, const UProperty* Property,
									  const FString& PropertyValue, const bool bIsUpdate,
									  TFunction<void(const FString&)> ObjectResolveFailureGenerator);

// Generates code to extract property data from a SpatialOS component update
// object and write it to
// an Unreal 'PropertyValue'
void GeneratePropertyToUnrealConversion(FCodeWriter& Writer, const FString& Update, const UProperty* Property,
										const FString& PropertyValue, const bool bIsUpdate,
										TFunction<void(const FString&)> ObjectResolveFailureGenerator);

// Generates the appropriate macro for Unreal to read a property from an FFrame
// in scope. This is
// the same method that
// Unreal uses in .generated.h files.
FString GenerateFFramePropertyReader(UProperty* Property);

// Generates the header of a type binding.
void GenerateTypeBindingHeader(FCodeWriter& HeaderWriter, FString SchemaFilename, FString InteropFilename,
							   UClass* Class, const TSharedPtr<FUnrealType> TypeInfo);

// Generates the source file of a type binding.
void GenerateTypeBindingSource(FCodeWriter& SourceWriter, FString SchemaFilename, FString InteropFilename,
							   UClass* Class, const TSharedPtr<FUnrealType> TypeInfo);

// Helper functions used when generating the source file.
void GenerateFunction_GetRepHandlePropertyMap(FCodeWriter& SourceWriter, UClass* Class,
											  const FUnrealFlatRepData& RepData);
void GenerateFunction_GetMigratableHandlePropertyMap(FCodeWriter& SourceWriter, UClass* Class,
													 const TMap<uint16, TSharedPtr<FUnrealProperty>>& MigratableData);
void GenerateFunction_GetBoundClass(FCodeWriter& SourceWriter, UClass* Class);
void GenerateFunction_Init(FCodeWriter& SourceWriter, UClass* Class, const FUnrealRPCsByType& RPCsByType);
void GenerateFunction_BindToView(FCodeWriter& SourceWriter, UClass* Class, const FUnrealRPCsByType& RPCsByType);
void GenerateFunction_UnbindFromView(FCodeWriter& SourceWriter, UClass* Class);
void GenerateFunction_CreateActorEntity(FCodeWriter& SourceWriter, UClass* Class);
void GenerateFunction_SendComponentUpdates(FCodeWriter& SourceWriter, UClass* Class);
void GenerateFunction_SendRPCCommand(FCodeWriter& SourceWriter, UClass* Class);
void GenerateFunction_ReceiveAddComponent(FCodeWriter& SourceWriter, UClass* Class);
void GenerateFunction_GetInterestOverrideMap(FCodeWriter& SourceWriter, UClass* Class);
void GenerateFunction_BuildSpatialComponentUpdate(FCodeWriter& SourceWriter, UClass* Class);
void GenerateFunction_ServerSendUpdate_RepData(FCodeWriter& SourceWriter, UClass* Class,
											   const FUnrealFlatRepData& RepData, EReplicatedPropertyGroup Group);
void GenerateFunction_ServerSendUpdate_MigratableData(FCodeWriter& SourceWriter, UClass* Class,
													  const TMap<uint16, TSharedPtr<FUnrealProperty>>& MigratableData);
void GenerateFunction_ReceiveUpdate_RepData(FCodeWriter& SourceWriter, UClass* Class, const FUnrealFlatRepData& RepData,
											EReplicatedPropertyGroup Group);
void GenerateFunction_ReceiveUpdate_MigratableData(FCodeWriter& SourceWriter, UClass* Class,
												   const TMap<uint16, TSharedPtr<FUnrealProperty>>& MigratableData);
void GenerateFunction_RPCSendCommand(FCodeWriter& SourceWriter, UClass* Class, const TSharedPtr<FUnrealRPC> RPC);
void GenerateFunction_RPCOnCommandRequest(FCodeWriter& SourceWriter, UClass* Class, const TSharedPtr<FUnrealRPC> RPC);
void GenerateFunction_RPCOnCommandResponse(FCodeWriter& SourceWriter, UClass* Class, const TSharedPtr<FUnrealRPC> RPC);
