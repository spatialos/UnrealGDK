// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "Schema/RPCPayload.h"
#include "SpatialConstants.h"

namespace SpatialGDK
{
// TODO: remove this logic when singletons can be referenced without entity IDs (UNR-1456) - deprecated???
struct FPendingRPCParams
{
	FPendingRPCParams(UObject* InTargetObject, UFunction* InFunction, int InRetryIndex = 0);
	~FPendingRPCParams() = default;

	TWeakObjectPtr<UObject> TargetObject;
	UFunction* Function;

	// TO-DO: Some of the variables might not be needed?
	int RetryIndex; // Index for ordering reliable RPCs on subsequent tries
	int ReliableRPCIndex;
	RPCPayload Payload;
};

using FPendingRPCParamsPtr = TSharedPtr<FPendingRPCParams>;

DECLARE_DELEGATE_RetVal_OneParam(bool, FProcessRPCDelegate, FPendingRPCParamsPtr)

class RPCContainer
{
public:
	void QueueRPC(FPendingRPCParamsPtr Params, ESchemaComponentType Type);
	void ProcessRPCs(const FProcessRPCDelegate& FunctionToApply);

private:
	using FQueueOfParams = TQueue<FPendingRPCParamsPtr>;
	using FRPCMap = TMap<TWeakObjectPtr<const UObject>, TSharedPtr<FQueueOfParams>>;
	using RPCContainerType = TMap<ESchemaComponentType, FRPCMap>;

	void QueueRPC(const UObject* TargetObject, ESchemaComponentType Type, FPendingRPCParamsPtr Params);
	void ProcessRPCs(const FProcessRPCDelegate& FunctionToApply, FQueueOfParams* RPCList);
	bool ApplyFunction(const FProcessRPCDelegate& FunctionToApply, FPendingRPCParamsPtr Params);

	RPCContainerType QueuedRPCs;
};
}
