// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "Net/RepLayout.h"

/*

This file contains functions to generate an abstract syntax tree which is used by the code generating in
SchemaGenerator.cpp. The AST follows a structure which is similar to a UClass/UProperty
structure, but also contains additional metadata such as replication data. One main difference is that the AST
structure will recurse into object properties if it's determined that the container type holds a strong reference
to that subobject (such as a character owning its movement component).

The AST is built by recursing through the UClass/UProperty structure generated by UHT. Afterwards, the rep layout
is generated with FRepLayout::InitFromObjectClass, and the replication handle / etc is stored in FUnrealRepData
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
			+ Type: nullptr						<- weak reference so not set.
			+ ReplicationData: FUnrealRepData
				+ RepLayoutType: REPCMD_PropertyObject
				+ Handle: 19
				...
		[3] FUnrealProperty
			+ Property: "SomeTransientProperty"
			+ Type: nullptr
			+ ReplicationData: FUnrealHandoverData
				+ RepLayoutType: REPCMD_PropertyFloat
				+ Handle: 1
				...
*/

// As we cannot fully implement replication conditions using SpatialOS's component interest API, we instead try
// to emulate it by separating all replicated properties into three groups: properties which are meant for just one
// client (AutonomousProxy/OwnerOnly), initial only properties (InitialOnly), server only properties (ServerOnly), or many clients
// (everything else).
enum EReplicatedPropertyGroup
{
	// Beware that the order of these currently matters for actually applying the variables.
	// Specifically, we will e.g. apply all MultiClient data, then all SingleClient data, etc.
	// This is a difference from native. TODO: UNR-5576.
	REP_MultiClient = 0,
	REP_SingleClient,
	REP_InitialOnly,
	REP_ServerOnly,

	// Iteration helpers
	REP_Count,
	REP_First = REP_MultiClient
};

struct FUnrealProperty;
struct FUnrealRepData;
struct FUnrealSubobject;

// A node which represents an unreal type, such as ACharacter or UCharacterMovementComponent.
struct FUnrealType
{
	UStruct* Type;
	UObject* Object; // Actual instance of the object. Could be the CDO or a Subobject on the CDO/BlueprintGeneratedClass
	FName Name;		 // Name for the object. This is either the name of the object itself, or the name of the property in the blueprint
	TMultiMap<FProperty*, TSharedPtr<FUnrealProperty>> Properties;
	TArray<FUnrealSubobject> NoPropertySubobjects;
	TWeakPtr<FUnrealProperty> ParentProperty;
};

// A node which represents a single property.
struct FUnrealProperty
{
	FProperty* Property;
	TSharedPtr<FUnrealType> Type;				// Only set if strong reference to object/struct property.
	TSharedPtr<FUnrealRepData> ReplicationData; // Only set if property is replicated.
	TWeakPtr<FUnrealType> ContainerType;

	// These variables are used for unique variable checksum generation. We do this to accurately match properties at run-time.
	// They are used in the function GenerateChecksum which will use all three variables and the UProperty itself to create a checksum for
	// each FUnrealProperty.
	int32 StaticArrayIndex;
	uint32 CompatibleChecksum;
	uint32 ParentChecksum;
};

struct FUnrealSubobject
{
	TSharedPtr<FUnrealType> Type;
};

// A node which represents replication data generated by the FRepLayout instantiated from a UClass.
struct FUnrealRepData
{
	ERepLayoutCmdType RepLayoutType;
	ELifetimeCondition Condition;
	ELifetimeRepNotifyCondition RepNotifyCondition;
	uint16 Handle;
	int32 RoleSwapHandle;
	int32 ArrayIndex;
};

using FUnrealFlatRepData = TMap<EReplicatedPropertyGroup, TMap<uint16, TSharedPtr<FUnrealProperty>>>;
using FCmdHandlePropertyMap = TMap<uint16, TSharedPtr<FUnrealProperty>>;
using FSubobjects = TArray<FUnrealSubobject>;

// Get an array of all replicated property groups
TArray<EReplicatedPropertyGroup> GetAllReplicatedPropertyGroups();

// Convert a replicated property group to a string. Used to generate component names.
FString GetReplicatedPropertyGroupName(EReplicatedPropertyGroup Group);

// Given an AST, this applies the function 'Visitor' to all FUnrealType's contained transitively within the properties. bRecurseIntoObjects
// will control whether this function will recurse into a UObject's properties, which may not always be desirable. However, it will always
// recurse into substructs. If the Visitor function returns false, it will not recurse any further into that part of the tree.
void VisitAllObjects(TSharedPtr<FUnrealType> TypeNode, TFunction<bool(TSharedPtr<FUnrealType>)> Visitor);

// Given an AST, this applies the function 'Visitor' to all properties contained transitively within the type. This will recurse into
// substructs. If the Visitor function returns false, it will not recurse any further into that part of the tree.
void VisitAllProperties(TSharedPtr<FUnrealType> TypeNode, TFunction<bool(TSharedPtr<FUnrealProperty>)> Visitor);

// Generates a unique checksum for the Property that allows matching to Unreal's RepLayout Cmds.
uint32 GenerateChecksum(FProperty* Property, uint32 ParentChecksum, int32 StaticArrayIndex);

// Creates a new FUnrealProperty for the included UProperty, generates a checksum for it and then adds it to the TypeNode included.
TSharedPtr<FUnrealProperty> CreateUnrealProperty(TSharedPtr<FUnrealType> TypeNode, FProperty* Property, uint32 ParentChecksum,
												 uint32 StaticArrayIndex);

// Generates an AST from an Unreal UStruct or UClass.
TSharedPtr<FUnrealType> CreateUnrealTypeInfo(UStruct* Type, uint32 ParentChecksum, int32 StaticArrayIndex);

// Traverses an AST, and generates a flattened list of replicated properties, which will match the Cmds array of FRepLayout.
// The list of replicated properties will all have the ReplicatedData field set to a valid FUnrealRepData node which contains
// data such as the handle or replication condition.
//
// This function will _not_ traverse into subobject properties (as the replication system deals with each object separately).
FUnrealFlatRepData GetFlatRepData(TSharedPtr<FUnrealType> TypeInfo);

// Given a property, traverse up to the root property and create a list of properties needed to reach the leaf property.
// For example: foo->bar->baz becomes {"foo", "bar", "baz"}.
TArray<TSharedPtr<FUnrealProperty>> GetPropertyChain(TSharedPtr<FUnrealProperty> LeafProperty);

// Get all default subobjects for an actor.
FSubobjects GetAllSubobjects(TSharedPtr<FUnrealType> TypeInfo);
