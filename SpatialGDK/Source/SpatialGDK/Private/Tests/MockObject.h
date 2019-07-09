// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "Utils/RPCContainer.h"

#include "Core.h"
#include "MockObject.generated.h"

using namespace SpatialGDK;

UCLASS()
class SPATIALGDK_API UMockObject : public UObject
{
	GENERATED_BODY()
public:
	bool ProcessRPC(const FPendingRPCParams& Params);
};
