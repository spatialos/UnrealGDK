// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "ExampleSpy.h"

// If this assertion fails, then TypeToArray and ArrayToType
// functions have to be updated correspondingly
static_assert(sizeof(ESchemaComponentType) == sizeof(int32), "");

//TArray<uint8> SpyUtils::SchemaTypeToByteArray(ESchemaComponentType Type)
//{
//	int32 ConvertedType = static_cast<int32>(Type);
//	return TArray<uint8>(reinterpret_cast<uint8*>(&ConvertedType), sizeof(ConvertedType));
//}
//
//ESchemaComponentType SpyUtils::ByteArrayToSchemaType(const TArray<uint8>& Array)
//{
//	return ESchemaComponentType(*reinterpret_cast<const int32*>(&Array[0]));
//}
//
FRPCErrorInfo UExampleSpy::ProcessRPC(const FPendingRPCParams& Params)
{
	//ESchemaComponentType Type = SpyUtils::ByteArrayToSchemaType(Params.Payload.PayloadData);
	//ProcessedRPCIndices.FindOrAdd(Type).Push(Params.Payload.Index);
	return FRPCErrorInfo{ nullptr, nullptr, true, ERPCQueueType::Send, ERPCResult::Success };
}
