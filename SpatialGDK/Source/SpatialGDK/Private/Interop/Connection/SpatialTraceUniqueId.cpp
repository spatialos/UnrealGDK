// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "Interop/Connection/SpatialTraceUniqueId.h"

#include "Utils/SchemaUtils.h"

using namespace SpatialGDK;

namespace
{
void ToHex(uint8 N, TCHAR OutHex[3])
{
	const TCHAR* Chars = TEXT("0123456789ABCDEF");
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
	TCHAR Tmp[3];
	for (int i = 0; i < sizeof(InternalBytes); i++)
	{
		ToHex(InternalBytes[i], Tmp);
		Str += Tmp;
	}
	return Str;
}

void EventTraceUniqueId::WriteTraceIDToSchemaObject(const EventTraceUniqueId& Id, Schema_Object* Obj, Schema_FieldId FieldId)
{
	AddBytesToSchema(Obj, FieldId, Id.InternalBytes, sizeof(InternalBytes));
}

EventTraceUniqueId EventTraceUniqueId::ReadTraceIDFromSchemaObject(Schema_Object* Obj, Schema_FieldId FieldId)
{
	EventTraceUniqueId Id;
	const uint8* Bytes = Schema_GetBytes(Obj, FieldId);
	uint32_t BytesLen = Schema_GetBytesLength(Obj, FieldId);
	if (BytesLen == sizeof(InternalBytes))
	{
		for (int i = 0; i < sizeof(InternalBytes); i++)
		{
			Id.InternalBytes[i] = Bytes[i];
		}
	}
	return Id;
}

EventTraceUniqueId EventTraceUniqueId::GenerateUnique(const Trace_SpanId& SpanId)
{
	// This will be replaced with a worker-supplied generation method.
	static_assert(sizeof(InternalBytes) == sizeof(SpanId.data), "InternalBytes expected to be the same size as Trace_SpanId");
	EventTraceUniqueId Id;
	memcpy(Id.InternalBytes, SpanId.data, sizeof(InternalBytes));
	return Id;
}
