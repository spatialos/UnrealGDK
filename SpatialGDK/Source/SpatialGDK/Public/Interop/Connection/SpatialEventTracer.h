// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "SpatialCommonTypes.h"

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

struct SpatialEventTracer;

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

struct SpatialGDKEvent
{
	//SpatialSpanId SpanId;
	FString Message;
	FString Type;
	TMap<FString, FString> Data;
};

// TODO: Remove
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

// TODO: is it really necessary?
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
	Trace_SpanId CreateNewSpan();
	void TraceEvent(const SpatialGDKEvent& Event);
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
