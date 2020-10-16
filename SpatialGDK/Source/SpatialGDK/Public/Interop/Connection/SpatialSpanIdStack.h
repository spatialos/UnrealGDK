// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include <WorkerSDK/improbable/c_trace.h>

namespace SpatialGDK
{
class FSpatialSpanIdStack
{
public:
	void AddNewLayer(const Trace_SpanId SpanId);
	void AddToLayer(const Trace_SpanId SpanId);
	TArray<Trace_SpanId> PopLayer();
	TArray<Trace_SpanId> GetTopLayer() const;
	bool HasSpanId() const;

private:
	TArray<TArray<Trace_SpanId>> Stack;
};
} // namespace SpatialGDK
