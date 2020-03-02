// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "ObjectStub.h"

FRPCErrorInfo UObjectStub::ProcessRPC(const FPendingRPCParams& Params)
{
	return FRPCErrorInfo{ nullptr, nullptr, ERPCResult::UnresolvedParameters };
}
