// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "Interop/Connection/SpatialTraceUniqueId.h"

#include "Utils/SchemaUtils.h"

using namespace SpatialGDK;

FString EventTraceUniqueId::GetString() const
{
	const uint64* ByteRep = reinterpret_cast<const uint64*>(InternalBytes);
	return FString::Printf(TEXT("%0X%0X"), ByteRep[0], ByteRep[1]);
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
		memcpy(Id.InternalBytes, Bytes, sizeof(InternalBytes));
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
