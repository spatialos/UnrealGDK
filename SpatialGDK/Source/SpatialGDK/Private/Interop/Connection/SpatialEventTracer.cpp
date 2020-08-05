// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "Interop/Connection/SpatialEventTracer.h"

#include "EngineClasses/SpatialNetDriver.h"
#include "EngineMinimal.h"
#include "GameFramework/Actor.h"
#include "Runtime/JsonUtilities/Public/JsonUtilities.h"
#include "UObject/Class.h"
#include <WorkerSDK/improbable/c_trace.h>
#include <WorkerSDK/improbable/c_io.h>

DEFINE_LOG_CATEGORY(LogSpatialEventTracer);

using namespace SpatialGDK;
using namespace worker::c;

// TODO(EventTracer): Use c_io.h functions instead, to write data to a file
// Below is a hacky function written for testing purposes, which must be removed

void MyTraceCallback(void* UserData, const Trace_Item* Item)
{
	switch (Item->item_type)
	{
	case TRACE_ITEM_TYPE_EVENT:
	{
		const Trace_Event& Event = Item->item.event;

		// temporary filtering for nicer UE_LOGs
		if (Event.type == FString("network.receive_raw_message") ||
			Event.type == FString("network.receive_udp_datagram") ||
			Event.type == FString("network.send_raw_message") ||
			Event.type == FString("network.send_udp_datagram") ||
			Event.type == FString("worker.dequeue_op") ||
			Event.type == FString("worker.enqueue_op"))
		{
			return;
		}

#if 0
		FString HexStr;
		for (int i = 0; i < 16; i++)
		{
			char b[32];
			unsigned int x = (unsigned char)Event.span_id.data[i];
			sprintf(b, "%0x", x);
			HexStr += ANSI_TO_TCHAR(b);
		}
		UE_LOG(LogSpatialEventTracer, Log, TEXT("Span: %s, Type: %s, Message: %s, Timestamp: %lu"), *HexStr, *FString(Event.type), *FString(Event.message), Event.unix_timestamp_millis);
#endif

		if (Event.data != nullptr)
		{
			uint32_t DataFieldCount = Trace_EventData_GetFieldCount(Event.data);

			TArray<const char*> Keys;
			Keys.SetNumUninitialized(DataFieldCount);
			TArray<const char*> Values;
			Values.SetNumUninitialized(DataFieldCount);

			SpatialEventTracer::EventTracingData EventTracingData;
			EventTracingData.Add(TEXT("EventType"), Event.type);

			Trace_EventData_GetStringFields(Event.data, Keys.GetData(), Values.GetData());
			for (uint32_t i = 0; i < DataFieldCount; ++i)
			{
				EventTracingData.Add(Keys[i], Values[i]);
			}

			SpatialEventTracer* EventTracer = static_cast<SpatialEventTracer*>(UserData);
			EventTracer->WriteEventDataToJson(EventTracingData);
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

SpatialSpanIdActivator::SpatialSpanIdActivator(SpatialEventTracer* InEventTracer, const TOptional<Trace_SpanId>& InCurrentSpanId)
	: CurrentSpanId(InCurrentSpanId)
	, EventTracer(InEventTracer->GetWorkerEventTracer())
{
	if (InCurrentSpanId.IsSet())
	{
		Trace_EventTracer_SetActiveSpanId(EventTracer, CurrentSpanId.GetValue());
	}
}

SpatialSpanIdActivator::~SpatialSpanIdActivator()
{
	if (CurrentSpanId.IsSet())
	{
		Trace_EventTracer_UnsetActiveSpanId(EventTracer);
	}
}


SpatialEventTracer::SpatialEventTracer(UWorld* World)
	: NetDriver(Cast<USpatialNetDriver>(World->GetNetDriver()))
{
	Trace_EventTracer_Parameters parameters = {};
	parameters.user_data = this;
	parameters.callback = MyTraceCallback;
	EventTracer = Trace_EventTracer_Create(&parameters);
	Trace_EventTracer_Enable(EventTracer);
}

SpatialEventTracer::~SpatialEventTracer()
{
	Trace_EventTracer_Disable(EventTracer);
	Trace_EventTracer_Destroy(EventTracer);

	if (Stream != nullptr)
	{
		Io_Stream_Destroy(Stream);
	}
}

void SpatialEventTracer::Start()
{
	FString FolderPath = FPaths::ProjectSavedDir() + TEXT("EventTracing\\");

	FDateTime CurrentDateTime = FDateTime::Now();
	FString FilePath = FolderPath + FString::Printf(TEXT("EventTrace_%s.json"), *CurrentDateTime.ToString());

	IPlatformFile& PlatformFile = FPlatformFileManager::Get().GetPlatformFile();
	if (PlatformFile.CreateDirectoryTree(*FolderPath))
	{
		Stream = Io_CreateFileStream(TCHAR_TO_ANSI(*FilePath), Io_OpenMode::IO_OPEN_MODE_DEFAULT);
	}
}

void SpatialEventTracer::WriteEventDataToJson(const EventTracingData& EventData)
{
	if (EventData.Num() == 0)
	{
		return;
	}

	TSharedRef<FJsonObject> TopJsonObject = MakeShared<FJsonObject>();
	for (const auto& Pair : EventData)
	{
		TopJsonObject->SetStringField(Pair.Key, Pair.Value);
	}

	FString JsonString;
	TSharedRef<TJsonWriter<TCHAR, TCondensedJsonPrintPolicy<TCHAR>> > JsonWriter = TJsonWriterFactory<TCHAR, TCondensedJsonPrintPolicy<TCHAR>>::Create(&JsonString, 0);
	bool bSuccess = FJsonSerializer::Serialize(TopJsonObject, JsonWriter);
	JsonWriter->Close();

	if (Stream == nullptr)
	{
		return;
	}

	JsonString.Append("\n");
	Io_Stream_Write(Stream, (const uint8*)TCHAR_TO_ANSI(*JsonString), JsonString.Len());
}

Trace_SpanId SpatialEventTracer::CreateNewSpanId()
{
	return Trace_EventTracer_AddSpan(EventTracer, nullptr, 0);
}

Trace_SpanId SpatialEventTracer::CreateNewSpanId(const TArray<Trace_SpanId>& Causes)
{
	return Trace_EventTracer_AddSpan(EventTracer, Causes.GetData(), Causes.Num());
}

TOptional<Trace_SpanId> SpatialEventTracer::TraceEvent(const FEventMessage& EventMessage, UStruct* Struct, const worker::c::Trace_SpanId* Cause)
{
	if (!IsEnabled())
	{
		return {};
	}

	Trace_SpanId CurrentSpanId;
	if (Cause)
	{
		CurrentSpanId = CreateNewSpanId({ *Cause });
	}
	else
	{
		CurrentSpanId = CreateNewSpanId();
	}

	Trace_Event TraceEvent{ CurrentSpanId, 0, "", EventMessage.GetType(), nullptr };
	if (!Trace_EventTracer_ShouldSampleEvent(EventTracer, &TraceEvent))
	{
		return {};
	}

	Trace_EventData* EventData = Trace_EventData_Create();

	auto AddTraceEventStringField = [EventData](const FString& KeyString, const FString& ValueString)
	{
		auto KeySrc = StringCast<ANSICHAR>(*KeyString);
		const ANSICHAR* Key = KeySrc.Get();
		auto ValueSrc = StringCast<ANSICHAR>(*ValueString);
		const ANSICHAR* Value = ValueSrc.Get();
		Trace_EventData_AddStringFields(EventData, 1, &Key, &Value);
	};

	for (TFieldIterator<UProperty> It(Struct); It; ++It)
	{
		UProperty* Property = *It;

		FString VariableName = Property->GetName();
		const void* Value = Property->ContainerPtrToValuePtr<uint8>(&EventMessage);

		check(Property->ArrayDim == 1); // Arrays not handled yet

		// convert the property to a FJsonValue
		if (UStrProperty *StringProperty = Cast<UStrProperty>(Property))
		{
			AddTraceEventStringField(VariableName, StringProperty->GetPropertyValue(Value));
		}
		else if (UObjectProperty* ObjectProperty = Cast<UObjectProperty>(Property))
		{
			UObject* Object = ObjectProperty->GetPropertyValue(Value);
			if (Object)
			{
				AddTraceEventStringField(VariableName, Object->GetName());
				if (AActor* Actor = Cast<AActor>(Object))
				{
					FString KeyString = VariableName + TEXT("Position");
					FString ValueString = Actor->GetTransform().GetTranslation().ToString();
					AddTraceEventStringField(KeyString, ValueString);
				}
			}
			else
			{
				AddTraceEventStringField(VariableName, "Null");
			}
		}
		else // Default
		{
			FString StringValue;
			Property->ExportTextItem(StringValue, Value, NULL, NULL, PPF_None);
			AddTraceEventStringField(VariableName, StringValue);
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

	// TODO(EventTracer): implement and call AddTargetObjectInfoToEventData
	// TODO(EventTracer): implement and call AddComponentIdInfoToEventData

	TraceEvent.data = EventData;
	Trace_EventTracer_AddEvent(EventTracer, &TraceEvent);
	Trace_EventData_Destroy(EventData);

	return CurrentSpanId;
}

void SpatialEventTracer::Enable()
{
	Trace_EventTracer_Enable(EventTracer);
	bEnalbed = true;
}

void SpatialEventTracer::Disable()
{
	Trace_EventTracer_Disable(EventTracer);
	bEnalbed = false;
}
