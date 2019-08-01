// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "Utils/RPCContainer.h"

#include "CoreMinimal.h"

#include "ObjectSpy.generated.h"

namespace SpyUtils
{
	TArray<uint8> TypeToArray(ESchemaComponentType Type);
	ESchemaComponentType ArrayToType(const TArray<uint8>& Array);
} // namespace SpyUtils

UCLASS()
class UObjectSpy : public UObject
{
	GENERATED_BODY()
public:
	bool ProcessRPC(const FPendingRPCParams& Params);

	TMap<ESchemaComponentType, TArray<uint32>> ProcessedRPCIndices;
};
