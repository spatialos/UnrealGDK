// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "Utils/RPCContainer.h"

#include "CoreMinimal.h"

#include "ExampleSpy.generated.h"

//namespace SpyUtils
//{
//	TArray<uint8> SchemaTypeToByteArray(ESchemaComponentType Type);
//	ESchemaComponentType ByteArrayToSchemaType(const TArray<uint8>& Array);
//} // namespace SpyUtils

UCLASS()
class UExampleSpy : public UObject
{
	GENERATED_BODY()
public:
	FRPCErrorInfo ProcessRPC(const FPendingRPCParams& Params);

	TMap<ESchemaComponentType, TArray<uint32>> ProcessedRPCIndices;
};
