// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "SpatialCommonTypes.h"
#include "SpatialEventMessages.h"

#include <WorkerSDK/improbable/c_worker.h>

// TODO(EventTracer): make sure SpatialEventTracer doesn't break the LatencyTracer functionality for now (maybe have some macro/branching in .cpp file, when the LatencyTracer is enabled?)
// TODO(EventTracer): make sure the overhead of SpatialEventTracer is minimal when it's switched off

DECLARE_LOG_CATEGORY_EXTERN(LogSpatialEventTracer, Log, All);

//TODO - Individual RPC Calls (distinguishing between GDK and USER)
//TODO - RPCs on newly created objects go through a different flow and need to be handled

class AActor;
class UFunction;

namespace worker
{
	namespace c
	{
		struct Io_Stream;
		struct Trace_Item;
	}
}

// Note(EventTracer): EventTracer must be created prior to WorkerConnection, since it has to be passed to ConnectionConfig
// (see SpatialConnectionManager diff)

namespace SpatialGDK
{

// Note: SpatialEventTracer wraps Trace_EventTracer related functionality
// It is constructed and owned by SpatialConnectionManager.
// SpatialNetDriver initializes SpatialSender and SpatialReceiver with pointers to EventTracer read from SpatialConnectionManager.
// Note(EventTracer): SpatialEventTracer is supposed to never be null in SpatialWorkerConnection, SpatialSender, SpatialReceiver. Make sure there are necessary nullptr checks if that changes.

class SpatialEventTracer
{
public:
	SpatialEventTracer(const FString& WorkerName);
	~SpatialEventTracer();
	Trace_SpanId CreateNewSpanId();
	Trace_SpanId CreateNewSpanId(const TArray<Trace_SpanId>& Causes);

	const worker::c::Trace_EventTracer* GetConstWorkerEventTracer() const { return EventTracer; };
	worker::c::Trace_EventTracer* GetWorkerEventTracer() const { return EventTracer; }

	// TODO(EventTracer): add the option to SpatialSpanIdActivator for Sent TraceEvents.
	// Consider making sure it's not accepting rvalue (since SpatialSpanIdActivatNetDriveror must live long enough for the worker sent op to be registered with this SpanId)
	// e.g. void TraceEvent(... SpatialSpanIdActivator&& SpanIdActivator) = delete;
	// TODO(EventTracer): Communicate to others, that SpatialSpanIdActivator must be creating prior to calling worker send functions

	template<class T>
	TOptional<Trace_SpanId> TraceEvent(const T& EventMessage, const worker::c::Trace_SpanId* Cause = nullptr)
	{
		return TraceEvent(EventMessage, T::StaticStruct(), Cause);
	}

	bool IsEnabled() const { return !!bEnabled; }

private:

	void Enable(const FString& FileName);
	void Disable();

	static void TraceCallback(void* UserData, const Trace_Item* Item);
	bool bRecordRuntimeAndWorkerEvents{ false };

	bool bEnabled{ false };
	worker::c::Io_Stream* Stream;

	worker::c::Trace_EventTracer* EventTracer{ nullptr };

	FString WorkerName;
	uint64 BytesWrittenToStream{ 0 };
	uint64 MaxFileSize{ 0 };

	TOptional<Trace_SpanId> TraceEvent(const FEventMessage& EventMessage, const UStruct* Struct, const worker::c::Trace_SpanId* Cause);
};

struct SpatialSpanIdActivator
{
	SpatialSpanIdActivator(SpatialEventTracer* InEventTracer, const TOptional<Trace_SpanId>& InCurrentSpanId);
	~SpatialSpanIdActivator();

	SpatialSpanIdActivator(const SpatialSpanIdActivator&) = delete;
	SpatialSpanIdActivator(SpatialSpanIdActivator&&) = delete;
	SpatialSpanIdActivator& operator=(const SpatialSpanIdActivator&) = delete;
	SpatialSpanIdActivator& operator=(SpatialSpanIdActivator&&) = delete;

private:
	TOptional<Trace_SpanId> CurrentSpanId;
	worker::c::Trace_EventTracer* EventTracer;
};

}

// TODO(EventTracer): a short list of requirements by Alex
/*
Actor name, Position,
Add/Remove Entity (can we also distinguish Remove Entity when moving to another worker vs Delete entity),
RPC calls (when they were sent/received/processed),
*/
