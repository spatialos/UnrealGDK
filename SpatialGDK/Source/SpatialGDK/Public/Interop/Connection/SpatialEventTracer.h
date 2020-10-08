// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "Interop/Connection/SpatialTraceEvent.h"
#include "SpatialView/EntityComponentId.h"

#include <WorkerSDK/improbable/c_io.h>
#include <WorkerSDK/improbable/c_trace.h>

// Documentation for event tracing in the GDK can be found here: https://brevi.link/gdk-event-tracing-documentation

DECLARE_LOG_CATEGORY_EXTERN(LogSpatialEventTracer, Log, All);

// TODO - Individual RPC Calls (distinguishing between GDK and USER)
// TODO - RPCs on newly created objects go through a different flow and need to be handled

namespace SpatialGDK
{
// Note: SpatialEventTracer wraps Trace_EventTracer related functionality
// Note(EventTracer): EventTracer must be created prior to WorkerConnection, since it has to be passed to ConnectionConfig
// It is owned by the connection handler and ViewCoordinator.
// SpatialNetDriver initializes SpatialSender and SpatialReceiver with pointers to EventTracer read from SpatialWorkerConnection.

class SpatialEventTracer
{
public:
	SpatialEventTracer(const FString& WorkerId);
	~SpatialEventTracer();

	const Trace_EventTracer* GetConstWorkerEventTracer() const { return EventTracer; };
	Trace_EventTracer* GetWorkerEventTracer() const { return EventTracer; }

	TOptional<Trace_SpanId> CreateSpan();
	TOptional<Trace_SpanId> CreateSpan(const Trace_SpanId* Causes, int32 NumCauses);
	void TraceEvent(FSpatialTraceEvent SpatialTraceEvent, const TOptional<Trace_SpanId>& OptionalSpanId);

	bool IsEnabled() const;

	void AddEntity(const Worker_Op& Op);
	void RemoveEntity(const Worker_Op& Op);
	void AuthChanged(const Worker_Op& Op);
	void ComponentAdd(const Worker_Op& Op);
	void ComponentRemove(const Worker_Op& Op);
	void ComponentUpdate(const Worker_Op& Op);

	bool GetSpanId(const EntityComponentId& Id, Trace_SpanId& OutSpanId) const;

private:
	struct StreamDeleter
	{
		void operator()(Io_Stream* StreamToDestroy) const;
	};

	static void TraceCallback(void* UserData, const Trace_Item* Item);

	void Enable(const FString& FileName);

	TUniquePtr<Io_Stream, StreamDeleter> Stream;
	Trace_EventTracer* EventTracer = nullptr;

	TMap<EntityComponentId, Trace_SpanId> EntityComponentSpanIds;

	bool bEnabled = false;
	uint64 BytesWrittenToStream = 0;
	uint64 MaxFileSize = 0;
};


// SpatialScopedActiveSpanId must be creating prior to calling worker send functions so that worker can use the input SpanId to continue traces
struct SpatialScopedActiveSpanId
{
	SpatialScopedActiveSpanId(SpatialEventTracer* InEventTracer, const TOptional<Trace_SpanId>& InCurrentSpanId);
	~SpatialScopedActiveSpanId();

	SpatialScopedActiveSpanId(const SpatialScopedActiveSpanId&) = delete;
	SpatialScopedActiveSpanId(SpatialScopedActiveSpanId&&) = delete;
	SpatialScopedActiveSpanId& operator=(const SpatialScopedActiveSpanId&) = delete;
	SpatialScopedActiveSpanId& operator=(SpatialScopedActiveSpanId&&) = delete;

private:
	const TOptional<Trace_SpanId>& CurrentSpanId;
	Trace_EventTracer* EventTracer;
};

} // namespace SpatialGDK
