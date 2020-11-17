// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "SpatialCommonTypes.h"

#include <WorkerSDK/improbable/c_trace.h>

struct SPATIALGDK_API FSpatialGDKSpanId
{
	FSpatialGDKSpanId();
	explicit FSpatialGDKSpanId(bool bInIsValid);
	explicit FSpatialGDKSpanId(const Trace_SpanIdType* TraceSpanId);

	FString ToString() const;
	static FString ToString(const Trace_SpanIdType* TraceSpanId);

	bool IsNull() const { return Trace_SpanId_IsNull(Data); }
	bool IsValid() const { return bIsValid; }

	void WriteData(const Trace_SpanIdType* TraceSpanId);
	Trace_SpanIdType* GetData();
	const Trace_SpanIdType* GetConstData() const;

private:
	bool bIsValid;
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
	void WriteToBuffer(const int32 SpanIndex, const FSpatialGDKSpanId& TraceSpanId);

	Trace_SpanIdType* Buffer;
	std::allocator<Trace_SpanIdType> Allocator;
	const int32 NumSpanIds;
	const int32 NumBytes;
};
