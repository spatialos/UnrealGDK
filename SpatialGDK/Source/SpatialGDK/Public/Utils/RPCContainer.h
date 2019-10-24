// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "Schema/RPCPayload.h"
#include "Schema/UnrealObjectRef.h"
#include "SpatialConstants.h"

#include "CoreMinimal.h"

struct FPendingRPCParams;
using FPendingRPCParamsPtr = TUniquePtr<FPendingRPCParams>;
DECLARE_DELEGATE_RetVal_OneParam(bool, FProcessRPCDelegate, const FPendingRPCParams&)

struct FPendingRPCParams
{
	FPendingRPCParams(const FUnrealObjectRef& InTargetObjectRef, SpatialGDK::RPCPayload&& InPayload, int InReliableRPCIndex = 0);

	// TODO: UNR-1653 Redesign bCheckRPCOrder Tests functionality
	int ReliableRPCIndex;
	FUnrealObjectRef ObjectRef;
	SpatialGDK::RPCPayload Payload;
};

class FRPCContainer
{
public:
	void QueueRPC(FPendingRPCParamsPtr Params, ESchemaComponentType Type);
	void ProcessRPCs(const FProcessRPCDelegate& FunctionToApply);
	bool ObjectHasRPCsQueuedOfType(const Worker_EntityId& EntityId, ESchemaComponentType Type) const;

private:
	using FArrayOfParams = TArray<FPendingRPCParamsPtr>;
	using FRPCMap = TMap<Worker_EntityId_Key, FArrayOfParams>;
	using RPCContainerType = TMap<ESchemaComponentType, FRPCMap>;

	void ProcessRPCs(const FProcessRPCDelegate& FunctionToApply, FArrayOfParams& RPCList);
	static bool ApplyFunction(const FProcessRPCDelegate& FunctionToApply, const FPendingRPCParams& Params);

	RPCContainerType QueuedRPCs;
};
