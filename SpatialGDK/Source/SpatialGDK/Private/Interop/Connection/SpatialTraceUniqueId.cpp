// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "Interop/Connection/SpatialTraceUniqueId.h"

#include "Utils/SchemaUtils.h"

using namespace SpatialGDK;

FString EventTraceUniqueId::ToString() const
{
	return FString::Printf(TEXT("%0X"), Hash);
}

EventTraceUniqueId EventTraceUniqueId::Generate(uint64 Entity, uint8 Type, uint64 Id)
{
	uint32 Hash = HashCombine(HashCombine(GetTypeHash(Entity), GetTypeHash(Type)), GetTypeHash(Id));
	return EventTraceUniqueId(Hash);
}

EventTraceUniqueId EventTraceUniqueId::Generate(uint64 Entity, FName PropertyName)
{
	return EventTraceUniqueId(HashCombine(GetTypeHash(Entity), GetTypeHash(PropertyName)));
}
