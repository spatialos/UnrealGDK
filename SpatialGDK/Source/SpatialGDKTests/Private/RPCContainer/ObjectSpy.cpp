// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "ObjectSpy.h"

TArray<uint8> SpyUtils::TypeToArray(ESchemaComponentType Type)
{
	// Make sure that if Type value changes - there will be an explicit error
	static_assert(sizeof(ESchemaComponentType) == sizeof(int32), "");

	int32 ConvertedType = static_cast<int32>(Type);
	return TArray<uint8>(reinterpret_cast<uint8*>(&ConvertedType), sizeof(ConvertedType));
}

ESchemaComponentType SpyUtils::ArrayToType(const TArray<uint8>& Array)
{
	// Make sure that if Type value changes - there will be an explicit error
	static_assert(sizeof(ESchemaComponentType) == sizeof(int32), "");

	return ESchemaComponentType(*reinterpret_cast<const int32*>(&Array[0]));
}

bool UObjectSpy::ProcessRPC(const FPendingRPCParams& Params)
{
	ESchemaComponentType Type = SpyUtils::ArrayToType(Params.Payload.PayloadData);
	ProcessedRPCIndices.FindOrAdd(Type).Push(Params.Payload.Index);
	return true;
}
