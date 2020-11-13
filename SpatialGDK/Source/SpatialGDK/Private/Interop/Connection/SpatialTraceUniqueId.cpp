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
	uint8 Data[8 + 8 + 1];
	memcpy(&Data[0], &Entity, 8);
	memcpy(&Data[8], &Id, 8);
	memcpy(&Data[16], &Type, 1);
	return EventTraceUniqueId(CityHash64((const char*)&Data, 17));
}
