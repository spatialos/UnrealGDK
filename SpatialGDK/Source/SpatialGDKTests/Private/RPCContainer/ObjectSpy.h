// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include <Utils/RPCContainer.h>

#include <CoreMinimal.h>

#include "SpyObject.generated.h"

TArray<uint8> TypeToArray(ESchemaComponentType Type);
ESchemaComponentType ArrayToType(const TArray<uint8>& Array);

UCLASS()
class USpyObject : public UObject
{
	GENERATED_BODY()
public:
	bool ProcessRPC(const FPendingRPCParams& Params);

	TMap<ESchemaComponentType, TArray<uint32>> ProcessedRPCIndices;
};
