// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "ExampleStub.h"

FRPCErrorInfo UExampleStub::ProcessRPC(const FPendingRPCParams& Params)
{
	return FRPCErrorInfo{ nullptr, nullptr, true, ERPCQueueType::Send, ERPCResult::UnresolvedParameters };
}
