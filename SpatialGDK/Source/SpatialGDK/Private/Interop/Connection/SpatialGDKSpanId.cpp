// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "Interop/Connection/SpatialGDKSpanId.h"

FMultiGDKSpanIdAllocator::FMultiGDKSpanIdAllocator(const FSpatialGDKSpanId& A, const FSpatialGDKSpanId& B)
	: NumSpanIds(2)
	, NumBytes(NumSpanIds * TRACE_SPAN_ID_SIZE_BYTES)
{
	Buffer = Allocator.allocate(NumBytes);
	Allocate(0, A);
	Allocate(1, B);
}

FMultiGDKSpanIdAllocator::FMultiGDKSpanIdAllocator(const TArray<FSpatialGDKSpanId>& SpanIds)
	: NumSpanIds(SpanIds.Num())
	, NumBytes(NumSpanIds * TRACE_SPAN_ID_SIZE_BYTES)
{
	Buffer = Allocator.allocate(NumBytes);
	for (int32 SpanIndex = 0; SpanIndex < SpanIds.Num(); ++SpanIndex)
	{
		Allocate(SpanIndex, SpanIds[SpanIndex]);
	}
}

FMultiGDKSpanIdAllocator::~FMultiGDKSpanIdAllocator()
{
	Allocator.deallocate(Buffer, NumBytes);
}

void FMultiGDKSpanIdAllocator::Allocate(const int32 SpanIndex, const FSpatialGDKSpanId& SpanId)
{
	for (int32 ElementIndex = 0; ElementIndex < TRAC_SPAN_ID_SIZE_BYTES; ++ElementIndex)
	{
		const int32 BufferIndex = SpanIndex * TRACE_SPAN_ID_SIZE_BYTES + ElementIndex;
		Allocator.construct(Buffer + BufferIndex, SpanId.Data[ElementIndex]);
	}
}
