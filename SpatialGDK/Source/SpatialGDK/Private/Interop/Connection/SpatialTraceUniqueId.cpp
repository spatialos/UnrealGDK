// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "Interop/Connection/SpatialTraceUniqueId.h"

using namespace SpatialGDK;

FString EventTraceUniqueId::ToString() const
{
	return FString::Printf(TEXT("%0X"), Hash);
}

EventTraceUniqueId EventTraceUniqueId::GenerateForRPC(Worker_EntityId Entity, uint8 Type, uint64 Id)
{
	uint32 ComputedHash = HashCombine(HashCombine(GetTypeHash(static_cast<int64>(Entity)), GetTypeHash(Type)), GetTypeHash(Id));
	return EventTraceUniqueId(ComputedHash);
}

EventTraceUniqueId EventTraceUniqueId::GenerateForProperty(Worker_EntityId Entity, const GDK_PROPERTY(Property) * Property)
{
	uint32 ComputedHash = HashCombine(GetTypeHash(static_cast<int64>(Entity)), GetTypeHash(Property->GetName()));
	return EventTraceUniqueId(ComputedHash);
}
