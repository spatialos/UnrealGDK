// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "EngineMinimal.h"
#include "Net/RepLayout.h"

/*

This file contains functions to generate an abstract syntax tree which is used
by the code
generating in
SchemaGenerator.cpp and TypeBindingGenerator.cpp. The AST follows a structure
which is similar to a
UClass/UProperty
structure, but also contains additional metadata such as replication data. One
main difference is
that the AST
structure will recurse into object properties if it's determined that the
container type holds a
strong reference
to that subobject (such as a character owning its movement component).

The AST is built by recursing through the UClass/UProperty structure generated
by UHT. Afterwards,
the rep layout
is generated with FRepLayout::InitFromObjectClass, and the replication handle /
etc is stored in
FUnrealRepData
and transposed into the correct part of the AST.

An example AST generated from ACharacter:

FUnrealType
  + Type: Character
  + Properties:
		[0] FUnrealProperty
		  + Property: "MovementComp"
		  + Type: FUnrealType
				+ Type: CharacterMovementComponent
				+ Properties:
				  [0] ....
				  ...
				+ RPCs:
				  [0] FUnrealRPC
						+ CallerType: CharacterMovementComponent
						+ Function: "ServerMove"
						+ Type: RPC_Server
						+ bReliable: false
						+ Parameters:
						  [0] FUnrealProperty
						  ...
				  [1] ....
		  + ReplicationData: FUnrealRepData
				+ RepLayoutType: REPCMD_PropertyObject
				+ Handle: 29
				+ ...
		[1] FUnrealProperty
		  + Property: "bIsCrouched"
		  + Type: nullptr
		  + ReplicationData: FUnrealRepData
				+ RepLayoutType: REPCMD_PropertyBool
				+ Handle: 15
				...
		[2] FUnrealProperty
		  + Property: "Controller":
		  + Type: nullptr						<- weak
reference
so not set.
		  + ReplicationData: FUnrealRepData
				+ RepLayoutType: REPCMD_PropertyObject
				+ Handle: 19
				...
		[3] FUnrealProperty
		  + Property: "SomeTransientProperty"
		  + Type: nullptr
		  + ReplicationData: nullptr
		  + MigratableData: FUnrealMigratableData
				+ RepLayoutType: REPCMD_PropertyFloat
				+ Handle: 1
				...
  + RPCs:
		[0] FUnrealRPC
		  + CallerType: Character
		  + Function: ClientRestart
		  + Type: RPC_Client
		  + bReliable: true
		[1] ....
*/

// As we cannot fully implement replication conditions using SpatialOS's
// component interest API, we
// instead try
// to emulate it by separating all replicated properties into two groups:
// properties which are meant
// for just one
// client (AutonomousProxy/OwnerOnly), or many clients (everything else).
enum EReplicatedPropertyGroup
{
	REP_SingleClient,
	REP_MultiClient
};

// RPC Type. This matches the tag specified in UPROPERTY. For example,
// RPC_Client means RPCs which
// are destined for
// the client (such as ClientAckGoodMove etc).
enum ERPCType
{
	RPC_Client,
	RPC_Server,
	RPC_Unknown
};

struct FUnrealProperty;
struct FUnrealRPC;
struct FUnrealRepData;
struct FUnrealMigratableData;

// A node which represents an unreal type, such as ACharacter or
// UCharacterMovementComponent.
struct FUnrealType
{
	UStruct* Type;
	TMap<UProperty*, TSharedPtr<FUnrealProperty>> Properties;
	TMap<UFunction*, TSharedPtr<FUnrealRPC>> RPCs;
	TWeakPtr<FUnrealProperty> ParentProperty;
};

// A node which represents a single property or parameter in an RPC.
struct FUnrealProperty
{
	UProperty* Property;
	TSharedPtr<FUnrealType> Type;  // Only set if strong reference to object/struct property.
	TSharedPtr<FUnrealRepData> ReplicationData;		   // Only set if property is replicated.
	TSharedPtr<FUnrealMigratableData> MigratableData;  // Only set if property is
													   // migratable (and not
													   // replicated).
	TWeakPtr<FUnrealType> ContainerType;  // Not set if this property is an RPC parameter.
};

// A node which represents an RPC.
struct FUnrealRPC
{
	UClass* CallerType;
	UFunction* Function;
	ERPCType Type;
	TMap<UProperty*, TSharedPtr<FUnrealProperty>> Parameters;
	bool bReliable;
};

// A node which represents replication data generated by the FRepLayout
// instantiated from a UClass.
struct FUnrealRepData
{
	ERepLayoutCmdType RepLayoutType;
	ELifetimeCondition Condition;
	ELifetimeRepNotifyCondition RepNotifyCondition;
	int32 CmdIndex;
	uint16 Handle;
	int32 RoleSwapHandle;
};

// A node which represents migratable data.
struct FUnrealMigratableData
{
	ERepLayoutCmdType RepLayoutType;
	uint16 Handle;
};

