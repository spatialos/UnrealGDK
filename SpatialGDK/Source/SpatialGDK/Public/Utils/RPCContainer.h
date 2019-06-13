// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "SpatialConstants.h"

#include "Logging/LogMacros.h"
#include "CoreMinimal.h"

DECLARE_LOG_CATEGORY_EXTERN(LogRPCContainer, Log, All);

#pragma optimize("", off)
namespace SpatialGDK
{

// This is currently only really used for queuing outgoing RPCs in case they have unresolved target object
// or arguments. This is only possible for singletons in a multi-worker scenario.
// TODO: remove this logic when singletons can be referenced without entity IDs (UNR-1456).
struct FPendingRPCParams
{
	FPendingRPCParams(UObject* InTargetObject, UFunction* InFunction, void* InParameters, int InRetryIndex);
	~FPendingRPCParams();

	TWeakObjectPtr<UObject> TargetObject;
	UFunction* Function;
	TArray<uint8> Parameters;
	int RetryIndex; // Index for ordering reliable RPCs on subsequent tries
	int ReliableRPCIndex;
};

using FPendingRPCParamsPtr = TSharedPtr<FPendingRPCParams>;
using FQueueOfParams = TQueue<FPendingRPCParamsPtr>;
using FRPCMap = TMap<TWeakObjectPtr<const UObject>, TSharedPtr<FQueueOfParams>>;

class RPCContainer
{
public:
	~RPCContainer();
	FRPCMap& operator[](ESchemaComponentType ComponentType);
	const FRPCMap& operator[](ESchemaComponentType ComponentType) const;

	const FRPCMap* begin() const;
	const FRPCMap* end() const;
	FRPCMap* begin();
	FRPCMap* end();

private:
	enum class RPCType
	{
		Commands = 0,
		Multicast = 1,
		Reliable = 2,
		Unreliable = 3,
		Invalid = 4,
		NumTypes = 5
	};
	FRPCMap RPCs[int(RPCType::NumTypes)];
};
#pragma optimize("", on)
} // namespace SpatialGDK
