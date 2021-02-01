// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "Utils/RPCContainer.h"

#include "CoreMinimal.h"

#include "ObjectSpy.generated.h"

namespace SpyUtils
{
TArray<uint8> RPCTypeToByteArray(ERPCType Type);
ERPCType ByteArrayToRPCType(const TArray<uint8>& Array);
} // namespace SpyUtils

UCLASS()
class UObjectSpy : public UObject
{
	GENERATED_BODY()
public:
	FRPCErrorInfo ProcessRPC(const FPendingRPCParams& Params);

	TMap<ERPCType, TArray<uint32>> ProcessedRPCIndices;
};
