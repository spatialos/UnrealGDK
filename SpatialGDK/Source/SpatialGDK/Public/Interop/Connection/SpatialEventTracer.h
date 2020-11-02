// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "Interop/Connection/SpatialTraceEvent.h"
#include "Interop/Connection/UserSpanId.h"
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

	TOptional<Trace_SpanId> CreateSpan() const;
	TOptional<Trace_SpanId> CreateSpan(const Trace_SpanId* Causes, int32 NumCauses) const;
	void TraceEvent(const FSpatialTraceEvent& SpatialTraceEvent, const TOptional<Trace_SpanId>& OptionalSpanId);

	bool IsEnabled() const;

	void AddComponent(Worker_EntityId EntityId, Worker_ComponentId ComponentId, const Trace_SpanId& SpanId);
	void RemoveComponent(Worker_EntityId EntityId, Worker_ComponentId ComponentId);
	void UpdateComponent(Worker_EntityId EntityId, Worker_ComponentId ComponentId, const Trace_SpanId& SpanId);

	TOptional<Trace_SpanId> GetSpanId(const EntityComponentId& Id) const;

	static FString SpanIdToString(const Trace_SpanId& SpanId);

	static FUserSpanId SpanIdToUserSpanId(const Trace_SpanId& SpanId);
	static TOptional<Trace_SpanId> UserSpanIdToSpanId(const FUserSpanId& UserSpanId);

	const FString& GetFolderPath() const { return FolderPath; }

	void AddToStack(const Trace_SpanId& SpanId);
	TOptional<Trace_SpanId> PopFromStack();
	TOptional<Trace_SpanId> GetFromStack() const;
	bool IsStackEmpty() const;

	void AddLatentPropertyUpdateSpanId(const TWeakObjectPtr<UObject>& Object, const Trace_SpanId& SpanId);
	TOptional<Trace_SpanId> PopLatentPropertyUpdateSpanId(const TWeakObjectPtr<UObject>& Object);

private:
	struct StreamDeleter
	{
		void operator()(Io_Stream* StreamToDestroy) const;
	};

	static void TraceCallback(void* UserData, const Trace_Item* Item);

	static const int32 TraceSpanIdLength = 16;

	void Enable(const FString& FileName);

	FString FolderPath;

	TUniquePtr<Io_Stream, StreamDeleter> Stream;
	Trace_EventTracer* EventTracer = nullptr;

	TArray<Trace_SpanId> SpanIdStack;
	TMap<EntityComponentId, Trace_SpanId> EntityComponentSpanIds;
	TMap<TWeakObjectPtr<UObject>, Trace_SpanId> ObjectSpanIdStacks;

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
