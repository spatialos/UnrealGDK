// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "UObject/ObjectMacros.h"
#include "UObject/Class.h"
//#include "ObjectMacros.h"
#include "SpatialCommonTypes.h"

// TODO Remove maybe?
#include <WorkerSDK/improbable/c_worker.h>

#include "SpatialEventTracer.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(LogSpatialEventTracer, Log, All);

//namespace worker {
//namespace c {
//	struct Trace_EventTracer;
//	struct Trace_SpanId;
//}
//}

// TODO: Hook up to build system
#define GDK_SPATIAL_EVENT_TRACING_ENABLED 1

static_assert(sizeof(Worker_EntityId) == sizeof(int64), "EntityId assumed 64-bit here");
static_assert(sizeof(VirtualWorkerId) == sizeof(uint32), "VirtualWorkerId assumed 32-bit here");

USTRUCT()
struct FEventCreateEntity
{
	GENERATED_BODY()
	UPROPERTY() int64 EntityId;
	UPROPERTY() const AActor* Actor;
	const char* Type = "CreateEntity";
};

USTRUCT()
struct FEventCreateEntitySuccess
{
	GENERATED_BODY()
	UPROPERTY() int64 EntityId;
	UPROPERTY() const AActor* Actor;
	const char* Type = "CreateEntity";
};

USTRUCT()
struct FEventAuthorityIntentUpdate
{
	GENERATED_BODY()
	UPROPERTY() uint32 NewWorkerId;
	UPROPERTY() const AActor* Actor;
	const char* Type = "AuthorityIntentUpdate";
};

USTRUCT()
struct FEventRetireEntityRequest
{
	GENERATED_BODY()
	UPROPERTY() int64 EntityId;
	UPROPERTY() const AActor* Actor;
	const char* Type = "EntityRetire";
};

USTRUCT()
struct FEventDeleteEntityRequest
{
	GENERATED_BODY()
	UPROPERTY() int64 EntityId;
	UPROPERTY() const AActor* Actor;
	const char* Type = "EntityDelete";
};

