// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "Interop/Connection/SpatialTraceEvent.h"
#include "SpatialView/EntityComponentId.h"

#include <WorkerSDK/improbable/c_io.h>
#include <WorkerSDK/improbable/c_trace.h>

// Documentation for event tracing in the GDK can be found here: https://brevi.link/gdk-event-tracing-documentation

DECLARE_LOG_CATEGORY_EXTERN(LogSpatialEventTracer, Log, All);

namespace SpatialGDK
{
// SpatialEventTracer wraps Trace_EventTracer related functionality
class SPATIALGDK_API SpatialEventTracer
{
public:
	explicit SpatialEventTracer(const FString& WorkerId);
	~SpatialEventTracer();

	const Trace_EventTracer* GetConstWorkerEventTracer() const { return EventTracer; };
	Trace_EventTracer* GetWorkerEventTracer() const { return EventTracer; }

	TOptional<Trace_SpanId> CreateSpan();
	TOptional<Trace_SpanId> CreateSpan(const Trace_SpanId* Causes, int32 NumCauses);
	void TraceEvent(FSpatialTraceEvent SpatialTraceEvent, const TOptional<Trace_SpanId>& OptionalSpanId);

	bool IsEnabled() const;

	void AddComponent(const Worker_Op& Op);
	void RemoveComponent(const Worker_Op& Op);
	void UpdateComponent(const Worker_Op& Op);

	Trace_SpanId GetSpanId(const EntityComponentId& Id) const;

	const FString& GetFolderPath() const { return FolderPath; }

	static FString SpanIdToString(const Trace_SpanId& SpanId);

private:
	struct StreamDeleter
	{
		void operator()(Io_Stream* StreamToDestroy) const;
	};

	static void TraceCallback(void* UserData, const Trace_Item* Item);

	void Enable(const FString& FileName);

	FString FolderPath;

	TUniquePtr<Io_Stream, StreamDeleter> Stream;
	Trace_EventTracer* EventTracer = nullptr;

	TMap<EntityComponentId, Trace_SpanId> EntityComponentSpanIds;

	bool bEnabled = false;
	uint64 BytesWrittenToStream = 0;
	uint64 MaxFileSize = 0;
};

// SpatialScopedActiveSpanIds are creating prior to calling worker send functions so that worker can use the input SpanId to continue
// traces.
struct SpatialScopedActiveSpanId
{
	explicit SpatialScopedActiveSpanId(SpatialEventTracer* InEventTracer, const TOptional<Trace_SpanId>& InCurrentSpanId);
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
