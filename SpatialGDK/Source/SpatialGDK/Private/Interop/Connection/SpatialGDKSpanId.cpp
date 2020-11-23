// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "Interop/Connection/SpatialGDKSpanId.h"

// ----- FSpatialGDKSpanId -----

FSpatialGDKSpanId::FSpatialGDKSpanId()
	: bIsValid(false)
{
}

FSpatialGDKSpanId::FSpatialGDKSpanId(bool bInIsValid)
	: bIsValid(bInIsValid)
{
	if (bIsValid)
	{
		WriteData(Trace_SpanId_Null());
	}
}

FSpatialGDKSpanId::FSpatialGDKSpanId(const Trace_SpanIdType* TraceSpanId)
	: bIsValid(true)
{
	WriteData(TraceSpanId);
}

void FSpatialGDKSpanId::WriteData(const Trace_SpanIdType* TraceSpanId)
{
	FMemory::Memcpy(Data, TraceSpanId, TRACE_SPAN_ID_SIZE_BYTES);
}

Trace_SpanIdType* FSpatialGDKSpanId::GetData()
{
	return bIsValid ? Data : nullptr;
}

const Trace_SpanIdType* FSpatialGDKSpanId::GetConstData() const
{
	return bIsValid ? Data : nullptr;
}

FString FSpatialGDKSpanId::ToString() const
{
	return bIsValid ? ToString(Data) : TEXT("");
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

// ----- FMultiGDKSpanIdAllocator -----

FMultiGDKSpanIdAllocator::FMultiGDKSpanIdAllocator(const FSpatialGDKSpanId& A, const FSpatialGDKSpanId& B)
	: Buffer(nullptr)
	, NumSpanIds(2)
	, NumBytes(NumSpanIds * TRACE_SPAN_ID_SIZE_BYTES)
{
	Buffer = Allocator.allocate(NumBytes);
	WriteToBuffer(0, A);
	WriteToBuffer(1, B);
}

FMultiGDKSpanIdAllocator::FMultiGDKSpanIdAllocator(const TArray<FSpatialGDKSpanId>& SpanIds)
	: Buffer(nullptr)
	, NumSpanIds(SpanIds.Num())
	, NumBytes(NumSpanIds * TRACE_SPAN_ID_SIZE_BYTES)
{
	Buffer = Allocator.allocate(NumBytes);
	for (int32 SpanIndex = 0; SpanIndex < SpanIds.Num(); ++SpanIndex)
	{
		WriteToBuffer(SpanIndex, SpanIds[SpanIndex]);
	}
}

FMultiGDKSpanIdAllocator::~FMultiGDKSpanIdAllocator()
{
	Allocator.deallocate(Buffer, NumBytes);
}

void FMultiGDKSpanIdAllocator::WriteToBuffer(const int32 SpanIndex, const FSpatialGDKSpanId& SpanId)
{
	const Trace_SpanIdType* Data = SpanId.GetConstData();
	const int32 NumElements = TRACE_SPAN_ID_SIZE_BYTES / sizeof(Trace_SpanIdType);
	for (int32 ElementIndex = 0; ElementIndex < NumElements; ++ElementIndex)
	{
		const int32 BufferIndex = SpanIndex * TRACE_SPAN_ID_SIZE_BYTES + ElementIndex * sizeof(Trace_SpanIdType);
		Allocator.construct(Buffer + BufferIndex, Data[ElementIndex]);
	}
}
