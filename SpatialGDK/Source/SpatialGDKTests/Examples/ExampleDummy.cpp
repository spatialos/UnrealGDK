// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "ExampleDummy.h"

FRPCErrorInfo UExampleDummy::ProcessRPC(const FPendingRPCParams& Params)
{
	return FRPCErrorInfo{ nullptr, nullptr, true, ERPCQueueType::Send, ERPCResult::Success };
}
