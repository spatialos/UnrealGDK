// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "Utils/RPCContainer.h"

#include "CoreMinimal.h"

#include "ExampleDummy.generated.h"

UCLASS()
class UExampleDummy : public UObject
{
	GENERATED_BODY()
public:
	FRPCErrorInfo ProcessRPC(const FPendingRPCParams& Params);
};
