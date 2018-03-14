// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "EngineMinimal.h"
#include "Net/RepLayout.h"

struct FPropertyInfo
{
	UProperty* Property;
	ERepLayoutCmdType Type;

	// If Struct or Object, this will be set to the UClass*
	UStruct* PropertyStruct;

	// Properties that were traversed to reach this property, including the property itself.
	TArray<UProperty*> Chain;

	bool operator==(const FPropertyInfo& Other) const
	{
		return Property == Other.Property && Type == Other.Type && Chain == Other.Chain;
	}
};

struct FRepLayoutEntry
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

struct FReplicatedPropertyInfo
{
	FRepLayoutEntry Entry;
	// Usually a singleton list containing the PropertyInfo this RepLayoutEntry refers to.
	// In some cases (such as a struct within a struct), this can refer to many properties in schema.
	TArray<FPropertyInfo> PropertyList;
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

struct FRPCDefinition
{
	UClass* CallerType;
	UFunction* Function;
	bool bReliable;
};

struct FPropertyLayout
{
	TMap<EReplicatedPropertyGroup, TArray<FReplicatedPropertyInfo>> ReplicatedProperties;
	TArray<FPropertyInfo> CompleteProperties;
	TMap<ERPCType, TArray<FRPCDefinition>> RPCs;
};

// Given a UClass, returns either "AFoo" or "UFoo" depending on whether it is a subclass of actor.
FString GetFullCPPName(UClass* Class);

// Utility functions to convert certain enum values to strings.
FString GetLifetimeConditionAsString(ELifetimeCondition Condition);
FString GetRepNotifyLifetimeConditionAsString(ELifetimeRepNotifyCondition Condition);

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
void VisitProperty(TArray<FPropertyInfo>& PropertyInfo, UObject* CDO, TArray<UProperty*> Stack, UProperty* Property);

FPropertyLayout CreatePropertyLayout(UClass* Class);