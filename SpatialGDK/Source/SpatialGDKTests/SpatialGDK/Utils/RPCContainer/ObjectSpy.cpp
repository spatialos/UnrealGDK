// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "ObjectSpy.h"

// If this assertion fails, then TypeToArray and ArrayToType
// functions have to be updated correspondingly
static_assert(sizeof(ERPCType) == sizeof(uint8), "");

TArray<uint8> SpyUtils::RPCTypeToByteArray(ERPCType Type)
{
	uint8 ConvertedType = static_cast<uint8>(Type);
	return TArray<uint8>(&ConvertedType, sizeof(ConvertedType));
}

ERPCType SpyUtils::ByteArrayToRPCType(const TArray<uint8>& Array)
{
	return ERPCType(Array[0]);
}

FRPCErrorInfo UObjectSpy::ProcessRPC(const FPendingRPCParams& Params)
{
	ERPCType Type = SpyUtils::ByteArrayToRPCType(Params.Payload.PayloadData);
	ProcessedRPCIndices.FindOrAdd(Type).Push(Params.Payload.Index);
	return FRPCErrorInfo{ nullptr, nullptr, true, ERPCQueueType::Send, ERPCResult::Success };
}
