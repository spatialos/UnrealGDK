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

void FSpatialSpanIdStack::AddNewLayer(const Trace_SpanId& SpanId)
{
	Stack.Add(SpanId);
}

void FSpatialSpanIdStack::AddToLayer(const Trace_SpanId& SpanId)
{
	const int32 Size = Stack.Num();
	if (Size == 0)
	{
		AddNewLayer(SpanId);
		return;
	}

	Trace_SpanId TopSpanId = Stack[Size - 1];
	Trace_SpanId MergeCauses[2] = { SpanId, TopSpanId };
	Stack[Size - 1] = EventTracer->CreateSpan(MergeCauses, 2).GetValue();
}

Trace_SpanId FSpatialSpanIdStack::PopLayer()
{
	return Stack.Pop();
}

Trace_SpanId FSpatialSpanIdStack::GetTopLayer() const
{
	if (!HasLayer())
	{
		return {};
	}

	return Stack[Stack.Num() - 1];
}

bool FSpatialSpanIdStack::HasLayer() const
{
	return Stack.Num() > 0;
}
} // namespace SpatialGDK
