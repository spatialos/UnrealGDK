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

class UFunction;
class AActor;

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

// TODO: discuss overhead from constructing SpatialGDKEvents
SpatialGDKEvent ConstructEventFromRPC(AActor* Actor, UFunction* Function);

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

// TODO

/*
Sending create entity request

Sending authority intent update

Sending delete entity request

Sending RPC

Sending RPC retry

Sending command response

Receiving add entity

Receiving remove entity

Receiving authority change

Receiving component update

Receiving command request

Receiving command response

Receiving create entity response

Individual RPC Calls (distinguishing between GDK and USER)

Custom events can be added
*/

/*
Actor name, Position,
Add/Remove Entity (can we also distinguish Remove Entity when moving to another worker vs Delete entity),
Authority/Authority intent changes,
RPC calls (when they were sent/received/processed),
Component Updates
*/
