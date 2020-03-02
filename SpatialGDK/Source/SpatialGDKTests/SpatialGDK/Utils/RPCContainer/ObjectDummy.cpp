// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "ObjectDummy.h"

FRPCErrorInfo UObjectDummy::ProcessRPC(const FPendingRPCParams& Params)
{
	return FRPCErrorInfo{ nullptr, nullptr, ERPCResult::Success };
}
