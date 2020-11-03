// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "Interop/Connection/SpatialTraceUniqueId.h"

#include "Utils/SchemaUtils.h"

using namespace SpatialGDK;

namespace
{
// TODO: Nicer way to represent this?
void ToHex(uint8 N, char OutHex[3])
{
	const char* Chars = "0123456789ABCDEF";
	OutHex[0] = Chars[(N >> 4) & 0xF];
	OutHex[1] = Chars[N & 0xF];
#if !PLATFORM_LITTLE_ENDIAN
	std::swap(OutHex[0], OutHex[1]);
#endif
	OutHex[2] = '\0';
}
} // namespace

FString EventTraceUniqueId::GetString() const
{
	FString Str;
	char Tmp[3];
	for (int i = 0; i < sizeof(internal_bytes); i++)
	{
		ToHex(internal_bytes[i], Tmp);
		Str += Tmp;
	}
	return Str;
}

void EventTraceUniqueId::WriteToSchemaObject(EventTraceUniqueId Id, Schema_Object* Obj, Schema_FieldId FieldId)
{
	SpatialGDK::AddBytesToSchema(Obj, FieldId, Id.internal_bytes, sizeof(internal_bytes));
}

EventTraceUniqueId EventTraceUniqueId::ReadFromSchemaObject(Schema_Object* Obj, Schema_FieldId FieldId)
{
	EventTraceUniqueId Id;
	const uint8* Bytes = Schema_GetBytes(Obj, FieldId);
	uint32_t BytesLen = Schema_GetBytesLength(Obj, FieldId);
	if (BytesLen == sizeof(internal_bytes))
	{
		for (int i = 0; i < sizeof(internal_bytes); i++)
		{
			Id.internal_bytes[i] = Bytes[i];
		}
	}
	return Id;
}

EventTraceUniqueId EventTraceUniqueId::GenerateUnique()
{
	// Worker api will provide us a function to generate an id which will replace this
	FRandomStream RandomStream(FPlatformTime::Cycles());
	EventTraceUniqueId id;
	for (int i = 0; i < sizeof(id.internal_bytes); i++)
	{
		id.internal_bytes[i] = static_cast<uint8>(RandomStream.GetFraction() * 255.0f);
	}
	return id;
}
