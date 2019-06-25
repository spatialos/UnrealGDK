// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "Schema/RPCPayload.h"
#include "SpatialConstants.h"

// TO-DO: Remove next line
using namespace SpatialGDK;

// TODO: remove this logic when singletons can be referenced without entity IDs (UNR-1456) - deprecated???
struct FPendingRPCParams
{
	FPendingRPCParams(UObject* InTargetObject, UFunction* InFunction, int InRetryIndex = 0)
		: TargetObject(InTargetObject)
		, Function(InFunction)
		, RetryIndex(InRetryIndex)
		, ReliableRPCIndex(0)
		, Payload(0, 0, TArray<uint8>{})
	{
	}

	~FPendingRPCParams() = default;

	TWeakObjectPtr<UObject> TargetObject;
	UFunction* Function;
	//TArray<uint8> Parameters;
	int RetryIndex; // Index for ordering reliable RPCs on subsequent tries
	int ReliableRPCIndex;
	RPCPayload Payload;
};

using FPendingRPCParamsPtr = TSharedPtr<FPendingRPCParams>;
using FQueueOfParams = TQueue<FPendingRPCParamsPtr>;
using FRPCMap = TMap<TWeakObjectPtr<const UObject>, TSharedPtr<FQueueOfParams>>;
using RPCContainer = TMap<ESchemaComponentType, FRPCMap>;
