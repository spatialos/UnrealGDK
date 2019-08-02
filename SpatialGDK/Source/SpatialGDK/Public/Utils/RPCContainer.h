// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "Schema/RPCPayload.h"
#include "Schema/UnrealObjectRef.h"
#include "SpatialConstants.h"

#include "Core.h"

DECLARE_LOG_CATEGORY_EXTERN(LogRPCContainer, Log, All);

struct FPendingRPCParams;
struct FRPCErrorInfo;
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
	FPendingRPCParams(const FUnrealObjectRef& InTargetObjectRef, ESchemaComponentType InType, SpatialGDK::RPCPayload&& InPayload);

	// Moveable, not copyable.
	FPendingRPCParams() = delete;
	FPendingRPCParams(const FPendingRPCParams&) = delete;
	FPendingRPCParams(FPendingRPCParams&&) = default;
	FPendingRPCParams& operator=(const FPendingRPCParams&) = delete;
	FPendingRPCParams& operator=(FPendingRPCParams&&) = default;
	~FPendingRPCParams() = default;

	FUnrealObjectRef ObjectRef;
	SpatialGDK::RPCPayload Payload;

	FDateTime Timestamp;
	ESchemaComponentType Type;
};

class FRPCContainer
{
public:
	void BindProcessingFunction(const FProcessRPCDelegate& Function);

	void ProcessOrQueueRPC(const FUnrealObjectRef& InTargetObjectRef, ESchemaComponentType InType, SpatialGDK::RPCPayload&& InPayload);
	void ProcessRPCs();

private:
	using FArrayOfParams = TArray<FPendingRPCParams>;
	using FRPCMap = TMap<Worker_EntityId_Key, FArrayOfParams>;
	using RPCContainerType = TMap<ESchemaComponentType, FRPCMap>;

	void ProcessRPCs(FArrayOfParams& RPCList);
	bool ApplyFunction(const FPendingRPCParams& Params);
	bool ObjectHasRPCsQueuedOfType(const Worker_EntityId& EntityId, ESchemaComponentType Type) const;

	RPCContainerType QueuedRPCs;
	FProcessRPCDelegate ProcessingFunction;
};
