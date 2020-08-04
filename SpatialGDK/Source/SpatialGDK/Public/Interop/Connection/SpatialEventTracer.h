// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "UObject/ObjectMacros.h"
#include "UObject/Class.h"
//#include "ObjectMacros.h"
#include "SpatialCommonTypes.h"

// TODO(EventTracer): make sure SpatialEventTracer doesn't break the LatencyTracer functionality for now (maybe have some macro/branching in .cpp file, when the LatencyTracer is enabled?)

// TODO(EventTracer): make sure the overhead of SpatialEventTracer is minimal when it's switched off
// TODO(EventTracer): it is only required here because Trace_SpanId is used.
// Consider if it's possible to remove it.

#include "Containers/Queue.h"
#include <WorkerSDK/improbable/c_worker.h>

#include "SpatialEventTracer.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(LogSpatialEventTracer, Log, All);

//namespace worker {
//namespace c {
//	struct Trace_EventTracer;
//	struct Trace_SpanId;
//}
//}

static_assert(sizeof(Worker_EntityId) == sizeof(int64), "EntityId assumed 64-bit here");
static_assert(sizeof(VirtualWorkerId) == sizeof(uint32), "VirtualWorkerId assumed 32-bit here");

USTRUCT()
struct FEventMessage
{
	GENERATED_BODY()

	FEventMessage() = default;
	FEventMessage(const char* InType)
		: Type(InType)
	{}

	const char* GetType() const { return Type; }

private:

	const char* Type = "Null";
};

USTRUCT()
struct FEventGenericMessage : public FEventMessage
{
	GENERATED_BODY()

	FEventGenericMessage() : FEventMessage("GenericMessage") {}

	UPROPERTY() FString Message;
};

USTRUCT()
struct FEventCreateEntity : public FEventMessage
{
	GENERATED_BODY()

	FEventCreateEntity() : FEventMessage("CreateEntity") {}

	UPROPERTY() int64 EntityId = -1;
	UPROPERTY() const AActor* Actor = nullptr;
};

USTRUCT()
struct FEventRemoveEntity : public FEventMessage
{
	GENERATED_BODY()

	FEventRemoveEntity() : FEventMessage("RemoveEntity") {}

	UPROPERTY() int64 EntityId { -1 };
	UPROPERTY() const AActor* Actor { nullptr };
};

USTRUCT()
struct FEventCreateEntitySuccess : public FEventMessage
{
	GENERATED_BODY()

	FEventCreateEntitySuccess() : FEventMessage("CreateEntitySuccess") {}

	UPROPERTY() int64 EntityId { -1 };
	UPROPERTY() const AActor* Actor { nullptr };
};

USTRUCT()
struct FEventAuthorityIntentUpdate : public FEventMessage
{
	GENERATED_BODY()

	FEventAuthorityIntentUpdate() : FEventMessage("AuthorityIntentUpdate") {}

	UPROPERTY() uint32 NewWorkerId { 0xFFFFFFFF };
	UPROPERTY() const AActor* Actor { nullptr };
};

USTRUCT()
struct FEventAuthorityLossImminent : public FEventMessage
{
	GENERATED_BODY()

	FEventAuthorityLossImminent() : FEventMessage("AuthorityLossImminent") {}

	UPROPERTY() TEnumAsByte<ENetRole> Role{ 0 };
	UPROPERTY() const AActor* Actor { nullptr };
};

USTRUCT()
struct FEventRetireEntityRequest : public FEventMessage
{
	GENERATED_BODY()

	FEventRetireEntityRequest() : FEventMessage("EntityRetire") {}

	UPROPERTY() int64 EntityId { -1 };
	UPROPERTY() const AActor* Actor { nullptr };
};

USTRUCT()
struct FEventSendRPC : public FEventMessage
{
	GENERATED_BODY()

	FEventSendRPC() : FEventMessage("SendRPC") {}

	UPROPERTY() const UObject* TargetObject { nullptr };
	UPROPERTY() const UFunction* Function { nullptr };
};

USTRUCT()
struct FEventRPCQueued : public FEventMessage
{
	GENERATED_BODY()

	FEventRPCQueued() : FEventMessage("RPCQueued") {}

	UPROPERTY() const UObject* TargetObject { nullptr };
	UPROPERTY() const UFunction* Function { nullptr };
};

USTRUCT()
struct FEventRPCRetried : public FEventMessage
{
	GENERATED_BODY()

	FEventRPCRetried() : FEventMessage("RPCRetried") {}

