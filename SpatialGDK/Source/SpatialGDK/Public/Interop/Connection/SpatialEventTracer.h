// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "Interop/Connection/SpatialGDKSpanId.h"
#include "Interop/Connection/SpatialTraceEventDataBuilder.h"
#include "Interop/Connection/UserSpanId.h"
#include "SpatialCommonTypes.h"
#include "SpatialView/EntityComponentId.h"

#include <WorkerSDK/improbable/c_io.h>
#include <WorkerSDK/improbable/c_trace.h>

#include "SpatialEventTracer.generated.h"

// Documentation for event tracing in the GDK can be found here: https://brevi.link/gdk-event-tracing-documentation
DECLARE_LOG_CATEGORY_EXTERN(LogSpatialEventTracer, Log, All);

UENUM()
namespace EEventTraceFileOutputType
{
enum Type
{
	Single,
	Rotating
};
}

namespace SpatialGDK
{
struct TraceQueryDeleter
{
	void operator()(Trace_Query* Query) const
	{
		if (Query)
		{
			Trace_Query_Destroy(Query);
		}
	}
};
typedef TUniquePtr<Trace_Query, TraceQueryDeleter> TraceQueryPtr;

// SpatialEventTracer wraps Trace_EventTracer related functionality
class SPATIALGDK_API SpatialEventTracer
{
public:
	explicit SpatialEventTracer(const FString& WorkerId);
	~SpatialEventTracer();

	const Trace_EventTracer* GetConstWorkerEventTracer() const { return EventTracer; }
	Trace_EventTracer* GetWorkerEventTracer() const { return EventTracer; }

	template <typename T>
	FSpatialGDKSpanId TraceEvent(const char* EventType, const char* Message, const Trace_SpanIdType* Causes, int32 NumCauses,
								 T&& DataCallback) const;

	void BeginOpsForFrame();
	void AddEntity(const Worker_AddEntityOp& Op, const FSpatialGDKSpanId& SpanId);
	void RemoveEntity(const Worker_RemoveEntityOp& Op, const FSpatialGDKSpanId& SpanId);
	void AuthorityChange(const Worker_ComponentSetAuthorityChangeOp& Op, const FSpatialGDKSpanId& SpanId);
	void AddComponent(const Worker_AddComponentOp& Op, const FSpatialGDKSpanId& SpanId);
	void RemoveComponent(const Worker_RemoveComponentOp& Op, const FSpatialGDKSpanId& SpanId);
	void UpdateComponent(const Worker_ComponentUpdateOp& Op, const FSpatialGDKSpanId& SpanId);
	void CommandRequest(const Worker_CommandRequestOp& Op, const FSpatialGDKSpanId& SpanId);

	TArray<FSpatialGDKSpanId> GetAndConsumeSpansForComponent(const EntityComponentId& Id);
	FSpatialGDKSpanId GetAndConsumeSpanForRequestId(Worker_RequestId RequestId);

	static FUserSpanId GDKSpanIdToUserSpanId(const FSpatialGDKSpanId& SpanId);
	static FSpatialGDKSpanId UserSpanIdToGDKSpanId(const FUserSpanId& UserSpanId);

	const FString& GetFolderPath() const { return FolderPath; }

	void AddToStack(const FSpatialGDKSpanId& SpanId);
	FSpatialGDKSpanId PopFromStack();
	FSpatialGDKSpanId GetFromStack() const;
	bool IsStackEmpty() const;

	void AddLatentPropertyUpdateSpanId(const TWeakObjectPtr<UObject>& Object, const FSpatialGDKSpanId& SpanId);
	FSpatialGDKSpanId PopLatentPropertyUpdateSpanId(const TWeakObjectPtr<UObject>& Object);

	void SetFlushOnWrite(bool bValue);

private:
	struct StreamDeleter
	{
		void operator()(Io_Stream* StreamToDestroy) const
		{
			if (StreamToDestroy)
			{
				Io_Stream_Destroy(StreamToDestroy);
			}
		}
	};

	static void TraceCallback(void* UserData, const Trace_Item* Item);

	FString FolderPath;

	int32 FlushOnWriteAtomic = 0;
	TUniquePtr<Io_Stream, StreamDeleter> Stream;
	Trace_EventTracer* EventTracer = nullptr;

	TArray<FSpatialGDKSpanId> SpanIdStack;
	TMap<TWeakObjectPtr<UObject>, FSpatialGDKSpanId> ObjectSpanIdStacks;

	uint64 BytesWrittenToStream = 0;
	uint64 MaxFileSize = 0;

	// Span IDs received from the wire, these live for a frame and are expected to continue into the stack
	// on an ops tick.
	TMap<EntityComponentId, TArray<FSpatialGDKSpanId>> EntityComponentSpanIds;
	TArray<EntityComponentId> EntityComponentsConsumed;
	TMap<int64, FSpatialGDKSpanId> RequestSpanIds;
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

template <typename T>
FSpatialGDKSpanId SpatialEventTracer::TraceEvent(const char* EventType, const char* Message, const Trace_SpanIdType* Causes,
												 int32 NumCauses, T&& DataCallback) const
{
	if (Causes == nullptr && NumCauses > 0)
	{
		return {};
	}

	// We could add the data to this event if a custom sampling callback was used.
	// This would allow for sampling dependent on trace event data.
	Trace_Event Event = { nullptr, 0, Message, EventType, nullptr };

	if (!Trace_EventTracer_ShouldSampleSpan(EventTracer, Causes, NumCauses, &Event))
	{
		return {};
	}

	FSpatialGDKSpanId TraceSpanId;
	Trace_EventTracer_AddSpan(EventTracer, Causes, NumCauses, &Event, TraceSpanId.GetId());
	Event.span_id = TraceSpanId.GetConstId();

	if (!Trace_EventTracer_ApplyEventPreFilter(EventTracer, &Event))
	{
		return {}; // TODO: Should return TraceSpanId?
	}

	FSpatialTraceEventDataBuilder EventDataBuilder;
	DataCallback(EventDataBuilder);

	// Frame counter
	EventDataBuilder.AddKeyValue("frame_num", GFrameCounter);

	Event.data = EventDataBuilder.GetEventData();
	Trace_EventTracer_AddEvent(EventTracer, &Event);
	return TraceSpanId;
}

} // namespace SpatialGDK
