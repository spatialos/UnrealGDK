// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "Interop/Connection/SpatialEventTracer.h"

#include "EngineClasses/SpatialNetDriver.h"
#include "EngineMinimal.h"
#include "GameFramework/Actor.h"
#include "GenericPlatform/GenericPlatformAtomics.h"
#include "Interop/Connection/SpatialConnectionManager.h"
#include "Interop/Connection/SpatialWorkerConnection.h"
#include "UObject/Class.h"
#include <WorkerSDK/improbable/c_io.h>
#include <WorkerSDK/improbable/c_trace.h>

DEFINE_LOG_CATEGORY(LogSpatialEventTracer);

using namespace SpatialGDK;
using namespace worker::c;

void SpatialEventTracer::TraceCallback(void* UserData, const Trace_Item* Item)
{
	SpatialEventTracer* EventTracer = static_cast<SpatialEventTracer*>(UserData);
	if (EventTracer == nullptr || !EventTracer->bEnabled)
	{
		return;
	}

	checkf(EventTracer->Stream, TEXT("Tracer in an invalid state in TraceCallback"));

	uint32_t ItemSize = Trace_GetSerializedItemSize(Item);
	if (EventTracer->BytesWrittenToStream + ItemSize <= EventTracer->MaxFileSize)
	{
		EventTracer->BytesWrittenToStream += ItemSize;
		int Code = Trace_SerializeItemToStream(EventTracer->Stream, Item, ItemSize);
		if (Code != 0)
		{
			UE_LOG(LogSpatialEventTracer, Error, TEXT("Failed to serialize to with error code %d (%s"), Code, Trace_GetLastError());
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

Trace_SpanId SpatialEventTracer::CreateNewSpanId()
{
	return Trace_EventTracer_AddSpan(EventTracer, nullptr, 0);
}

Trace_SpanId SpatialEventTracer::CreateNewSpanId(const TArray<Trace_SpanId>& Causes)
{
	return Trace_EventTracer_AddSpan(EventTracer, Causes.GetData(), Causes.Num());
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

		// convert the property to a FJsonValue
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
		Stream = Io_CreateFileStream(TCHAR_TO_ANSI(*FilePath), Io_OpenMode::IO_OPEN_MODE_WRITE);
	}
}

void SpatialEventTracer::Disable()
{
	UE_LOG(LogSpatialEventTracer, Log, TEXT("Spatial event tracing disabled."));
	Trace_EventTracer_Disable(EventTracer);
	bEnabled = false;
	Io_Stream_Destroy(Stream);
	Stream = nullptr;

	Trace_EventTracer_Destroy(EventTracer);

	if (Stream != nullptr)
	{
		Io_Stream_Destroy(Stream);
	}
}

Trace_SpanId* SpatialEventTracer::GetEntityComponentSpanId(const Worker_EntityId EntityId, const Worker_ComponentId ComponentId)
{
	return SpanIdStore.GetEntityComponentSpanId({ EntityId, ComponentId });
}

void SpatialEventTracer::ComponentAdd(const Worker_EntityId EntityId, const Worker_ComponentId ComponentId,
									  const worker::c::Trace_SpanId SpanId)
{
	SpanIdStore.ComponentAdd({ EntityId, ComponentId }, SpanId);
}

void SpatialEventTracer::ComponentRemove(const Worker_EntityId EntityId, const Worker_ComponentId ComponentId,
										 const worker::c::Trace_SpanId SpanId)
{
	SpanIdStore.ComponentRemove({ EntityId, ComponentId }, SpanId);
}

void SpatialEventTracer::ComponentUpdate(const Worker_EntityId EntityId, const Worker_ComponentId ComponentId,
										 const worker::c::Trace_SpanId SpanId)
{
	EntityComponentId Id = { EntityId, ComponentId };
	Trace_SpanId* ExistingSpanId = SpanIdStore.GetEntityComponentSpanId(Id);
	if (ExistingSpanId != nullptr)
	{
		TraceEvent(FEventMergeComponentUpdate(EntityId, ComponentId), ExistingSpanId);
	}

	SpanIdStore.ComponentUpdate(Id, SpanId);
}

worker::c::Trace_SpanId SpatialEventTracer::GetNextRPCSpanID()
{
	return SpanIdStore.GetNextRPCSpanID();
}

void SpatialEventTracer::ClearSpanStore()
{
	SpanIdStore.Clear();
}
