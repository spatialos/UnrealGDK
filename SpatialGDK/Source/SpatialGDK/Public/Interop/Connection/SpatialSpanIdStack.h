// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include <WorkerSDK/improbable/c_trace.h>

namespace SpatialGDK
{
class SpatialEventTracer;

class FSpatialSpanIdStack
{
public:
	FSpatialSpanIdStack() {}
	explicit FSpatialSpanIdStack(const SpatialEventTracer* InEventTracer);

	void SetEventTracer(const SpatialEventTracer* InEventTracer);

	void AddNewLayer(const Trace_SpanId& SpanId);
	void AddToLayer(const Trace_SpanId& SpanId);
	TOptional<Trace_SpanId> PopLayer();
	TOptional<Trace_SpanId> GetTopSpanId() const;
	bool HasLayer() const;

private:
	const SpatialEventTracer* EventTracer;
	TArray<Trace_SpanId> Stack;
};
} // namespace SpatialGDK
