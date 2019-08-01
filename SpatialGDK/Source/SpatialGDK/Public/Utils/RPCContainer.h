// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "Schema/RPCPayload.h"
#include "Schema/UnrealObjectRef.h"
#include "SpatialConstants.h"

#include "CoreMinimal.h"

DECLARE_LOG_CATEGORY_EXTERN(LogRPCContainer, Log, All);

struct FPendingRPCParams;
struct FRPCErrorInfo;
using FPendingRPCParamsPtr = TUniquePtr<FPendingRPCParams>;
DECLARE_DELEGATE_RetVal_OneParam(FRPCErrorInfo, FProcessRPCDelegate, const FPendingRPCParams&)

enum class ERPCError : uint8_t
{
	Success,

	// Shared across Sender and Receiver
	UnresolvedTargetObject,
	MissingFunctionInfo,
	UnresolvedParameters,

	// Sender specific
	NoActorChannel,
	SpatialActorChannelNotListening,
	NoNetConnection,
	NoAuthority,
	InvalidRPCType,

	// Specific to packing
	NoOwningController,
	NoControllerChannel,
	UnresolvedController,
	ControllerChannelNotListening
};

struct FRPCErrorInfo
{
	bool Success() const
	{
		return (ErrorCode == ERPCError::Success);
	}

	UObject* TargetObject;
	UFunction* Function;
	ERPCError ErrorCode;
};

struct FPendingRPCParams
{
	FPendingRPCParams(const FUnrealObjectRef& InTargetObjectRef, SpatialGDK::RPCPayload&& InPayload, int InReliableRPCIndex = 0);

	// TODO: UNR-1653 Redesign bCheckRPCOrder Tests functionality
	int ReliableRPCIndex;
	FUnrealObjectRef ObjectRef;
	SpatialGDK::RPCPayload Payload;

	FDateTime Timestamp;
};

class FRPCContainer
{
public:
	void BindProcessingFunction(const FProcessRPCDelegate& Function);

	void ProcessOrQueueRPC(FPendingRPCParamsPtr Params, ESchemaComponentType Type);
	void ProcessRPCs();

private:
	using FArrayOfParams = TArray<FPendingRPCParamsPtr>;
	using FRPCMap = TMap<Worker_EntityId_Key, FArrayOfParams>;
	using RPCContainerType = TMap<ESchemaComponentType, FRPCMap>;

	void ProcessRPCs(FArrayOfParams& RPCList);
	bool ApplyFunction(const FPendingRPCParams& Params);
	bool ObjectHasRPCsQueuedOfType(const Worker_EntityId& EntityId, ESchemaComponentType Type) const;

	RPCContainerType QueuedRPCs;
	FProcessRPCDelegate ProcessingFunction;
};
