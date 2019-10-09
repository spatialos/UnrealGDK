// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "Utils/RPCContainer.h"

#include "CoreMinimal.h"

#include "SchemaGenObjectStub.generated.h"

UCLASS()
class USchemaGenObjectStub : public UObject
{
	GENERATED_BODY()
public:
	FRPCErrorInfo ProcessRPC(const FPendingRPCParams& Params);

	UPROPERTY(Replicated)
	int IntValue;

	UPROPERTY(Replicated)
	bool BoolValue;
};
