// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include <Utils/RPCContainer.h>

#include <Core.h>

#include "DummyObject.generated.h"

TArray<uint8> TypeToArray(ESchemaComponentType Type);
ESchemaComponentType ArrayToType(const TArray<uint8>& Array);

UCLASS()
class UDummyObject : public UObject
{
	GENERATED_BODY()
public:
	bool ProcessRPC(const FPendingRPCParams& Params);

	TMap<ESchemaComponentType, TArray<uint32>> ProcessedRPCIndices;
};