	UPROPERTY() const UObject* TargetObject { nullptr };
	UPROPERTY() const UFunction* Function { nullptr };
};

USTRUCT()
struct FEventComponentUpdate : public FEventMessage
{
	GENERATED_BODY()

	FEventComponentUpdate() : FEventMessage("ComponentUpdate") {}

	UPROPERTY() const AActor* Actor { nullptr };
	UPROPERTY() const UObject* TargetObject { nullptr };
	UPROPERTY() uint32 ComponentId { 0 };
};

USTRUCT()
struct FEventCommandResponse : public FEventMessage
{
	GENERATED_BODY()

	FEventCommandResponse() : FEventMessage("CommandResponse") {}

	UPROPERTY() FString Command;
	UPROPERTY() const AActor* Actor { nullptr };
	UPROPERTY() const UObject* TargetObject { nullptr };
	UPROPERTY() const UFunction* Function { nullptr };
	UPROPERTY() int64 RequestID { -1 };
	UPROPERTY() bool bSuccess;
};

USTRUCT()
struct FEventCommandRequest : public FEventMessage
{
	GENERATED_BODY()

	FEventCommandRequest() : FEventMessage("CommandRequest") {}

	UPROPERTY() FString Command;
	UPROPERTY() const AActor* Actor { nullptr };
	UPROPERTY() const UObject* TargetObject { nullptr };
	UPROPERTY() const UFunction* Function { nullptr };
	UPROPERTY() int32 TraceId { -1 };
	UPROPERTY() int64 RequestID { -1 };
};

/*
TODO
[+] Sending create entity request
[+] Sending authority intent update
[+] Sending delete entity request
[+] Sending RPC
[+] Sending RPC retry - requires testing
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

class AActor;
class UFunction;
class USpatialNetDriver;

// Note(EventTracer): EventTracer must be created prior to WorkerConnection, since it has to be passed to ConnectionConfig
// (see SpatialConnectionManager diff)

// TODO(EventTracer): consider whether it's necessary to create a SpatialSpanId wrapper that holds Trace_SpanId,
// so that Trace_SpanId is not used directly in UnrealGDK

namespace worker
{
namespace c
{
	struct Io_Stream;
}
}

namespace SpatialGDK
{

// Note: SpatialEventTracer wraps Trace_EventTracer related functionality
// It is constructed and owned by SpatialConnectionManager.
// SpatialNetDriver initializes SpatialSender and SpatialReceiver with pointers to EventTracer read from SpatialConnectionManager.
// Note(EventTracer): SpatialEventTracer is supposed to never be null in SpatialWorkerConnection, SpatialSender, SpatialReceiver. Make sure there are necessary nullptr checks if that changes.

struct SpatialEventTracer
{
	SpatialEventTracer(UWorld* World);
	~SpatialEventTracer();
	Trace_SpanId CreateNewSpanId();
	Trace_SpanId CreateNewSpanId(const TArray<Trace_SpanId>& Causes);

	void Enable();
	void Disable();
	bool IsEnabled() { return bEnalbed; }

	const worker::c::Trace_EventTracer* GetConstWorkerEventTracer() const { return EventTracer; };
	worker::c::Trace_EventTracer* GetWorkerEventTracer() const { return EventTracer; }

	USpatialNetDriver* GetNetDriver() const { return NetDriver; }

	// TODO(EventTracer): add the option to SpatialSpanIdActivator for Sent TraceEvents.
	// Consider making sure it's not accepting rvalue (since SpatialSpanIdActivator must live long enough for the worker sent op to be registered with this SpanId)
	// e.g. void TraceEvent(... SpatialSpanIdActivator&& SpanIdActivator) = delete;
	// TODO(EventTracer): Communicate to others, that SpatialSpanIdActivator must be creating prior to calling worker send functions

	template<class T>
	TOptional<Trace_SpanId> TraceEvent(const T& EventMessage, const worker::c::Trace_SpanId* Cause = nullptr)
	{
		return TraceEvent(EventMessage, T::StaticStruct(), Cause);
	}

	TOptional<Trace_SpanId> TraceEvent(const FEventMessage& EventMessage, UStruct* Struct, const worker::c::Trace_SpanId* Cause);

	using EventTracingData = TMap<FString, FString>;

	void Start();
	void WriteEventDataToJson(const EventTracingData& EventData);

private:
	bool bEnalbed{ true }; // TODO: Disable by default
	worker::c::Io_Stream* Stream;
	worker::c::Trace_EventTracer* EventTracer;
	USpatialNetDriver* NetDriver;
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
