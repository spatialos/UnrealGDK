// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "Interop/Connection/SpatialSpanIdStack.h"

namespace SpatialGDK
{
void FSpatialSpanIdStack::AddNewLayer(const Trace_SpanId SpanId)
{
	Stack.Add({ SpanId });
}

void FSpatialSpanIdStack::AddToLayer(const Trace_SpanId SpanId)
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
	const int32 Size = Stack.Num();
	const TArray<Trace_SpanId> TopLayer = Stack[Size - 1];
	Stack.RemoveAt(Size - 1);
	return TopLayer;
}

TArray<Trace_SpanId> FSpatialSpanIdStack::GetTopLayer() const
{
	return Stack[Stack.Num() - 1];
}

bool FSpatialSpanIdStack::HasSpanId() const
{
	return Stack.Num() > 0;
}

} // namespace SpatialGDK
