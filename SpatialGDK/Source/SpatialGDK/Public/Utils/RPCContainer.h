// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "SpatialConstants.h"

#include "Logging/LogMacros.h"
#include "CoreMinimal.h"

DECLARE_LOG_CATEGORY_EXTERN(LogRPCContainer, Log, All);

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
using FOutgoingRPCMap = TMap<TWeakObjectPtr<const UObject>, TSharedPtr<FQueueOfParams>>;

class RPCContainer
{
public:
	~RPCContainer();
	FOutgoingRPCMap& operator[](ESchemaComponentType ComponentType);
	const FOutgoingRPCMap& operator[](ESchemaComponentType ComponentType) const;

	const FOutgoingRPCMap* begin() const;
	const FOutgoingRPCMap* end() const;
	FOutgoingRPCMap* begin();
	FOutgoingRPCMap* end();

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
	FOutgoingRPCMap OutgoingRPCs[int(RPCType::NumTypes)];
};
} // namespace SpatialGDK
