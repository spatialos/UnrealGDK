// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "Interop/Connection/SpatialSpanIdStack.h"

namespace SpatialGDK
{
void FSpatialSpanIdStack::AddNewLayer(const Trace_SpanId& SpanId)
{
	Stack.Add({ SpanId });
}

void FSpatialSpanIdStack::AddToLayer(const Trace_SpanId& SpanId)
{
	const int32 Size = Stack.Num();
	if (Size == 0)
	{
		AddNewLayer(SpanId);
		return;
	}

	Stack[Size - 1].Add(SpanId);
}

TArray<Trace_SpanId> FSpatialSpanIdStack::PopLayer()
{
	return Stack.Pop();
}

TArray<Trace_SpanId> FSpatialSpanIdStack::GetTopLayer() const
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
