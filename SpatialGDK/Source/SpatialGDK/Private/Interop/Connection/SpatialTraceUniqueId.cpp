// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "Interop/Connection/SpatialTraceUniqueId.h"

using namespace SpatialGDK;

FString EventTraceUniqueId::ToString() const
{
	return FString::Printf(TEXT("%0X"), Hash);
}

EventTraceUniqueId EventTraceUniqueId::GenerateForRPC(Worker_EntityId Entity, uint8 Type, uint64 Id)
{
	uint32 Hash = HashCombine(HashCombine(GetTypeHash(Entity), GetTypeHash(Type)), GetTypeHash(Id));
	return EventTraceUniqueId(Hash);
}
