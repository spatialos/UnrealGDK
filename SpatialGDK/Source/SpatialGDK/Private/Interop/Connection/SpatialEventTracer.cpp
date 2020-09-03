// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "Interop/Connection/SpatialEventTracer.h"

#include "SpatialGDKSettings.h"
#include "UObject/Object.h"
#include <WorkerSDK/improbable/c_io.h>
#include <WorkerSDK/improbable/c_trace.h>

DEFINE_LOG_CATEGORY(LogSpatialEventTracer);

using namespace SpatialGDK;
using namespace worker::c;

void SpatialEventTracer::TraceCallback(void* UserData, const Trace_Item* Item)
{
	SpatialEventTracer* EventTracer = static_cast<SpatialEventTracer*>(UserData);
	if (EventTracer == nullptr || !EventTracer->IsEnabled())
	{
		return;
	}

	if (!ensure(EventTracer->Stream.Get() != nullptr))
	{
		return;
	}

	uint32_t ItemSize = Trace_GetSerializedItemSize(Item);
	if (EventTracer->BytesWrittenToStream + ItemSize <= EventTracer->MaxFileSize)
	{
		EventTracer->BytesWrittenToStream += ItemSize;
		int Code = Trace_SerializeItemToStream(EventTracer->Stream.Get(), Item, ItemSize);
		if (Code != 1)
		{
			UE_LOG(LogSpatialEventTracer, Error, TEXT("Failed to serialize to with error code %d (%s"), Code, Trace_GetLastError());
		}
	}
}

SpatialScopedActiveSpanId::SpatialScopedActiveSpanId(SpatialEventTracer* InEventTracer, const TOptional<Trace_SpanId>& InCurrentSpanId)
	: CurrentSpanId(InCurrentSpanId)
	, EventTracer(InEventTracer->GetWorkerEventTracer())
{
	if (InCurrentSpanId.IsSet())
	{
		Trace_EventTracer_SetActiveSpanId(EventTracer, CurrentSpanId.GetValue());
	}
}

SpatialScopedActiveSpanId::~SpatialScopedActiveSpanId()
{
	if (CurrentSpanId.IsSet())
	{
		Trace_EventTracer_ClearActiveSpanId(EventTracer);
	}
}

SpatialEventTracer::SpatialEventTracer(const FString& WorkerId)
{
	if (const USpatialGDKSettings* Settings = GetDefault<USpatialGDKSettings>())
	{
		if (Settings->bEventTracingEnabled)
		{
			MaxFileSize = Settings->MaxEventTracingFileSizeBytes;
			Enable(WorkerId);
		}
	}
}

SpatialEventTracer::~SpatialEventTracer()
{
	if (IsEnabled())
	{
		Disable();
	}
}

