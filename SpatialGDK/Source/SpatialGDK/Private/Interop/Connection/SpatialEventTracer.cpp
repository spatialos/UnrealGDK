// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "Interop/Connection/SpatialEventTracer.h"

#include <inttypes.h>

#include "HAL/PlatformFile.h"
#include "HAL/PlatformFilemanager.h"
#include "Interop/Connection/SpatialTraceEventBuilder.h"
#include "SpatialGDKSettings.h"

DEFINE_LOG_CATEGORY(LogSpatialEventTracer);

namespace SpatialGDK
{
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

SpatialScopedActiveSpanId::SpatialScopedActiveSpanId(SpatialEventTracer* InEventTracer, const FSpatialGDKSpanId& InCurrentSpanId)
	: CurrentSpanId(InCurrentSpanId)
{
	if (InEventTracer == nullptr)
	{
		EventTracer = nullptr;
		return;
	}

	EventTracer = InEventTracer->GetWorkerEventTracer();
	if (CurrentSpanId.IsValid())
	{
		Trace_EventTracer_SetActiveSpanId(EventTracer, CurrentSpanId.GetConstData());
	}
}

SpatialScopedActiveSpanId::~SpatialScopedActiveSpanId()
{
	if (EventTracer != nullptr && CurrentSpanId.IsValid())
	{
		Trace_EventTracer_ClearActiveSpanId(EventTracer);
	}
}

SpatialEventTracer::SpatialEventTracer(const FString& WorkerId)
{
	const USpatialGDKSettings* Settings = GetDefault<USpatialGDKSettings>();
	MaxFileSize = Settings->MaxEventTracingFileSizeBytes;

	Trace_EventTracer_Parameters parameters = {};
	parameters.user_data = this;
	parameters.callback = &SpatialEventTracer::TraceCallback;
	EventTracer = Trace_EventTracer_Create(&parameters);

	Trace_SamplingParameters SamplingParameters = {};
	SamplingParameters.sampling_mode = Trace_SamplingMode::TRACE_SAMPLING_MODE_PROBABILISTIC;

	TArray<Trace_SpanSamplingProbability> SpanSamplingProbabilities;
	for (const auto& Pair : Settings->EventSamplingModeOverrides)
	{
		Trace_SpanSamplingProbability Element;

		auto TypeSrc = StringCast<ANSICHAR>(*Pair.Key.ToString());
		Element.event_type = TypeSrc.Get();
		Element.probability = Pair.Value;

		SpanSamplingProbabilities.Add(Element);
	}

	SamplingParameters.probabilistic_parameters.default_probability = Settings->SamplingProbability;
	SamplingParameters.probabilistic_parameters.event_type_count = SpanSamplingProbabilities.Num();
	SamplingParameters.probabilistic_parameters.event_type_sampling_probabilities = SpanSamplingProbabilities.GetData();

	Trace_EventTracer_SetSampler(EventTracer, &SamplingParameters);

	UE_LOG(LogSpatialEventTracer, Log, TEXT("Spatial event tracing enabled."));

	// Open a local file
	FString EventTracePath = FPaths::Combine(FPaths::ProjectSavedDir(), TEXT("EventTracing"));
	FString AbsLogPath;
	if (FParse::Value(FCommandLine::Get(), TEXT("eventLogPath="), AbsLogPath, false))
	{
		EventTracePath = FPaths::GetPath(AbsLogPath);
	}

	FolderPath = EventTracePath;
	const FString FullFileName = FString::Printf(TEXT("EventTrace_%s_%s.trace"), *WorkerId, *FDateTime::Now().ToString());
	const FString FilePath = FPaths::Combine(FolderPath, FullFileName);

	IPlatformFile& PlatformFile = FPlatformFileManager::Get().GetPlatformFile();
	if (PlatformFile.CreateDirectoryTree(*FolderPath))
	{
		UE_LOG(LogSpatialEventTracer, Log, TEXT("Capturing trace to %s."), *FilePath);
		Stream.Reset(Io_CreateFileStream(TCHAR_TO_ANSI(*FilePath), Io_OpenMode::IO_OPEN_MODE_WRITE));
	}
}

SpatialEventTracer::~SpatialEventTracer()
{
	UE_LOG(LogSpatialEventTracer, Log, TEXT("Spatial event tracing disabled."));
	Trace_EventTracer_Destroy(EventTracer);
}

FUserSpanId SpatialEventTracer::GDKSpanIdToUserSpanId(const FSpatialGDKSpanId& SpanId)
{
	if (!SpanId.IsValid())
	{
		return {};
	}

	FUserSpanId UserSpanId;
	UserSpanId.Data.SetNum(TRACE_SPAN_ID_SIZE_BYTES);
	FMemory::Memcpy(UserSpanId.Data.GetData(), SpanId.GetConstData(), TRACE_SPAN_ID_SIZE_BYTES);
	return UserSpanId;
}

FSpatialGDKSpanId SpatialEventTracer::UserSpanIdToGDKSpanId(const FUserSpanId& UserSpanId)
{
	if (!UserSpanId.IsValid())
	{
		return FSpatialGDKSpanId(false);
	}

	FSpatialGDKSpanId TraceSpanId(true);
	FMemory::Memcpy(TraceSpanId.GetData(), UserSpanId.Data.GetData(), TRACE_SPAN_ID_SIZE_BYTES);
	return TraceSpanId;
}

FSpatialGDKSpanId SpatialEventTracer::CreateSpan(const Trace_SpanIdType* Causes /* = nullptr*/, int32 NumCauses /* = 0*/) const
{
	if (Causes == nullptr && NumCauses > 0)
	{
		check(false);
		return FSpatialGDKSpanId(false);
	}

	Trace_SamplingResult SamplingResult = Trace_EventTracer_ShouldSampleSpan(EventTracer, Causes, NumCauses, nullptr);
	if (SamplingResult.decision == Trace_SamplingDecision::TRACE_SHOULD_NOT_SAMPLE)
	{
		return FSpatialGDKSpanId(true);
	}

	FSpatialGDKSpanId TraceSpanId(true);
	if (Causes != nullptr && NumCauses > 0)
	{
		Trace_EventTracer_AddSpan(EventTracer, Causes, NumCauses, nullptr, TraceSpanId.GetData());
	}
	else
	{
		Trace_EventTracer_AddSpan(EventTracer, Trace_SpanId_Null(), 1, nullptr, TraceSpanId.GetData());
	}
	return TraceSpanId;
}

void SpatialEventTracer::TraceEvent(const FSpatialTraceEvent& SpatialTraceEvent, const FSpatialGDKSpanId& EventSpanId)
{
	if (!EventSpanId.IsValid())
	{
		return;
	}

	auto MessageSrc = StringCast<ANSICHAR>(*SpatialTraceEvent.Message);
	auto TypeSrc = StringCast<ANSICHAR>(*SpatialTraceEvent.Type.ToString());

	Trace_Event Event;
	Event.span_id = EventSpanId.GetConstData();
	Event.message = MessageSrc.Get();
	Event.type = TypeSrc.Get();
	Event.unix_timestamp_millis = 0;
	Event.data = nullptr;

	Trace_SamplingResult EventSamplingResult = Trace_EventTracer_ShouldSampleEvent(EventTracer, &Event);
	switch (EventSamplingResult.decision)
	{
	case Trace_SamplingDecision::TRACE_SHOULD_NOT_SAMPLE:
	{
		break;
	}
	case Trace_SamplingDecision::TRACE_SHOULD_SAMPLE_WITHOUT_DATA:
	{
		Trace_EventTracer_AddEvent(EventTracer, &Event);
		break;
	}
	case Trace_SamplingDecision::TRACE_SHOULD_SAMPLE:
	{
		Trace_EventData* EventData = Trace_EventData_Create();
		ConstructTraceEventData(EventData, SpatialTraceEvent);
		Event.data = EventData;
		Trace_EventTracer_AddEvent(EventTracer, &Event);
		Trace_EventData_Destroy(EventData);
		break;
	}
	default:
	{
		UE_LOG(LogSpatialEventTracer, Log, TEXT("Could not handle invalid sampling decision %d."), static_cast<int>(EventSamplingResult.decision));
		break;
	}
	}
}

FSpatialGDKSpanId SpatialEventTracer::TraceFilterableEvent(const FSpatialTraceEvent& SpatialTraceEvent,
														   const Trace_SpanIdType* Causes /* = nullptr*/, int32 NumCauses /* = 0*/)
{
	if (Causes == nullptr && NumCauses > 0)
	{
		return FSpatialGDKSpanId(false);
	}

	auto MessageSrc = StringCast<ANSICHAR>(*SpatialTraceEvent.Message);
	auto TypeSrc = StringCast<ANSICHAR>(*SpatialTraceEvent.Type.ToString());

	Trace_Event Event;
	Event.span_id = nullptr;
	Event.message = MessageSrc.Get();
	Event.type = TypeSrc.Get();
	Event.unix_timestamp_millis = 0;
	Event.data = nullptr;

	Trace_SamplingResult SpanSamplingResult = Trace_EventTracer_ShouldSampleSpan(EventTracer, Causes, NumCauses, &Event);
	if (SpanSamplingResult.decision == Trace_SamplingDecision::TRACE_SHOULD_NOT_SAMPLE)
	{
		return FSpatialGDKSpanId(true);
	}

	FSpatialGDKSpanId TraceSpanId(true);
	Trace_EventTracer_AddSpan(EventTracer, Causes, NumCauses, &Event, TraceSpanId.GetData());
	Event.span_id = TraceSpanId.GetData();

	Trace_SamplingResult EventSamplingResult = Trace_EventTracer_ShouldSampleEvent(EventTracer, &Event);
	switch (EventSamplingResult.decision)
	{
	case Trace_SamplingDecision::TRACE_SHOULD_NOT_SAMPLE:
	{
		return TraceSpanId;
	}
	case Trace_SamplingDecision::TRACE_SHOULD_SAMPLE_WITHOUT_DATA:
	{
		Trace_EventTracer_AddEvent(EventTracer, &Event);
		return TraceSpanId;
	}
	case Trace_SamplingDecision::TRACE_SHOULD_SAMPLE:
	{
		Trace_EventData* EventData = Trace_EventData_Create();
		ConstructTraceEventData(EventData, SpatialTraceEvent);
		Event.data = EventData;
		Trace_EventTracer_AddEvent(EventTracer, &Event);
		Trace_EventData_Destroy(EventData);
		return TraceSpanId;
	}
	default:
	{
		UE_LOG(LogSpatialEventTracer, Log, TEXT("Could not handle invalid sampling decision %d."), static_cast<int>(EventSamplingResult.decision));
		return FSpatialGDKSpanId(false);
	}
	}
}

void SpatialEventTracer::ConstructTraceEventData(Trace_EventData* EventData, const FSpatialTraceEvent& SpatialTraceEvent)
{
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
}

void SpatialEventTracer::StreamDeleter::operator()(Io_Stream* StreamToDestroy) const
{
	Io_Stream_Destroy(StreamToDestroy);
}

void SpatialEventTracer::AddComponent(Worker_EntityId EntityId, Worker_ComponentId ComponentId, const FSpatialGDKSpanId& SpanId)
{
	if (!SpanId.IsValid())
	{
		return;
	}

	EntityComponentSpanIds.FindOrAdd({ EntityId, ComponentId }, SpanId);
}

void SpatialEventTracer::RemoveComponent(Worker_EntityId EntityId, Worker_ComponentId ComponentId)
{
	EntityComponentSpanIds.Remove({ EntityId, ComponentId });
}

void SpatialEventTracer::UpdateComponent(Worker_EntityId EntityId, Worker_ComponentId ComponentId, const FSpatialGDKSpanId& SpanId)
{
	if (!SpanId.IsValid())
	{
		return;
	}

	FSpatialGDKSpanId& StoredSpanId = EntityComponentSpanIds.FindChecked({ EntityId, ComponentId });

	FMultiGDKSpanIdAllocator SpanIdAllocator = FMultiGDKSpanIdAllocator(SpanId, StoredSpanId);
	FSpatialGDKSpanId NewSpanId = CreateSpan(SpanIdAllocator.GetBuffer(), SpanIdAllocator.GetNumSpanIds());
	TraceEvent(FSpatialTraceEventBuilder::CreateMergeComponentUpdate(EntityId, ComponentId), NewSpanId);

	StoredSpanId = NewSpanId;
}

FSpatialGDKSpanId SpatialEventTracer::GetSpanId(const EntityComponentId& Id) const
{
	const FSpatialGDKSpanId* SpanId = EntityComponentSpanIds.Find(Id);
	if (SpanId == nullptr)
	{
		return FSpatialGDKSpanId(true);
	}

	return *SpanId;
}

void SpatialEventTracer::AddToStack(const FSpatialGDKSpanId& SpanId)
{
	if (!SpanId.IsValid())
	{
		return;
	}

	SpanIdStack.Add(SpanId);
}

FSpatialGDKSpanId SpatialEventTracer::PopFromStack()
{
	if (SpanIdStack.Num() == 0)
	{
		return FSpatialGDKSpanId(true);
	}
	return SpanIdStack.Pop();
}

FSpatialGDKSpanId SpatialEventTracer::GetFromStack() const
{
	const int32 Size = SpanIdStack.Num();
	if (Size == 0)
	{
		return FSpatialGDKSpanId(true);
	}
	return SpanIdStack[Size - 1];
}

bool SpatialEventTracer::IsStackEmpty() const
{
	return SpanIdStack.Num() == 0;
}

void SpatialEventTracer::AddLatentPropertyUpdateSpanId(const TWeakObjectPtr<UObject>& Object, const FSpatialGDKSpanId& SpanId)
{
	if (!SpanId.IsValid())
	{
		return;
	}

	FSpatialGDKSpanId* ExistingSpanId = ObjectSpanIdStacks.Find(Object);
	if (ExistingSpanId == nullptr)
	{
		ObjectSpanIdStacks.Add(Object, SpanId);
	}
	else
	{
		FMultiGDKSpanIdAllocator SpanIdAllocator = FMultiGDKSpanIdAllocator(SpanId, *ExistingSpanId);
		*ExistingSpanId = CreateSpan(SpanIdAllocator.GetBuffer(), SpanIdAllocator.GetNumSpanIds());
	}
}

FSpatialGDKSpanId SpatialEventTracer::PopLatentPropertyUpdateSpanId(const TWeakObjectPtr<UObject>& Object)
{
	FSpatialGDKSpanId* ExistingSpanId = ObjectSpanIdStacks.Find(Object);
	if (ExistingSpanId == nullptr)
	{
		return FSpatialGDKSpanId(true);
	}

	FSpatialGDKSpanId TempSpanId = *ExistingSpanId;
	ObjectSpanIdStacks.Remove(Object);

	return TempSpanId;
}

} // namespace SpatialGDK
