// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "Interop/Connection/SpatialEventTracer.h"

#include "UObject/Class.h"
#include "GameFramework/Actor.h"

#include <WorkerSDK/improbable/c_trace.h>

DEFINE_LOG_CATEGORY(LogSpatialEventTracer);

using namespace SpatialGDK;
using namespace worker::c;

void MyTraceCallback(void* UserData, const Trace_Item* Item)
{
	switch (Item->item_type)
	{
	case TRACE_ITEM_TYPE_EVENT:
	{
		const Trace_Event& Event = Item->item.event;

		// TODO: remove temporary filtering?
		if (Event.type == FString("network.receive_raw_message") ||
			Event.type == FString("network.receive_udp_datagram") ||
			Event.type == FString("network.send_raw_message") ||
			Event.type == FString("network.send_udp_datagram") ||
			Event.type == FString("worker.dequeue_op") ||
			Event.type == FString("worker.enqueue_op"))
		{
			return;
		}

		unsigned long int span1 = *reinterpret_cast<const unsigned long int*>(&Event.span_id.data[0]);
		unsigned long int span2 = *reinterpret_cast<const unsigned long int*>(&Event.span_id.data[8]);
		UE_LOG(LogSpatialEventTracer, Warning, TEXT("Span: %ul%ul, Type: %s, Message: %s, Timestamp: %ul"),
			span1, span2, *FString(Event.type), *FString(Event.message), Event.unix_timestamp_millis);

		if (Event.data != nullptr)
		{
			uint32_t DataFieldCount = Trace_EventData_GetFieldCount(Event.data);

			TArray<const char*> Keys;
			Keys.SetNumUninitialized(DataFieldCount);
			TArray<const char*> Values;
			Values.SetNumUninitialized(DataFieldCount);

			Trace_EventData_GetStringFields(Event.data, Keys.GetData(), Values.GetData());
			for (uint32_t i = 0; i < DataFieldCount; ++i)
			{
				UE_LOG(LogSpatialEventTracer, Warning, TEXT("%s : %s"), ANSI_TO_TCHAR(Keys[i]), ANSI_TO_TCHAR(Values[i]));
			}
		}

		break;
	}
	//case TRACE_ITEM_TYPE_SPAN:
	//{
	//	const Trace_Span& Span = Item->item.span;
	//	//UE_LOG(LogSpatialEventTracer, Warning, TEXT("Span: %s"), *FString(Span.id.data));
	//	unsigned long int span1 = *reinterpret_cast<const unsigned long int*>(&Span.id.data[0]);
	//	unsigned long int span2 = *reinterpret_cast<const unsigned long int*>(&Span.id.data[8]);
	//	UE_LOG(LogSpatialEventTracer, Warning, TEXT("Span: %ul%ul"), span1, span2);
	//	break;
	//}
	default:
	{
		break;
	}
	}
}

SpatialSpanId::SpatialSpanId(Trace_EventTracer* InEventTracer)
	: CurrentSpanId{}
	, EventTracer(InEventTracer)
{
	CurrentSpanId = Trace_EventTracer_AddSpan(EventTracer, nullptr, 0);
	Trace_EventTracer_SetActiveSpanId(EventTracer, CurrentSpanId);
}

SpatialSpanId::~SpatialSpanId()
{
	Trace_EventTracer_UnsetActiveSpanId(EventTracer);
}

SpatialEventTracer::SpatialEventTracer()
{
	Trace_EventTracer_Parameters parameters = {};
	parameters.callback = MyTraceCallback;
	EventTracer = Trace_EventTracer_Create(&parameters);
	Trace_EventTracer_Enable(EventTracer);
}

SpatialEventTracer::~SpatialEventTracer()
{
	Trace_EventTracer_Disable(EventTracer);
	Trace_EventTracer_Destroy(EventTracer);
}

SpatialSpanId SpatialEventTracer::CreateActiveSpan()
{
	return SpatialSpanId(EventTracer);
}

const Trace_EventTracer* SpatialEventTracer::GetWorkerEventTracer() const
{
	return EventTracer;
}

void SpatialEventTracer::TraceEvent(const SpatialGDKEvent& Event)
{
	Trace_SpanId CurrentSpanId = Trace_EventTracer_AddSpan(EventTracer, nullptr, 0);
	Trace_Event TraceEvent{ CurrentSpanId, 0, TCHAR_TO_ANSI(*Event.Message), TCHAR_TO_ANSI(*Event.Type), nullptr };

	if (Trace_EventTracer_ShouldSampleEvent(EventTracer, &TraceEvent))
	{
		Trace_EventData* EventData = Trace_EventData_Create();
		for (const auto& Elem : Event.Data)
		{
			const char* Key = TCHAR_TO_ANSI(*Elem.Key);
			const char* Value = TCHAR_TO_ANSI(*Elem.Value);
			Trace_EventData_AddStringFields(EventData, 1, &Key, &Value);
		}
		TraceEvent.data = EventData;
		Trace_EventTracer_AddEvent(EventTracer, &TraceEvent);
		Trace_EventData_Destroy(EventData);
	}
}

void SpatialEventTracer::Enable()
{
	Trace_EventTracer_Enable(EventTracer);
}

void SpatialEventTracer::Disable()
{
	Trace_EventTracer_Disable(EventTracer);
}

SpatialGDKEvent SpatialGDK::ConstructEventFromRPC(const AActor* Actor, const UFunction* Function)
{
	SpatialGDKEvent Event;
	Event.Message = "";
	Event.Type = "RPC";
	Event.Data.Add("Actor", Actor->GetName());
	Event.Data.Add("Position", Actor->GetActorTransform().GetTranslation().ToString());
	Event.Data.Add("Function", Function->GetName());
	return Event;
}

SpatialGDK::SpatialGDKEvent SpatialGDK::ConstructEventFromRPC(const AActor* Actor, Worker_EntityId EntityId, Worker_RequestId RequestID)
{
	SpatialGDKEvent Event;
	Event.Message = "";
	Event.Type = "RetireEntity";
	Event.Data.Add("Actor", Actor->GetName());
	Event.Data.Add("Position", Actor->GetActorTransform().GetTranslation().ToString());
	//Event.Data.Add("CreateEntityRequestId", ToString(RequestID));
	return Event;
}

SpatialGDK::SpatialGDKEvent SpatialGDK::ConstructEventFromRPC(const AActor* Actor, VirtualWorkerId NewAuthoritativeWorkerId)
{
	SpatialGDKEvent Event;
	Event.Message = "";
	Event.Type = "AuthorityIntentUpdate";
	Event.Data.Add("Actor", Actor->GetName());
	Event.Data.Add("Position", Actor->GetActorTransform().GetTranslation().ToString());
	//Event.Data.Add("NewAuthoritativeWorkerId", ToString(NewAuthoritativeWorkerId));
	return Event;
}

SpatialGDK::SpatialGDKEvent SpatialGDK::ConstructEventFromRPC(const AActor* Actor, Worker_RequestId CreateEntityRequestId)
{
	SpatialGDKEvent Event;
	Event.Message = "";
	Event.Type = "CreateEntity";
	Event.Data.Add("Actor", Actor->GetName());
	Event.Data.Add("Position", Actor->GetActorTransform().GetTranslation().ToString());
	//Event.Data.Add("CreateEntityRequestId", ToString(CreateEntityRequestId));
	return Event;
}
