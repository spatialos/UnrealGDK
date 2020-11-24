// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "SpatialCommonTypes.h"

#include <WorkerSDK/improbable/c_trace.h>

struct SPATIALGDK_API FSpatialGDKSpanId
{
	FSpatialGDKSpanId();
	explicit FSpatialGDKSpanId(const Trace_SpanIdType* TraceSpanId);

	FString ToString() const;
	static FString ToString(const Trace_SpanIdType* TraceSpanId);

	bool IsNull() const { return Trace_SpanId_IsNull(Id) > 0; }

	void WriteId(const Trace_SpanIdType* TraceSpanId);
	Trace_SpanIdType* GetId();
	const Trace_SpanIdType* GetConstId() const;

private:
	Trace_SpanIdType Id[TRACE_SPAN_ID_SIZE_BYTES];
};
