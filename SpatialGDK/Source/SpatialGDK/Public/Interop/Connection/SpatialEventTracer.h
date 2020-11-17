// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "Interop/Connection/SpatialGDKSpanId.h"
#include "Interop/Connection/SpatialTraceEvent.h"
#include "Interop/Connection/UserSpanId.h"
#include "SpatialCommonTypes.h"
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

	FSpatialGDKSpanId CreateSpan(const Trace_SpanIdType* Causes = nullptr, int32 NumCauses = 0) const;
	void TraceEvent(const FSpatialTraceEvent& SpatialTraceEvent, const FSpatialGDKSpanId& SpanId);
	FSpatialGDKSpanId TraceFilterableEvent(const FSpatialTraceEvent& SpatialTraceEvent, const Trace_SpanIdType* Causes = nullptr, int32 NumCauses = 0);

	bool IsEnabled() const;

	void AddComponent(Worker_EntityId EntityId, Worker_ComponentId ComponentId, const FSpatialGDKSpanId& SpanId);
	void RemoveComponent(Worker_EntityId EntityId, Worker_ComponentId ComponentId);
	void UpdateComponent(Worker_EntityId EntityId, Worker_ComponentId ComponentId, const FSpatialGDKSpanId& SpanId);

	FSpatialGDKSpanId GetSpanId(const EntityComponentId& Id) const;

	static FUserSpanId GDKSpanIdToUserSpanId(const FSpatialGDKSpanId& SpanId);
	static FSpatialGDKSpanId UserSpanIdToGDKSpanId(const FUserSpanId& UserSpanId);

	const FString& GetFolderPath() const { return FolderPath; }

	void AddToStack(const FSpatialGDKSpanId& SpanId);
	FSpatialGDKSpanId PopFromStack();
	FSpatialGDKSpanId GetFromStack() const;
	bool IsStackEmpty() const;

	void AddLatentPropertyUpdateSpanId(const TWeakObjectPtr<UObject>& Object, const FSpatialGDKSpanId& SpanId);
	FSpatialGDKSpanId PopLatentPropertyUpdateSpanId(const TWeakObjectPtr<UObject>& Object);

private:
	struct StreamDeleter
	{
		void operator()(Io_Stream* StreamToDestroy) const;
	};

	static void TraceCallback(void* UserData, const Trace_Item* Item);

	void Enable(const FString& FileName);
	static void ConstructTraceEventData(Trace_EventData* EventData, const FSpatialTraceEvent& SpatialTraceEvent);

	FString FolderPath;

	TUniquePtr<Io_Stream, StreamDeleter> Stream;
	Trace_EventTracer* EventTracer = nullptr;

	TArray<FSpatialGDKSpanId> SpanIdStack;
	TMap<EntityComponentId, FSpatialGDKSpanId> EntityComponentSpanIds;
	TMap<TWeakObjectPtr<UObject>, FSpatialGDKSpanId> ObjectSpanIdStacks;

	bool bEnabled = false;
	uint64 BytesWrittenToStream = 0;
	uint64 MaxFileSize = 0;
};

// SpatialScopedActiveSpanIds are creating prior to calling worker send functions so that worker can use the input SpanId to continue
// traces.
struct SpatialScopedActiveSpanId
{
	explicit SpatialScopedActiveSpanId(SpatialEventTracer* InEventTracer, const FSpatialGDKSpanId& InCurrentSpanId);
	~SpatialScopedActiveSpanId();

	SpatialScopedActiveSpanId(const SpatialScopedActiveSpanId&) = delete;
	SpatialScopedActiveSpanId(SpatialScopedActiveSpanId&&) = delete;
	SpatialScopedActiveSpanId& operator=(const SpatialScopedActiveSpanId&) = delete;
	SpatialScopedActiveSpanId& operator=(SpatialScopedActiveSpanId&&) = delete;

private:
	const FSpatialGDKSpanId& CurrentSpanId;
	Trace_EventTracer* EventTracer;
};

} // namespace SpatialGDK
