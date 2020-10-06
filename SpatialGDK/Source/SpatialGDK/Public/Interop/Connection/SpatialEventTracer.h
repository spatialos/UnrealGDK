// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "Interop/Connection/SpatialSpanIdCache.h"
#include "Interop/Connection/SpatialTraceEvent.h"

// Documentation for event tracing in the GDK can be found here: https://brevi.link/gdk-event-tracing-documentation

// TODO(EventTracer): make sure SpatialEventTracer doesn't break the LatencyTracer functionality for now (maybe have some macro/branching in
// .cpp file, when the LatencyTracer is enabled?)
// TODO(EventTracer): make sure the overhead of SpatialEventTracer is minimal when it's switched off

DECLARE_LOG_CATEGORY_EXTERN(LogSpatialEventTracer, Log, All);

// TODO - Individual RPC Calls (distinguishing between GDK and USER)
// TODO - RPCs on newly created objects go through a different flow and need to be handled

namespace worker
{
namespace c
{
struct Io_Stream;
struct Trace_Item;
} // namespace c
} // namespace worker

// Note(EventTracer): EventTracer must be created prior to WorkerConnection, since it has to be passed to ConnectionConfig
// (see SpatialConnectionManager diff)

namespace SpatialGDK
{
// Note: SpatialEventTracer wraps Trace_EventTracer related functionality
// It is owned by the connection handler and ViewCoordinator.
// SpatialNetDriver initializes SpatialSender and SpatialReceiver with pointers to EventTracer read from SpatialWorkerConnection.
// Note(EventTracer): SpatialEventTracer is supposed to never be null in SpatialWorkerConnection, SpatialSender, SpatialReceiver. Make sure
// there are necessary nullptr checks if that changes.

class SpatialEventTracer
{
public:
	SpatialEventTracer(const FString& WorkerId);
	~SpatialEventTracer();

	const worker::c::Trace_EventTracer* GetConstWorkerEventTracer() const { return EventTracer; };
	worker::c::Trace_EventTracer* GetWorkerEventTracer() const { return EventTracer; }

	// TODO(EventTracer): add the option to SpatialScopedActiveSpanId for Sent TraceEvents.
	// Consider making sure it's not accepting rvalue (since SpatialSpanIdActivatNetDriveror must live long enough for the worker sent op to
	// be registered with this SpanId) e.g. void TraceEvent(... SpatialScopedActiveSpanId&& SpanIdActivator) = delete;
	// TODO(EventTracer): Communicate to others, that SpatialScopedActiveSpanId must be creating prior to calling worker send functions

	TOptional<Trace_SpanId> TraceEvent(FSpatialTraceEvent SpatialTraceEvent);
	TOptional<Trace_SpanId> TraceEvent(FSpatialTraceEvent SpatialTraceEvent, const worker::c::Trace_SpanId Causes);
	TOptional<Trace_SpanId> TraceEvent(FSpatialTraceEvent SpatialTraceEvent, const TArray<worker::c::Trace_SpanId>& Causes);
	TOptional<Trace_SpanId> TraceEvent(FSpatialTraceEvent SpatialTraceEvent, const worker::c::Trace_SpanId* Causes, int32 NumCauses);

	bool IsEnabled() const;

	void ComponentAdd(const Worker_Op& Op);
	void ComponentRemove(const Worker_Op& Op);
	void ComponentUpdate(const Worker_Op& Op);

	bool GetSpanId(const EntityComponentId& Id, const uint32 FieldId, worker::c::Trace_SpanId& CauseSpanId, bool bRemove = true);
	bool GetMostRecentSpanId(const EntityComponentId& Id, worker::c::Trace_SpanId& CauseSpanId, bool bRemove = true);

	bool DropSpanId(const EntityComponentId& Id, const uint32 FieldId);
	bool DropSpanIds(const EntityComponentId& Id);

private:
	bool bEnabled{ false };
	worker::c::Trace_EventTracer* EventTracer{ nullptr };

	uint64 BytesWrittenToStream{ 0 };
	uint64 MaxFileSize{ 0 };

	struct StreamDeleter
	{
		void operator()(worker::c::Io_Stream* StreamToDestroy) const;
	};

	TUniquePtr<worker::c::Io_Stream, StreamDeleter> Stream;

	SpatialSpanIdCache SpanIdStore;

	void Enable(const FString& FileName);
	void Disable();

	static void TraceCallback(void* UserData, const Trace_Item* Item);
};

struct SpatialScopedActiveSpanId
{
	SpatialScopedActiveSpanId(SpatialEventTracer* InEventTracer, const TOptional<Trace_SpanId>& InCurrentSpanId);
	~SpatialScopedActiveSpanId();

	SpatialScopedActiveSpanId(const SpatialScopedActiveSpanId&) = delete;
	SpatialScopedActiveSpanId(SpatialScopedActiveSpanId&&) = delete;
	SpatialScopedActiveSpanId& operator=(const SpatialScopedActiveSpanId&) = delete;
	SpatialScopedActiveSpanId& operator=(SpatialScopedActiveSpanId&&) = delete;

private:
	TOptional<Trace_SpanId> CurrentSpanId;
	worker::c::Trace_EventTracer* EventTracer;
};

} // namespace SpatialGDK
