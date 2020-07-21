// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "Interop/Connection/SpatialWorkerConnection.h"

#include <WorkerSDK/improbable/c_trace.h>

SpanId::SpanId(Trace_EventTracer* InEventTracer)
	: CurrentSpanId{}
	, EventTracer(InEventTracer)
{
	CurrentSpanId = Trace_EventTracer_AddSpan(EventTracer, nullptr, 0);
	Trace_EventTracer_SetActiveSpanId(EventTracer, CurrentSpanId);
}

SpanId::~SpanId()
{
	Trace_EventTracer_UnsetActiveSpanId(EventTracer);
}

void USpatialWorkerConnection::SetEventTracer(Trace_EventTracer* EventTracerIn)
{
	EventTracer = EventTracerIn;
}

SpanId USpatialWorkerConnection::CreateActiveSpan()
{
	return SpanId(EventTracer);
}