using FUnrealFlatRepData =
	TMap<EReplicatedPropertyGroup, TMap<uint16, TSharedPtr<FUnrealProperty>>>;
using FUnrealRPCsByType = TMap<ERPCType, TArray<TSharedPtr<FUnrealRPC>>>;

// Given a UClass, returns either "AFoo" or "UFoo" depending on whether Foo is a
// subclass of actor.
FString GetFullCPPName(UClass* Class);

// Utility functions to convert certain enum values to strings.
FString GetLifetimeConditionAsString(ELifetimeCondition Condition);
FString GetRepNotifyLifetimeConditionAsString(ELifetimeRepNotifyCondition Condition);

// Get an array of all replicated property groups
TArray<EReplicatedPropertyGroup> GetAllReplicatedPropertyGroups();

// Convert a replicated property group to a string. Used to generate component
// names.
FString GetReplicatedPropertyGroupName(EReplicatedPropertyGroup Group);

// Get an array of all RPC types.
TArray<ERPCType> GetRPCTypes();

// Given a UFunction, determines the RPC type.
ERPCType GetRPCTypeFromFunction(UFunction* Function);

TArray<FString> GetRPCTypeOwners(TSharedPtr<FUnrealType> TypeInfo);

// Converts an RPC type to string. Used to generate component names.
FString GetRPCTypeName(ERPCType RPCType);

// Given a UProperty, returns the corresponding ERepLayoutCmdType.
ERepLayoutCmdType PropertyToRepLayoutType(UProperty* Property);

// Given an AST, this applies the function 'Visitor' to all FUnrealType's
// contained transitively
// within the properties. bRecurseIntoObjects will control
// whether this function will recurse into a UObject's properties, which may not
// always be
// desirable. However, it will always recurse into substructs.
// If the Visitor function returns false, it will not recurse any further into
// that part of the
// tree.
void VisitAllObjects(TSharedPtr<FUnrealType> TypeNode,
					 TFunction<bool(TSharedPtr<FUnrealType>)> Visitor, bool bRecurseIntoSubobjects);

// Similar to 'VisitAllObjects', but instead applies the Visitor function to all
// properties which
// are traversed.
void VisitAllProperties(TSharedPtr<FUnrealType> TypeNode,
						TFunction<bool(TSharedPtr<FUnrealProperty>)> Visitor,
						bool bRecurseIntoSubobjects);

// Similar to 'VisitAllObjects', but instead applies the Visitor function to all
// parameters in an
// RPC (and subproperties of structs/objects where appropriate).
void VisitAllProperties(TSharedPtr<FUnrealRPC> RPCNode,
						TFunction<bool(TSharedPtr<FUnrealProperty>)> Visitor,
						bool bRecurseIntoSubobjects);

// Generates an AST from an Unreal UStruct or UClass.
// At the moment, this function receives a manual list of migratable property
// chains in this form:
//   {
//     {"Property"},
//	   {"OtherProperty", "PropertyWithinOtherProperty"}
//   }
// In the future, we can get this information directly from the UStruct*.
TSharedPtr<FUnrealType> CreateUnrealTypeInfo(UStruct* Type,
											 const TArray<TArray<FName>>& MigratableProperties);

// Traverses an AST, and generates a flattened list of replicated properties,
// which will match the
// Cmds array of FRepLayout.
// The list of replicated properties will all have the ReplicatedData field set
// to a valid
// FUnrealRepData node which contains
// data such as the handle or replication condition.
//
// This function will _not_ traverse into subobject properties (as the
// replication system deals with
// each object separately).
FUnrealFlatRepData GetFlatRepData(TSharedPtr<FUnrealType> TypeInfo);

// Traverses an AST, and generates a flattened list of migratable properties.
// The list of migratable
// properties will all have
// the MigratableData field set to a value FUnrealMigratableData node which
// contains data such as
// the handle or replication type.
//
// This function will traverse into subobject properties.
TMap<uint16, TSharedPtr<FUnrealProperty>> GetFlatMigratableData(TSharedPtr<FUnrealType> TypeInfo);

// Traverses an AST fully (including subobjects) and generates a list of all
// RPCs which would be
// routed through an actor channel
// of the Unreal class represented by TypeInfo.
//
// This function will traverse into subobject properties.
FUnrealRPCsByType GetAllRPCsByType(TSharedPtr<FUnrealType> TypeInfo);

// Given an AST, traverses all its parameters (and properties within structs)
// and generates a
// complete flattened list of properties.
TArray<TSharedPtr<FUnrealProperty>> GetFlatRPCParameters(TSharedPtr<FUnrealRPC> RPCNode);

// Given a property, traverse up to the root property and create a list of
// properties needed to
// reach the leaf property.
// For example: foo->bar->baz becomes {"foo", "bar", "baz"}.
TArray<TSharedPtr<FUnrealProperty>> GetPropertyChain(TSharedPtr<FUnrealProperty> LeafProperty);
