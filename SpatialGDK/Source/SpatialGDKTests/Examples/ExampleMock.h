// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "Utils/RPCContainer.h"

#include "CoreMinimal.h"

#include "ExampleMock.generated.h"

UCLASS()
class UExampleMock : public UObject
{
	GENERATED_BODY()
public:
	FRPCErrorInfo ProcessRPC(const FPendingRPCParams& Params);
};
