// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "Schema/RPCPayload.h"
#include "Schema/UnrealObjectRef.h"
#include "SpatialConstants.h"

#include "UObject/Class.h"
#include "UObject/Object.h"
#include "UObject/WeakObjectPtr.h"

#include "CoreMinimal.h"

DECLARE_LOG_CATEGORY_EXTERN(LogRPCContainer, Log, All);

struct FPendingRPCParams;
struct FRPCErrorInfo;
DECLARE_DELEGATE_RetVal_OneParam(FRPCErrorInfo, FProcessRPCDelegate, const FPendingRPCParams&)

enum class ERPCResult : uint8_t
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
	ControllerChannelNotListening,

	Unknown
};

enum class ERPCQueueType : uint8_t
{
	Send,
	Receive,
	Unknown
};

struct FRPCErrorInfo
{
	bool Success() const
	{
		return (ErrorCode == ERPCResult::Success);
	}

	TWeakObjectPtr<UObject> TargetObject = nullptr;
	TWeakObjectPtr<UFunction> Function = nullptr;
	bool bIsServer = false;
	ERPCQueueType QueueType = ERPCQueueType::Unknown;
	ERPCResult ErrorCode = ERPCResult::Unknown;
};

struct SPATIALGDK_API FPendingRPCParams
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

class SPATIALGDK_API FRPCContainer
{
public:
	// Moveable, not copyable.
	FRPCContainer() = default;
	FRPCContainer(const FRPCContainer&) = delete;
	FRPCContainer(FRPCContainer&&) = default;
	FRPCContainer& operator=(const FRPCContainer&) = delete;
	FRPCContainer& operator=(FRPCContainer&&) = default;
	~FRPCContainer() = default;

	void BindProcessingFunction(const FProcessRPCDelegate& Function);
	void ProcessOrQueueRPC(const FUnrealObjectRef& InTargetObjectRef, ESchemaComponentType InType, SpatialGDK::RPCPayload&& InPayload);
	void ProcessRPCs();

	bool ObjectHasRPCsQueuedOfType(const Worker_EntityId& EntityId, ESchemaComponentType Type) const;

	static const double SECONDS_BEFORE_WARNING;

private:
	using FArrayOfParams = TArray<FPendingRPCParams>;
	using FRPCMap = TMap<Worker_EntityId_Key, FArrayOfParams>;
	using RPCContainerType = TMap<ESchemaComponentType, FRPCMap>;

	void ProcessRPCs(FArrayOfParams& RPCList);
	bool ApplyFunction(FPendingRPCParams& Params);
	RPCContainerType QueuedRPCs;
	FProcessRPCDelegate ProcessingFunction;
	bool bAlreadyProcessingRPCs = false;
};
