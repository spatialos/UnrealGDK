// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "ExampleMock.h"

FRPCErrorInfo UExampleMock::ProcessRPC(const FPendingRPCParams& Params)
{
	return FRPCErrorInfo{ nullptr, nullptr, true, ERPCQueueType::Send, ERPCResult::UnresolvedParameters };
}
