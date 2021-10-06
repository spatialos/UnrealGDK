// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "Interop/Connection/SpatialGDKSpanId.h"

static_assert(sizeof(FSpatialGDKSpanId) == TRACE_SPAN_ID_SIZE_BYTES, "Size must match TRACE_SPAN_ID_SIZE_BYTES");

// ----- FSpatialGDKSpanId -----

FSpatialGDKSpanId::FSpatialGDKSpanId()
{
	WriteId(Trace_SpanId_Null());
}

FSpatialGDKSpanId::FSpatialGDKSpanId(const Trace_SpanIdType* TraceSpanId)
{
	WriteId(TraceSpanId != nullptr ? TraceSpanId : Trace_SpanId_Null());
}

void FSpatialGDKSpanId::WriteId(const Trace_SpanIdType* TraceSpanId)
{
	FMemory::Memcpy(Id, TraceSpanId, TRACE_SPAN_ID_SIZE_BYTES);
}

Trace_SpanIdType* FSpatialGDKSpanId::GetId()
{
	return Id;
}

const Trace_SpanIdType* FSpatialGDKSpanId::GetConstId() const
{
	return Id;
}

FString FSpatialGDKSpanId::ToString() const
{
	return ToString(Id);
}

FString FSpatialGDKSpanId::ToString(const Trace_SpanIdType* TraceSpanId)
{
	FString HexStr;
	for (int i = 0; i < TRACE_SPAN_ID_SIZE_BYTES; i++)
	{
		HexStr += FString::Printf(TEXT("%02x"), TraceSpanId[i]);
	}
	return HexStr;
}
