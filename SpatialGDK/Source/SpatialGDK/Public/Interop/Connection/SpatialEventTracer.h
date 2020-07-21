// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

// TODO Remove maybe?
#include <WorkerSDK/improbable/c_worker.h>

DECLARE_LOG_CATEGORY_EXTERN(LogSpatialEventTracer, Log, All);

//namespace worker {
//namespace c {
//	struct Trace_EventTracer;
//	struct Trace_SpanId;
//}
//}

namespace SpatialGDK
{

struct SpatialSpanId
{
	SpatialSpanId(worker::c::Trace_EventTracer* InEventTracer);
	~SpatialSpanId();

private:
	Trace_SpanId CurrentSpanId;
	worker::c::Trace_EventTracer* EventTracer;
};

struct SpatialGDKEvent
{
	//SpatialSpanId SpanId;
	FString Message;
	FString Type;
	TMap<FString, FString> Data;
};

struct SpatialEventTracer
{
	SpatialEventTracer();
	~SpatialEventTracer();
	SpatialSpanId CreateActiveSpan();
	void TraceEvent(const SpatialGDKEvent& Event);

	void Enable();
	void Disable();
	const worker::c::Trace_EventTracer* GetWorkerEventTracer() const;

private:
	worker::c::Trace_EventTracer* EventTracer;
};

}
