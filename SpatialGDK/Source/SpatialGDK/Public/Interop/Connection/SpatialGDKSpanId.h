// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "SpatialCommonTypes.h"

#include <WorkerSDK/improbable/c_trace.h>

struct FSpatialGDKSpanId
{
	explicit FSpatialGDKSpanId(){};
	explicit FSpatialGDKSpanId(const Trace_SpanIdType* TraceSpanId)
	{
		const int32 Size = TRACE_SPAN_ID_SIZE_BYTES * sizeof(Trace_SpanIdType);
		FMemory::Memcpy(Data, TraceSpanId, Size);
	}

	bool IsValid() const { return Trace_SpanId_IsNull(Data); }

	Trace_SpanIdType Data[TRACE_SPAN_ID_SIZE_BYTES];
};

class FMultiGDKSpanIdAllocator
{
public:
	FMultiGDKSpanIdAllocator() = delete;
	FMultiGDKSpanIdAllocator(const FSpatialGDKSpanId& A, const FSpatialGDKSpanId& B);
	FMultiGDKSpanIdAllocator(const TArray<FSpatialGDKSpanId>& SpanIds);
	~FMultiGDKSpanIdAllocator();

	Trace_SpanIdType* GetBuffer() const { return Buffer; }
	int32 GetNumSpanIds() const { return NumSpanIds; }

private:
	void Allocate(const int32 SpanIndex, const FSpatialGDKSpanId& TraceSpanId);

	std::allocator<Trace_SpanIdType> Allocator;
	const int32 NumSpanIds;
	const int32 NumBytes;
	Trace_SpanIdType* Buffer;
};