TOptional<Trace_SpanId> SpatialEventTracer::TraceEvent(const FEventMessage& EventMessage, const UStruct* Struct,
													   const worker::c::Trace_SpanId* Cause)
{
	if (!IsEnabled())
	{
		return {};
	}

	Trace_SpanId CurrentSpanId;
	if (Cause)
	{
		CurrentSpanId = Trace_EventTracer_AddSpan(EventTracer, Cause, 1);
	}
	else
	{
		CurrentSpanId = Trace_EventTracer_AddSpan(EventTracer, nullptr, 0);
	}

	Trace_Event TraceEvent{ CurrentSpanId, /* unix_timestamp_millis: ignored */ 0, "", EventMessage.GetType(), nullptr };
	if (!Trace_EventTracer_ShouldSampleEvent(EventTracer, &TraceEvent))
	{
		return {};
	}

	Trace_EventData* EventData = Trace_EventData_Create();

	auto AddTraceEventStringField = [EventData](const FString& KeyString, const FString& ValueString) {
		auto KeySrc = StringCast<ANSICHAR>(*KeyString);
		const ANSICHAR* Key = KeySrc.Get();
		auto ValueSrc = StringCast<ANSICHAR>(*ValueString);
		const ANSICHAR* Value = ValueSrc.Get();
		Trace_EventData_AddStringFields(EventData, 1, &Key, &Value);
	};

#if ENGINE_MINOR_VERSION >= 25
	typedef FProperty UnrealProperty;
	typedef FStrProperty UnrealStrProperty;
	typedef FObjectProperty UnrealObjectProperty;
#else
	typedef UProperty UnrealProperty;
	typedef UStrProperty UnrealStrProperty;
	typedef UObjectProperty UnrealObjectProperty;
#endif
	for (TFieldIterator<UnrealProperty> It(Struct); It; ++It)
	{
		UnrealProperty* Property = *It;

		FString VariableName = Property->GetName();
		const void* Value = Property->ContainerPtrToValuePtr<uint8>(&EventMessage);

		check(Property->ArrayDim == 1); // Arrays not handled yet

		if (UnrealStrProperty* StringProperty = CastField<UnrealStrProperty>(Property))
		{
			AddTraceEventStringField(VariableName, StringProperty->GetPropertyValue(Value));
		}
		else if (UnrealObjectProperty* ObjectProperty = CastField<UnrealObjectProperty>(Property))
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

bool SpatialEventTracer::IsEnabled() const
{
	return bEnabled; // Trace_EventTracer_IsEnabled(EventTracer);
}

void SpatialEventTracer::Enable(const FString& FileName)
{
	Trace_EventTracer_Parameters parameters = {};
	parameters.user_data = this;
	parameters.callback = &SpatialEventTracer::TraceCallback;
	EventTracer = Trace_EventTracer_Create(&parameters);
	Trace_EventTracer_Enable(EventTracer);
	bEnabled = true;

	UE_LOG(LogSpatialEventTracer, Log, TEXT("Spatial event tracing enabled."));

	// Open a local file
	const FString FolderPath = FPaths::Combine(FPaths::ProjectSavedDir(), TEXT("EventTracing"));
	const FString FullFileName = FString::Printf(TEXT("EventTrace_%s_%s.trace"), *FileName, *FDateTime::Now().ToString());
	const FString FilePath = FPaths::Combine(FolderPath, FullFileName);

	IPlatformFile& PlatformFile = FPlatformFileManager::Get().GetPlatformFile();
	if (PlatformFile.CreateDirectoryTree(*FolderPath))
	{
		UE_LOG(LogSpatialEventTracer, Log, TEXT("Capturing trace to %s."), *FilePath);
		Stream.Reset(Io_CreateFileStream(TCHAR_TO_ANSI(*FilePath), Io_OpenMode::IO_OPEN_MODE_WRITE));
	}
}

void SpatialEventTracer::StreamDeleter::operator()(worker::c::Io_Stream* StreamToDestroy) const
{
	Io_Stream_Destroy(StreamToDestroy);
}

void SpatialEventTracer::Disable()
{
	UE_LOG(LogSpatialEventTracer, Log, TEXT("Spatial event tracing disabled."));
	Trace_EventTracer_Disable(EventTracer);
	bEnabled = false;
	Stream = nullptr;

	Trace_EventTracer_Destroy(EventTracer);
}

void SpatialEventTracer::ComponentAdd(const Worker_Op& Op)
{
	SpanIdStore.ComponentAdd(Op);
}

void SpatialEventTracer::ComponentRemove(const Worker_Op& Op)
{
	SpanIdStore.ComponentRemove(Op);
}

void SpatialEventTracer::ComponentUpdate(const Worker_Op& Op)
{
	const Worker_ComponentUpdateOp& ComponentUpdateOp = Op.op.component_update;
	EntityComponentId Id(ComponentUpdateOp.entity_id, ComponentUpdateOp.update.component_id);

	if (SpanIdStore.HasSpanIds(Id))
	{
		TraceEvent(FEventMergeComponentUpdate(Id.EntityId, Id.ComponentId));
	}

	SpanIdStore.ComponentUpdate(Op);
}

worker::c::Trace_SpanId SpatialEventTracer::GetSpanId(const EntityComponentId& Id, const uint32 FieldId)
{
	return SpanIdStore.GetSpanId(Id, FieldId);
}

void SpatialEventTracer::DropSpanIds(const EntityComponentId& Id)
{
	SpanIdStore.DropSpanIds(Id);
}

void SpatialEventTracer::DropSpanId(const EntityComponentId& Id, const uint32 FieldId)
{
	SpanIdStore.DropSpanId(Id, FieldId);
}

void SpatialEventTracer::DropOldUpdates()
{
	SpanIdStore.DropOldSpanIds();
}
