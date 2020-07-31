// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "SpatialCommonTypes.h"

// TODO(EventTracer): it is only required here because Trace_SpanId is used.
// Consider if it's possible to remove it.
#include <WorkerSDK/improbable/c_worker.h>

DECLARE_LOG_CATEGORY_EXTERN(LogSpatialEventTracer, Log, All);

class UFunction;
class AActor;

namespace SpatialGDK
{

struct SpatialEventTracer;

// TODO(EventTracer): consider whether it's necessary to create a SpatialSpanId wrapper that holds Trace_SpanId,
// so that Trace_SpanId is not used directly in UnrealGDK

struct SpatialSpanIdActivator
{
	SpatialSpanIdActivator(SpatialEventTracer* InEventTracer, Trace_SpanId CurrentSpanId);

	SpatialSpanIdActivator(const SpatialSpanIdActivator&) = delete;
	SpatialSpanIdActivator(SpatialSpanIdActivator&&) = delete;
	SpatialSpanIdActivator& operator=(const SpatialSpanIdActivator&) = delete;
	SpatialSpanIdActivator& operator=(SpatialSpanIdActivator&&) = delete;

	~SpatialSpanIdActivator();

private:
	worker::c::Trace_EventTracer* EventTracer;
};

// TODO(EventTracer): Remove SpatialGDKEvent
struct SpatialGDKEvent
{
	FString Message;
	FString Type;
	TMap<FString, FString> Data;
};

// TODO(EventTracer): Remove ConstructEvent functions, write data with TraceEvent directly instead
SpatialGDKEvent ConstructEvent(const AActor* Actor, VirtualWorkerId NewAuthoritativeWorkerId);
SpatialGDKEvent ConstructEvent(const AActor* Actor, Worker_EntityId EntityId, Worker_RequestId RequestID);
SpatialGDKEvent ConstructEvent(const AActor* Actor, const FString& Type, Worker_CommandResponseOp ResponseOp);
SpatialGDKEvent ConstructEvent(const AActor* Actor, const UObject* TargetObject, const UFunction* Function, TraceKey TraceId, Worker_RequestId RequestID);
SpatialGDKEvent ConstructEvent(const AActor* Actor, const FString& Message, Worker_CreateEntityResponseOp ResponseOp);
SpatialGDKEvent ConstructEvent(const AActor* Actor, const UObject* TargetObject, const UFunction* Function, Worker_CommandResponseOp ResponseOp);

enum class EventType
{
	Received,
	Sent,
};

enum class EventName
{
	AuthorityChange,
	CommandFailure,
	CommandRequest,
	CommandResponse,
	ComponentUpdate,
	CreateEntity,
	RPC,
};

// TODO(EventTracer): decide whether that enum is useful
enum class CommandType
{
	ClearRPCsOnEntityCreation,
	ServerWorkerForwardSpawnRequest,
	ShutdownMultiProcess,
	SpawnPlayer,
};

struct SpatialEventTracer
{
	SpatialEventTracer();
	~SpatialEventTracer();
	Trace_SpanId CreateNewSpanId();
	Trace_SpanId CreateNewSpanId(const TArray<Trace_SpanId>& Causes);
	
	void TraceEvent(const SpatialGDKEvent& Event);

	// TODO(EventTracer): consider using this to reduce code duplication.
	// Maybe it's worth moving it to a separate utility h/cpp along with related functionality
	/*
	template<typename ... TArgs>
	void AddDataToEventData(Trace_EventData* EventData, TArgs&& ... Args)
	{
		// call AddRoleInfoToEventData, AddActorInfoToEventData etc.
	}

	template<typename ... TArgs>
	void TraceEvent(EventName Name, EventType Type, TArgs&& ... Args)
	{
		Trace_SpanId CurrentSpanId = CreateNewSpanId();
		//Trace_Event TraceEvent{ CurrentSpanId, 0, TypeToString(Type), NameToString(Name), nullptr };
		Trace_Event TraceEvent{ CurrentSpanId, 0, "SomeType", "SomeName", nullptr };
		if (Trace_EventTracer_ShouldSampleEvent(EventTracer, &TraceEvent))
		{
			Trace_EventData* EventData = Trace_EventData_Create();
			AddDataToEventData(EventData, Forward<TArgs>(Args)...);
			TraceEvent.data = EventData;
			Trace_EventTracer_AddEvent(EventTracer, &TraceEvent);
			Trace_EventData_Destroy(EventData);
		}
	}
	*/

	// TODO(EventTracer): add more TraceEvent here
	// TODO(EventTracer): add the option to SpatialSpanIdActivator for Sent TraceEvents.
	// Consider making sure it's not accepting rvalue (since SpatialSpanIdActivator must live long enough for the worker sent op to be registered with this SpanId)
	// e.g. void TraceEvent(... SpatialSpanIdActivator&& SpanIdActivator) = delete;
	// TODO(EventTracer): Communicate to others, that SpatialSpanIdActivator must be creating prior to calling worker send functions
	// TODO(EventTracer): Make sure that in SpatialSender/SpatialReceiver these functions are called properly and at proper moments
	// (So far they have been added to the same places as in Jose's event tracing branch)
	void TraceEvent(EventName Name, EventType Type, Worker_RequestId CommandResponseId, CommandType Command);
	void TraceEvent(EventName Name, EventType Type, Worker_RequestId CommandResponseId, bool bSuccess);
	void TraceEvent(EventName Name, EventType Type, const AActor* Actor, ENetRole Role);
	void TraceEvent(EventName Name, EventType Type, const AActor* Actor, Worker_RequestId CreateEntityRequestId);
	void TraceEvent(EventName Name, EventType Type, const AActor* Actor, const UFunction* Function);
	void TraceEvent(EventName Name, EventType Type, const AActor* Actor, const UObject* TargetObject, Worker_ComponentId ComponentId);

	void Enable();
	void Disable();
	const worker::c::Trace_EventTracer* GetWorkerEventTracer() const;
	worker::c::Trace_EventTracer* GetWorkerEventTracer();

private:
	worker::c::Trace_EventTracer* EventTracer;
};

}

// TODO(EventTracer): (a list of requirements by Chris from Jira ticket)
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

// TODO(EventTracer): a short list of requirements by Alex
/*
Actor name, Position,
Add/Remove Entity (can we also distinguish Remove Entity when moving to another worker vs Delete entity),
Authority/Authority intent changes,
RPC calls (when they were sent/received/processed),
Component Updates
*/
