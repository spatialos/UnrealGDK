// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "Interop/Connection/SpatialEventTracer.h"

#include <inttypes.h>

#include "Interop/Connection/SpatialTraceEventBuilder.h"
#include "SpatialGDKSettings.h"
#include <WorkerSDK/improbable/c_io.h>
#include <WorkerSDK/improbable/c_trace.h>

DEFINE_LOG_CATEGORY(LogSpatialEventTracer);

using namespace SpatialGDK;
using namespace worker::c;

void SpatialEventTracer::TraceCallback(void* UserData, const Trace_Item* Item)
{
	SpatialEventTracer* EventTracer = static_cast<SpatialEventTracer*>(UserData);

	Io_Stream* Stream = EventTracer->Stream.Get();
	if (!ensure(Stream != nullptr))
	{
		return;
	}

	uint32_t ItemSize = Trace_GetSerializedItemSize(Item);
	if (EventTracer->BytesWrittenToStream + ItemSize <= EventTracer->MaxFileSize)
	{
		EventTracer->BytesWrittenToStream += ItemSize;
		int Code = Trace_SerializeItemToStream(Stream, Item, ItemSize);
		if (Code != 1)
		{
			UE_LOG(LogSpatialEventTracer, Error, TEXT("Failed to serialize to with error code %d (%s"), Code, Trace_GetLastError());
		}
	}
	else
	{
		EventTracer->BytesWrittenToStream = EventTracer->MaxFileSize;
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

TOptional<Trace_SpanId> SpatialEventTracer::TraceEvent(FSpatialTraceEvent SpatialTraceEvent)
{
	return TraceEvent(SpatialTraceEvent, nullptr, 0);
}

TOptional<Trace_SpanId> SpatialEventTracer::TraceEvent(FSpatialTraceEvent SpatialTraceEvent, const worker::c::Trace_SpanId Causes)
{
	return TraceEvent(SpatialTraceEvent, &Causes, 1);
}

TOptional<Trace_SpanId> SpatialEventTracer::TraceEvent(FSpatialTraceEvent SpatialTraceEvent, const TArray<worker::c::Trace_SpanId>& Causes)
{
	return TraceEvent(SpatialTraceEvent, Causes.GetData(), Causes.Num());
}

TOptional<Trace_SpanId> SpatialEventTracer::TraceEvent(FSpatialTraceEvent SpatialTraceEvent, const worker::c::Trace_SpanId* Causes,
													   int32 NumCauses)
{
	if (!IsEnabled())
	{
		return {};
	}

	Trace_SpanId CurrentSpanId;
	if (NumCauses > 0)
	{
		CurrentSpanId = Trace_EventTracer_AddSpan(EventTracer, Causes, NumCauses);
	}
	else
	{
		CurrentSpanId = Trace_EventTracer_AddSpan(EventTracer, nullptr, 0);
	}

	auto MessageSrc = StringCast<ANSICHAR>(*SpatialTraceEvent.Message);
	const ANSICHAR* Message = MessageSrc.Get();

	Trace_Event TraceEvent{ CurrentSpanId, /* unix_timestamp_millis: ignored */ 0, Message, SpatialTraceEvent.Type, nullptr };
	if (!Trace_EventTracer_ShouldSampleEvent(EventTracer, &TraceEvent))
	{
		return {};
	}

	Trace_EventData* EventData = Trace_EventData_Create();

	for (const auto& Pair : SpatialTraceEvent.Data)
	{
		auto KeySrc = StringCast<ANSICHAR>(*Pair.Key);
		const ANSICHAR* Key = KeySrc.Get();
		auto ValueSrc = StringCast<ANSICHAR>(*Pair.Value);
		const ANSICHAR* Value = ValueSrc.Get();
		Trace_EventData_AddStringFields(EventData, 1, &Key, &Value);
	}

	// Frame counter
	{
		const char* FrameCountStr = "FrameNum";
		char TmpBuffer[64];
		FCStringAnsi::Sprintf(TmpBuffer, "%" PRIu64, GFrameCounter);
		const char* TmpBufferPtr = TmpBuffer;
		Trace_EventData_AddStringFields(EventData, 1, &FrameCountStr, &TmpBufferPtr);
	}

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
	FString EventTracePath = FPaths::Combine(FPaths::ProjectSavedDir(), TEXT("EventTracing"));
	FString AbsLogPath;
	if (FParse::Value(FCommandLine::Get(), TEXT("eventLogPath="), AbsLogPath, false))
	{
		EventTracePath = FPaths::GetPath(AbsLogPath);
	}
	const FString FolderPath = EventTracePath;

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
	Trace_EventTracer_Destroy(EventTracer);

	bEnabled = false;
	Stream = nullptr;
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

	TArray<SpatialSpanIdCache::FieldSpanIdUpdate> FieldSpanIdUpdates = SpanIdStore.ComponentUpdate(Op);
	for (const SpatialSpanIdCache::FieldSpanIdUpdate& FieldSpanIdUpdate : FieldSpanIdUpdates)
	{
		uint32 FieldId = FieldSpanIdUpdate.FieldId;
		Trace_SpanId MergeCauses[2] = { FieldSpanIdUpdate.NewSpanId, FieldSpanIdUpdate.OldSpanId };

		TOptional<Trace_SpanId> NewSpanId =
			TraceEvent(FSpatialTraceEventBuilder::MergeComponentField(Id.EntityId, Id.ComponentId, FieldId), MergeCauses, 2);
		SpanIdStore.AddSpanId(Id, FieldId, NewSpanId.GetValue());
	}
}

bool SpatialEventTracer::GetSpanId(const EntityComponentId& Id, const uint32 FieldId, Trace_SpanId& CauseSpanId, bool bRemove /*= true*/)
{
	if (!IsEnabled())
	{
		return false;
	}

	if (!SpanIdStore.GetSpanId(Id, FieldId, CauseSpanId, bRemove))
	{
		UE_LOG(LogSpatialEventTracer, Warning, TEXT("Could not find SpanId for Entity: %d Component: %d FieldId: %d"), Id.EntityId,
			   Id.ComponentId, FieldId);
		return false;
	}
	return true;
}

bool SpatialEventTracer::GetMostRecentSpanId(const EntityComponentId& Id, worker::c::Trace_SpanId& CauseSpanId, bool bRemove /*= true*/)
{
	if (!IsEnabled())
	{
		return false;
	}

	if (!SpanIdStore.GetMostRecentSpanId(Id, CauseSpanId, bRemove))
	{
		UE_LOG(LogSpatialEventTracer, Warning, TEXT("Could not find SpanId for Entity: %d Component: %d"), Id.EntityId, Id.ComponentId);
		return false;
	}
	return true;
}

bool SpatialEventTracer::DropSpanId(const EntityComponentId& Id, const uint32 FieldId)
{
	if (!IsEnabled())
	{
		return false;
	}

	return SpanIdStore.DropSpanId(Id, FieldId);
}

bool SpatialEventTracer::DropSpanIds(const EntityComponentId& Id)
{
	if (!IsEnabled())
	{
		return false;
	}

	return SpanIdStore.DropSpanIds(Id);
}
