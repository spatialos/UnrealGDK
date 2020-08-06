// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "Interop/Connection/SpatialEventTracer.h"

#include "EngineClasses/SpatialNetDriver.h"
#include "EngineMinimal.h"
#include "GameFramework/Actor.h"
#include "GenericPlatform/GenericPlatformAtomics.h"
#include "Interop/Connection/SpatialConnectionManager.h"
#include "Interop/Connection/SpatialWorkerConnection.h"
#include "Runtime/JsonUtilities/Public/JsonUtilities.h"
#include "UObject/Class.h"
#include <WorkerSDK/improbable/c_trace.h>
#include <WorkerSDK/improbable/c_io.h>

DEFINE_LOG_CATEGORY(LogSpatialEventTracer);

using namespace SpatialGDK;
using namespace worker::c;

void SpatialEventTracer::TraceCallback(void* UserData, const Trace_Item* Item)
{
	SpatialEventTracer* EventTracer = static_cast<SpatialEventTracer*>(UserData);
	if (FPlatformAtomics::AtomicRead(&EventTracer->bEnabled))
	{
		FScopeLock Lock(&EventTracer->StreamLock); // TODO: Remove this and figure out how to use EventTracer->Stream without race conditions
		if (EventTracer->Stream)
		{
			int Code = Trace_SerializeItemToStream(EventTracer->Stream, Item, Trace_GetSerializedItemSize(Item));
			if (Code != 0)
			{
				UE_LOG(LogSpatialEventTracer, Error, TEXT("Failed to serialize to with error code %d (%s"), Code, Trace_GetLastError());
			}
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

SpatialEventTracer::SpatialEventTracer()
{
	Trace_EventTracer_Parameters parameters = {};
	parameters.user_data = this;
	parameters.callback = &SpatialEventTracer::TraceCallback;
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

Trace_SpanId SpatialEventTracer::CreateNewSpanId()
{
	return Trace_EventTracer_AddSpan(EventTracer, nullptr, 0);
}

Trace_SpanId SpatialEventTracer::CreateNewSpanId(const TArray<Trace_SpanId>& Causes)
{
	return Trace_EventTracer_AddSpan(EventTracer, Causes.GetData(), Causes.Num());
}

TOptional<Trace_SpanId> SpatialEventTracer::TraceEvent(const FEventMessage& EventMessage, const UStruct* Struct, const worker::c::Trace_SpanId* Cause)
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

void SpatialEventTracer::Enable(const FString& FileName)
{
	FScopeLock Lock(&StreamLock);
	Trace_EventTracer_Enable(EventTracer);
	FPlatformAtomics::AtomicStore(&bEnabled, 1);

	UE_LOG(LogSpatialEventTracer, Log, TEXT("Spatial event tracing enabled."));

	// Open a local file
	FString FolderPath = FPaths::ProjectSavedDir() + TEXT("EventTracing\\");

	FDateTime CurrentDateTime = FDateTime::Now();
	FString FilePath = FolderPath + FString::Printf(TEXT("EventTrace_%s_%s.trace"), *FileName, *CurrentDateTime.ToString());

	IPlatformFile& PlatformFile = FPlatformFileManager::Get().GetPlatformFile();
	if (PlatformFile.CreateDirectoryTree(*FolderPath))
	{
		UE_LOG(LogSpatialEventTracer, Log, TEXT("Capturing trace to %s."), *FilePath);
		Stream = Io_CreateFileStream(TCHAR_TO_ANSI(*FilePath), Io_OpenMode::IO_OPEN_MODE_DEFAULT);
	}
}

void SpatialEventTracer::Disable()
{
	FScopeLock Lock(&StreamLock);
	UE_LOG(LogSpatialEventTracer, Log, TEXT("Spatial event tracing disabled."));
	Trace_EventTracer_Disable(EventTracer);
	bEnabled = false;
	Io_Stream_Destroy(Stream);
	Stream = nullptr; 
}

static TAutoConsoleVariable<int32> CVarGDKEventTracing(
	TEXT("gdk.eventTracing"),
	0,
	TEXT("Defines the distortion/refraction quality, adjust for quality or performance.\n")
	TEXT("<=0: off (disabled)\n")
	TEXT("  1: enabled\n"),
	ECVF_Scalability);

static void EventTracingSink()
{
	int Level = CVarGDKEventTracing.GetValueOnGameThread();
	bool bIsEnabled = Level != 0;

	const TIndirectArray<FWorldContext>& WorldContexts = GEngine->GetWorldContexts();
	for (const FWorldContext& Context : WorldContexts)
	{
		UWorld* World = Context.World();
		if (USpatialNetDriver* SpatialNetDriver = Cast<USpatialNetDriver>(World->GetNetDriver()))
		{
			USpatialConnectionManager* ConnectionManager = SpatialNetDriver->ConnectionManager;
			if (ConnectionManager != nullptr
				&& ConnectionManager->GetEventTracer() != nullptr
				&& ConnectionManager->GetWorkerConnection() != nullptr)
			{
				SpatialEventTracer* EventTracer = ConnectionManager->GetEventTracer();
				bool bIsTracerEnabled = EventTracer->IsEnabled();
				if (bIsTracerEnabled != bIsEnabled)
				{
					if (bIsEnabled)
					{
						EventTracer->Enable(ConnectionManager->GetWorkerConnection()->GetWorkerId());
					}
					else
					{
						EventTracer->Disable();
					}
				}
			}
		}
	}
}

FAutoConsoleVariableSink CVarGDKEventTracingSink(FConsoleCommandDelegate::CreateStatic(&EventTracingSink));
