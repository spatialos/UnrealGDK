// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "MockObject.h"

#include "Core.h"

using namespace SpatialGDK;

TArray<uint8> TypeToArray(ESchemaComponentType Type)
{
	// Make sure that if Type value changes - there will be an explicit error
	check(sizeof(ESchemaComponentType) == sizeof(int32));

	return TArray<uint8>(reinterpret_cast<uint8*>(&static_cast<int32>(Type)), sizeof(int32));
}

ESchemaComponentType ArrayToType(const TArray<uint8>& Array)
{
	// Make sure that if Type value changes - there will be an explicit error
	check(sizeof(ESchemaComponentType) == sizeof(int32));

	return ESchemaComponentType(*reinterpret_cast<const int32*>(&Array[0]));
}

bool UMockObject::ProcessRPC(const FPendingRPCParams& Params)
{
	ESchemaComponentType Type = ArrayToType(Params.Payload.PayloadData);
	ProcessedRPCIndices.FindOrAdd(Type).Push(Params.Payload.Index);
	return true;
}
