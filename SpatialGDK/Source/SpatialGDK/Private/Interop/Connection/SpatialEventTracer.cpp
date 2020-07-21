// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "Interop/Connection/SpatialEventTracer.h"

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
		unsigned long int span1 = *reinterpret_cast<const unsigned long int*>(&Event.span_id.data[0]);
		unsigned long int span2 = *reinterpret_cast<const unsigned long int*>(&Event.span_id.data[8]);
		UE_LOG(LogTemp, Warning, TEXT("Span: %ul%ul, Type: %s, Message: %s, Timestamp: %ul"),
			span1, span2, *FString(Event.type), *FString(Event.message), Event.unix_timestamp_millis);
		break;
	}
	//case TRACE_ITEM_TYPE_SPAN:
	//{
	//	const Trace_Span& Span = Item->item.span;
	//	//UE_LOG(LogTemp, Warning, TEXT("Span: %s"), *FString(Span.id.data));
	//	unsigned long int span1 = *reinterpret_cast<const unsigned long int*>(&Span.id.data[0]);
	//	unsigned long int span2 = *reinterpret_cast<const unsigned long int*>(&Span.id.data[8]);
	//	UE_LOG(LogTemp, Warning, TEXT("Span: %ul%ul"), span1, span2);
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

void SpatialGDK::SpatialEventTracer::Enable()
{
	Trace_EventTracer_Enable(EventTracer);
}

void SpatialGDK::SpatialEventTracer::Disable()
{
	Trace_EventTracer_Disable(EventTracer);
}

