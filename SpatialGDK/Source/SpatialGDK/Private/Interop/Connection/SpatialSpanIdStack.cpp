// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "Interop/Connection/SpatialSpanIdStack.h"

#include "Interop/Connection/SpatialEventTracer.h"

namespace SpatialGDK
{
FSpatialSpanIdStack::FSpatialSpanIdStack(const SpatialEventTracer* InEventTracer)
	: EventTracer(InEventTracer)
{
}

void FSpatialSpanIdStack::SetEventTracer(const SpatialEventTracer* InEventTracer)
{
	EventTracer = InEventTracer;
}

void FSpatialSpanIdStack::Stack(const Trace_SpanId& SpanId)
{
	SpanIdStack.Add(SpanId);
}

void FSpatialSpanIdStack::Add(const Trace_SpanId& SpanId)
{
	const int32 Size = SpanIdStack.Num();
	if (Size == 0)
	{
		Stack(SpanId);
		return;
	}

	const Trace_SpanId& TopSpanId = SpanIdStack[Size - 1];
	Trace_SpanId MergeCauses[2] = { SpanId, TopSpanId };
	SpanIdStack[Size - 1] = EventTracer->CreateSpan(MergeCauses, 2).GetValue();
}

TOptional<Trace_SpanId> FSpatialSpanIdStack::Pop()
{
	return SpanIdStack.Pop();
}

TOptional<Trace_SpanId> FSpatialSpanIdStack::GetSpanId() const
{
	if (IsEmpty())
	{
		return {};
	}

	return SpanIdStack[SpanIdStack.Num() - 1];
}

bool FSpatialSpanIdStack::IsEmpty() const
{
	return SpanIdStack.Num() == 0;
}
} // namespace SpatialGDK
