// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "Interop/Connection/SpatialTraceUniqueId.h"

using namespace SpatialGDK;

FString EventTraceUniqueId::ToString() const
{
	return FString::Printf(TEXT("%0X"), Hash);
}

EventTraceUniqueId EventTraceUniqueId::GenerateForRPC(FSpatialEntityId Entity, uint8 Type, uint64 Id)
{
	uint32 ComputedHash = HashCombine(HashCombine(GetTypeHash(Entity), GetTypeHash(Type)), GetTypeHash(Id));
	return EventTraceUniqueId(ComputedHash);
}

EventTraceUniqueId EventTraceUniqueId::GenerateForNamedRPC(FSpatialEntityId Entity, FName Name, uint64 Id)
{
	uint32 ComputedHash = HashCombine(HashCombine(GetTypeHash(Entity), GetTypeHash(Name.ToString())), GetTypeHash(Id));
	return EventTraceUniqueId(ComputedHash);
}

EventTraceUniqueId EventTraceUniqueId::GenerateForProperty(FSpatialEntityId Entity, const GDK_PROPERTY(Property) * Property)
{
	uint32 ComputedHash = HashCombine(GetTypeHash(Entity), GetTypeHash(Property->GetName()));
	return EventTraceUniqueId(ComputedHash);
}

EventTraceUniqueId EventTraceUniqueId::GenerateForCrossServerRPC(FSpatialEntityId Entity, uint64 UniqueRequestId)
{
	uint32 ComputedHash = HashCombine(GetTypeHash(Entity), GetTypeHash(static_cast<uint64>(UniqueRequestId)));
	return EventTraceUniqueId(ComputedHash);
}