/*
TODO
[+] Sending create entity request
[+] Sending authority intent update
[+] Sending delete entity request
[+] Sending RPC
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

class UFunction;
class AActor;

namespace SpatialGDK
{

struct SpatialSpanId
{
	SpatialSpanId(worker::c::Trace_EventTracer* InEventTracer);
	~SpatialSpanId();

private:
	Trace_SpanId CurrentSpanId;
	worker::c::Trace_EventTracer* EventTracer;
};

struct SpatialGDKEvent
{
	//SpatialSpanId SpanId;
	FString Message;
	FString Type;
	TMap<FString, FString> Data;
};

// TODO: discuss overhead from constructing SpatialGDKEvents
// TODO: Rename
SpatialGDKEvent ConstructEvent(const AActor* Actor, const UFunction* Function);
SpatialGDKEvent ConstructEvent(const AActor* Actor, ENetRole Role);
SpatialGDKEvent ConstructEvent(const AActor* Actor, const UObject* TargetObject, Worker_ComponentId ComponentId);
SpatialGDKEvent ConstructEvent(const AActor* Actor, Worker_RequestId CreateEntityRequestId);
SpatialGDKEvent ConstructEvent(const AActor* Actor, Worker_EntityId EntityId, Worker_RequestId RequestID);
SpatialGDKEvent ConstructEvent(const AActor* Actor, const FString& Type, Worker_RequestId RequestID);
SpatialGDKEvent ConstructEvent(const AActor* Actor, const FString& Type, Worker_CommandResponseOp ResponseOp);
SpatialGDKEvent ConstructEvent(const AActor* Actor, const UObject* TargetObject, const UFunction* Function, TraceKey TraceId, Worker_RequestId RequestID);
SpatialGDKEvent ConstructEvent(const AActor* Actor, const FString& Message, Worker_CreateEntityResponseOp ResponseOp);
SpatialGDKEvent ConstructEvent(const AActor* Actor, const UObject* TargetObject, const UFunction* Function, Worker_CommandResponseOp ResponseOp);
SpatialGDKEvent ConstructEvent(Worker_RequestId RequestID, bool bSuccess);

struct SpatialEventTracer
{
	SpatialEventTracer();
	~SpatialEventTracer();
	SpatialSpanId CreateActiveSpan();
	TOptional<Trace_SpanId> TraceEvent(const SpatialGDKEvent& Event);

	void Enable();
	void Disable();
	bool IsEnabled() { return bEnalbed; }
	worker::c::Trace_EventTracer* GetWorkerEventTracer() { return EventTracer; }

	/*TOptional<Trace_SpanId> CreateEntity(AActor* Actor, Worker_EntityId EntityId)
	{
		SpatialGDKEvent Event;
		Event.Message = "";
		Event.Type = "CreateEntity";
		if (Actor != nullptr)
		{
			Event.Data.Add("Actor", Actor->GetName());
			Event.Data.Add("Position", Actor->GetActorTransform().GetTranslation().ToString());
		}
		else
		{
			Event.Data.Add("Actor", "Null");
		}
		//Event.Data.Add("ResponseOp", FString::Printf(TEXT("%lu"), ResponseOp));
		return TraceEvent(Event);
	}
	TOptional<Trace_SpanId> CreateEntitySuccess(AActor* Actor, Worker_EntityId EntityId)
	{
		SpatialGDKEvent Event;
		Event.Message = "";
		Event.Type = "CreateEntitySuccess";
		if (Actor != nullptr)
		{
			Event.Data.Add("Actor", Actor->GetName());
			Event.Data.Add("Position", Actor->GetActorTransform().GetTranslation().ToString());
		}
		else
		{
			Event.Data.Add("Actor", "Null");
		}
		//Event.Data.Add("ResponseOp", FString::Printf(TEXT("%lu"), ResponseOp));
		return TraceEvent(Event);
	}*/

	template<typename T>
	TOptional<Trace_SpanId> TraceEvent2(const T& Message)
	{
		if (!IsEnabled())
		{
			return {};
		}
		
		SpatialGDKEvent Event;
		Event.Type = Message.Type; // This is expected

		for (TFieldIterator<UProperty> It(T::StaticStruct()); It; ++It)
		{
			UProperty* Property = *It;

			FString VariableName = Property->GetName();
			const void* Value = Property->ContainerPtrToValuePtr<uint8>(&Message);

			check(Property->ArrayDim == 1); // Arrays not handled yet
			
			// convert the property to a FJsonValue
			if (UStrProperty *StringProperty = Cast<UStrProperty>(Property))
			{
				Event.Data.Add(VariableName, StringProperty->GetPropertyValue(Value));
			}
			else if (UObjectProperty* ObjectProperty = Cast<UObjectProperty>(Property))
			{
				UObject* Object = ObjectProperty->GetPropertyValue(Value);
				if(Object)
				{
					Event.Data.Add(VariableName, Object->GetName());
					if (AActor* Actor = Cast<AActor>(Object))
					{
						Event.Data.Add(VariableName + TEXT("Position"), Actor->GetTransform().GetTranslation().ToString());
					}
				}
				else
				{
					Event.Data.Add(VariableName, "Null");
				}
			}
			else // Default
			{
				FString StringValue;
				Property->ExportTextItem(StringValue, Value, NULL, NULL, PPF_None);
				Event.Data.Add(VariableName, StringValue);
			}
			// else if (UEnumProperty* EnumProperty = Cast<UEnumProperty>(Property))
			// else if (UNumericProperty *NumericProperty = Cast<UNumericProperty>(Property))
			// else if (UBoolProperty *BoolProperty = Cast<UBoolProperty>(Property))
			// else if (UTextProperty *TextProperty = Cast<UTextProperty>(Property))
			// else if (UArrayProperty *ArrayProperty = Cast<UArrayProperty>(Property))
			// else if (USetProperty* SetProperty = Cast<USetProperty>(Property))
			// else if (UMapProperty* MapProperty = Cast<UMapProperty>(Property))
			// else if (UStructProperty *StructProperty = Cast<UStructProperty>(Property))
		}
		return TraceEvent(Event);
	}
private:
	bool bEnalbed{ true }; // TODO: Disable by default
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
