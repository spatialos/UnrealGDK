// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include <Utils/RPCContainer.h>

#include <Core.h>

#include "DummyObject.generated.h"

UCLASS()
class UDummyObject : public UObject
{
	GENERATED_BODY()
public:
	bool ProcessRPC(const FPendingRPCParams& Params);
};
