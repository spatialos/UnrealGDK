// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "Utils/RPCContainer.h"

#include "CoreMinimal.h"

#include "ObjectDummy.generated.h"

UCLASS()
class UObjectDummy : public UObject
{
	GENERATED_BODY()
public:
	FRPCErrorInfo ProcessRPC(const FPendingRPCParams& Params);
};
