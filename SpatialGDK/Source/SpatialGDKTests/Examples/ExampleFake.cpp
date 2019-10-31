// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "ExampleFake.h"

FRPCErrorInfo UExampleFake::ProcessRPC(const FPendingRPCParams& Params)
{
	return FRPCErrorInfo{ nullptr, nullptr, true, ERPCQueueType::Send, ERPCResult::UnresolvedParameters };
}
