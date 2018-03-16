// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "EngineMinimal.h"
#include "Net/RepLayout.h"

/*

Example AST generated from Character:

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
			+ Type: nullptr						<- weak reference so not set.
			+ ReplicationData: FUnrealRepData
				+ RepLayoutType: REPCMD_PropertyObject
				+ Handle: 19
				...
	+ RPCs:
		[0] FUnrealRPC
			+ CallerType: Character
			+ Function: ClientRestart
			+ Type: RPC_Client
			+ bReliable: true
		[1] ....

*/

struct FPropertyInfo_OLD
{
	UProperty* Property;
	ERepLayoutCmdType Type;

	// If Struct or Object, this will be set to the UClass*
	UStruct* PropertyStruct;

	// Properties that were traversed to reach this property, including the property itself.
	TArray<UProperty*> Chain;

	bool operator==(const FPropertyInfo_OLD& Other) const
	{
		return Property == Other.Property && Type == Other.Type && Chain == Other.Chain;
	}
};

struct FRepLayoutEntry_OLD
{
	UProperty* Property;
	UProperty* Parent;
	TArray<UProperty*> Chain;
	ELifetimeCondition Condition;
	ELifetimeRepNotifyCondition RepNotifyCondition;
	ERepLayoutCmdType Type;
	int CmdIndex;
	uint16 Handle;
};

struct FReplicatedPropertyInfo_OLD
{
	FRepLayoutEntry_OLD Entry;
	// Usually a singleton list containing the PropertyInfo this RepLayoutEntry refers to.
	// In some cases (such as a struct within a struct), this can refer to many properties in schema.
	TArray<FPropertyInfo_OLD> PropertyList;
};

enum EReplicatedPropertyGroup
{
	REP_SingleClient,
	REP_MultiClient
};

// RPC Type. This matches the tag specified in UPROPERTY. For example, RPC_Client means RPCs which are destined for
// the client (such as ClientAckGoodMove etc).
enum ERPCType
{
	RPC_Client,
	RPC_Server,
	RPC_Unknown
};

struct FRPCDefinition_OLD
{
	UClass* CallerType;
	UFunction* Function;
	ERPCType Type;
	bool bReliable;
};

struct FPropertyLayout_OLD
{
	TMap<EReplicatedPropertyGroup, TArray<FReplicatedPropertyInfo_OLD>> ReplicatedProperties;
	TArray<FPropertyInfo_OLD> CompleteProperties;
	TMap<ERPCType, TArray<FRPCDefinition_OLD>> RPCs;
};

struct FUnrealProperty;
struct FUnrealRPC;
struct FUnrealRepData;

struct FUnrealType
{
	UStruct* Type;
	TMap<UProperty*, TSharedPtr<FUnrealProperty>> Properties;
	TMap<UFunction*, TSharedPtr<FUnrealRPC>> RPCs;
	TWeakPtr<FUnrealProperty> ParentProperty;
};

struct FUnrealProperty
{
	UProperty* Property;
	TSharedPtr<FUnrealType> Type; // Only set if strong reference to object/struct property.
	TSharedPtr<FUnrealRepData> ReplicationData; // Only set if property is replicated.
	TWeakPtr<FUnrealType> ContainerType;
};

struct FUnrealRPC
{
	UClass* CallerType;
	UFunction* Function;
	ERPCType Type;
	TMap<UProperty*, TSharedPtr<FUnrealProperty>> Parameters;
	bool bReliable;
};

struct FUnrealRepData
{
	ERepLayoutCmdType RepLayoutType;
	ELifetimeCondition Condition;
	ELifetimeRepNotifyCondition RepNotifyCondition;
	int CmdIndex;
	uint16 Handle;
};

// Given a UClass, returns either "AFoo" or "UFoo" depending on whether it is a subclass of actor.
FString GetFullCPPName(UClass* Class);

// Utility functions to convert certain enum values to strings.
FString GetLifetimeConditionAsString(ELifetimeCondition Condition);
FString GetRepNotifyLifetimeConditionAsString(ELifetimeRepNotifyCondition Condition);

// Get an array of all replicated property groups
TArray<EReplicatedPropertyGroup> GetAllReplicatedPropertyGroups();

// Convert a replicated property group to a string. Used to generate component names.
FString GetReplicatedPropertyGroupName(EReplicatedPropertyGroup Group);

// Get an array of all RPC types.
TArray<ERPCType> GetRPCTypes();

// Given a UFunction, determines the RPC type.
ERPCType GetRPCTypeFromFunction(UFunction* Function);

// Converts an RPC type to string. Used to generate component names.
FString GetRPCTypeName(ERPCType RPCType);

// Given a UProperty, returns the corresponding ERepLayoutCmdType.
ERepLayoutCmdType PropertyToRepLayoutType(UProperty* Property);

// CDO - Class default object which contains Property
void VisitProperty(TArray<FPropertyInfo_OLD>& PropertyInfo, UObject* CDO, TArray<UProperty*> Stack, UProperty* Property);

FPropertyLayout_OLD CreatePropertyLayout(UClass* Class);

void VisitAllObjects(TSharedPtr<FUnrealType> TypeNode, TFunction<bool(TSharedPtr<FUnrealType>)> Visitor, bool bRecurseIntoSubobjects);
void VisitAllProperties(TSharedPtr<FUnrealType> TypeNode, TFunction<bool(TSharedPtr<FUnrealProperty>)> Visitor, bool bRecurseIntoSubobjects);
void VisitAllProperties(TSharedPtr<FUnrealRPC> TypeNode, TFunction<bool(TSharedPtr<FUnrealProperty>)> Visitor, bool bRecurseIntoSubobjects);

TSharedPtr<FUnrealType> CreateUnrealTypeInfo(UStruct* Type);

TMap<EReplicatedPropertyGroup, TMap<uint16, TSharedPtr<FUnrealProperty>>> GetFlatRepData(TSharedPtr<FUnrealType> TypeInfo);
TMap<ERPCType, TArray<TSharedPtr<FUnrealRPC>>> GetAllRPCsByType(TSharedPtr<FUnrealType> TypeInfo);