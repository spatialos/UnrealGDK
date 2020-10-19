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

	UENUM() enum class ERPCResult : uint8 {
		Success,

		// Shared across Sender and Receiver
		UnresolvedTargetObject,
		MissingFunctionInfo,
		UnresolvedParameters,
		NoAuthority,

		// Sender specific
		NoActorChannel,
		SpatialActorChannelNotListening,
		NoNetConnection,
		InvalidRPCType,

		// Specific to packing
		NoOwningController,
		NoControllerChannel,
		ControllerChannelNotListening,

		RPCServiceFailure,

		Unknown
	};

enum class ERPCQueueProcessResult : uint8_t
{
	ContinueProcessing,
	StopProcessing,
	DropEntireQueue
};

enum class ERPCQueueType : uint8_t
{
	Send,
	Receive,
	Unknown
};

struct FRPCErrorInfo
{
	bool Success() const { return (ErrorCode == ERPCResult::Success); }

	TWeakObjectPtr<UObject> TargetObject = nullptr;
	TWeakObjectPtr<UFunction> Function = nullptr;
	ERPCResult ErrorCode = ERPCResult::Unknown;
	ERPCQueueProcessResult QueueProcessResult = ERPCQueueProcessResult::StopProcessing;
};

struct SPATIALGDK_API FPendingRPCParams
{
	FPendingRPCParams(const FUnrealObjectRef& InTargetObjectRef, ERPCType InType, SpatialGDK::RPCPayload&& InPayload);

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
	ERPCType Type;
};

class SPATIALGDK_API FRPCContainer
{
public:
	// Moveable, not copyable.
	FRPCContainer(ERPCQueueType QueueType);
	FRPCContainer() = delete;
	FRPCContainer(const FRPCContainer&) = delete;
	FRPCContainer(FRPCContainer&&) = default;
	FRPCContainer& operator=(const FRPCContainer&) = delete;
	FRPCContainer& operator=(FRPCContainer&&) = default;
	~FRPCContainer() = default;

	void BindProcessingFunction(const FProcessRPCDelegate& Function);
	void ProcessOrQueueRPC(const FUnrealObjectRef& InTargetObjectRef, ERPCType InType, SpatialGDK::RPCPayload&& InPayload);
	void ProcessRPCs();
	void DropForEntity(const Worker_EntityId& EntityId);

	bool ObjectHasRPCsQueuedOfType(const Worker_EntityId& EntityId, ERPCType Type) const;

private:
	using FArrayOfParams = TArray<FPendingRPCParams>;
	using FRPCMap = TMap<Worker_EntityId_Key, FArrayOfParams>;
	using RPCContainerType = TMap<ERPCType, FRPCMap>;

	void ProcessRPCs(FArrayOfParams& RPCList);
	ERPCQueueProcessResult ApplyFunction(FPendingRPCParams& Params);
	RPCContainerType QueuedRPCs;
	FProcessRPCDelegate ProcessingFunction;
	bool bAlreadyProcessingRPCs = false;

	ERPCQueueType QueueType = ERPCQueueType::Unknown;
};
