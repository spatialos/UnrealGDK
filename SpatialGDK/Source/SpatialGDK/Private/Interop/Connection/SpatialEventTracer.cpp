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

SpatialScopedActiveSpanId::SpatialScopedActiveSpanId(SpatialEventTracer* InEventTracer, const TOptional<FSpatialGDKSpanId>& InCurrentSpanId)
	: CurrentSpanId(InCurrentSpanId)
	, EventTracer(InEventTracer->GetWorkerEventTracer())
{
	if (InCurrentSpanId.IsSet())
	{
		Trace_EventTracer_SetActiveSpanId(EventTracer, CurrentSpanId.GetValue().Data);
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
		UE_LOG(LogSpatialEventTracer, Log, TEXT("Spatial event tracing disabled."));
		Trace_EventTracer_Destroy(EventTracer);
	}
}

FString SpatialEventTracer::GDKSpanIdToString(const FSpatialGDKSpanId& SpanId)
{
	FString HexStr;
	for (int i = 0; i < TRACE_SPAN_ID_SIZE_BYTES; i++)
	{
		HexStr += FString::Printf(TEXT("%02x"), SpanId.Data[i]);
	}
	return HexStr;
}

FUserSpanId SpatialEventTracer::GDKSpanIdToUserSpanId(const FSpatialGDKSpanId& SpanId)
{
	FUserSpanId UserSpanId;
	UserSpanId.Data.SetNum(TRACE_SPAN_ID_SIZE_BYTES);
	const int32 Size = TRACE_SPAN_ID_SIZE_BYTES * sizeof(uint8);
	FMemory::Memcpy(UserSpanId.Data.GetData(), SpanId.Data, Size);
	return UserSpanId;
}

TOptional<FSpatialGDKSpanId> SpatialEventTracer::UserSpanIdToGDKSpanId(const FUserSpanId& UserSpanId)
{
	if (!UserSpanId.IsValid())
	{
		return {};
	}

	FSpatialGDKSpanId TraceSpanId;
	const int32 Size = TRACE_SPAN_ID_SIZE_BYTES * sizeof(uint8);
	FMemory::Memcpy(TraceSpanId.Data, UserSpanId.Data.GetData(), Size);
	return TraceSpanId;
}

TOptional<FSpatialGDKSpanId> SpatialEventTracer::CreateSpan() const
{
	if (!IsEnabled())
	{
		return {};
	}

	FSpatialGDKSpanId TraceSpanId;
	Trace_EventTracer_AddSpan(EventTracer, nullptr, 0, nullptr, TraceSpanId.Data);
	return TraceSpanId;
}

TOptional<FSpatialGDKSpanId> SpatialEventTracer::CreateSpan(const Trace_SpanIdType* Causes, int32 NumCauses) const
{
	if (!IsEnabled())
	{
		return {};
	}

	FSpatialGDKSpanId TraceSpanId;

	if (Causes != nullptr && NumCauses > 0)
	{
		Trace_EventTracer_AddSpan(EventTracer, Causes, NumCauses, nullptr, TraceSpanId.Data);
	}
	else
	{
		Trace_EventTracer_AddSpan(EventTracer, nullptr, 0, nullptr, TraceSpanId.Data);
	}

	return TraceSpanId;
}

void SpatialEventTracer::TraceEvent(const FSpatialTraceEvent& SpatialTraceEvent, const TOptional<FSpatialGDKSpanId>& SpanId)
{
	if (!IsEnabled())
	{
		return;
	}

	if (!SpanId.IsSet())
	{
		return;
	}

	auto MessageSrc = StringCast<ANSICHAR>(*SpatialTraceEvent.Message);
	const ANSICHAR* Message = MessageSrc.Get();

	auto TypeSrc = StringCast<ANSICHAR>(*SpatialTraceEvent.Type.ToString());
	const ANSICHAR* Type = TypeSrc.Get();

	Trace_Event TraceEvent{ SpanId.GetValue().Data, /* unix_timestamp_millis: ignored */ 0, Message, Type, nullptr };
	Trace_SamplingResult SamplingResult = Trace_EventTracer_ShouldSampleEvent(EventTracer, &TraceEvent);
	if (SamplingResult.decision == Trace_SamplingDecision::TRACE_SHOULD_NOT_SAMPLE)
	{
		return;
	}

	if (SamplingResult.decision == Trace_SamplingDecision::TRACE_SHOULD_SAMPLE_WITHOUT_DATA)
	{
		Trace_EventTracer_AddEvent(EventTracer, &TraceEvent);
		return;
	}

	if (SamplingResult.decision != Trace_SamplingDecision::TRACE_SHOULD_SAMPLE)
	{
		return;
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
}

bool SpatialEventTracer::IsEnabled() const
{
	return bEnabled;
}

void SpatialEventTracer::Enable(const FString& FileName)
{
	Trace_EventTracer_Parameters parameters = {};
	parameters.user_data = this;
	parameters.callback = &SpatialEventTracer::TraceCallback;
	EventTracer = Trace_EventTracer_Create(&parameters);
	bEnabled = true;

	Trace_SamplingParameters SamplingParameters = {};
	SamplingParameters.sampling_mode = Trace_SamplingMode::TRACE_SAMPLING_MODE_PROBABILISTIC;

	const USpatialGDKSettings* Settings = GetDefault<USpatialGDKSettings>();

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
	const FString FullFileName = FString::Printf(TEXT("EventTrace_%s_%s.trace"), *FileName, *FDateTime::Now().ToString());
	const FString FilePath = FPaths::Combine(FolderPath, FullFileName);

	IPlatformFile& PlatformFile = FPlatformFileManager::Get().GetPlatformFile();
	if (PlatformFile.CreateDirectoryTree(*FolderPath))
	{
		UE_LOG(LogSpatialEventTracer, Log, TEXT("Capturing trace to %s."), *FilePath);
		Stream.Reset(Io_CreateFileStream(TCHAR_TO_ANSI(*FilePath), Io_OpenMode::IO_OPEN_MODE_WRITE));
	}
}

void SpatialEventTracer::StreamDeleter::operator()(Io_Stream* StreamToDestroy) const
{
	Io_Stream_Destroy(StreamToDestroy);
}

void SpatialEventTracer::AddComponent(Worker_EntityId EntityId, Worker_ComponentId ComponentId, const FSpatialGDKSpanId& SpanId)
{
	if (SpanId.IsValid())
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
	if (SpanId.IsValid())
	{
		return;
	}

	FSpatialGDKSpanId& StoredSpanId = EntityComponentSpanIds.FindChecked({ EntityId, ComponentId });

	FMultiGDKSpanIdAllocator SpanIdAllocator = FMultiGDKSpanIdAllocator(SpanId, StoredSpanId);
	TOptional<FSpatialGDKSpanId> NewSpanId = CreateSpan(SpanIdAllocator.GetBuffer(), SpanIdAllocator.GetNumSpanIds());
	TraceEvent(FSpatialTraceEventBuilder::CreateMergeComponentUpdate(EntityId, ComponentId), NewSpanId);

	StoredSpanId = NewSpanId.GetValue();
}

TOptional<FSpatialGDKSpanId> SpatialEventTracer::GetSpanId(const EntityComponentId& Id) const
{
	if (!IsEnabled())
	{
		return {};
	}

	const FSpatialGDKSpanId* SpanId = EntityComponentSpanIds.Find(Id);
	if (!ensure(SpanId != nullptr))
	{
		return {};
	}

	return *SpanId;
}

void SpatialEventTracer::AddToStack(const FSpatialGDKSpanId& SpanId)
{
	SpanIdStack.Add(SpanId);
}

TOptional<FSpatialGDKSpanId> SpatialEventTracer::PopFromStack()
{
	return SpanIdStack.Pop();
}

TOptional<FSpatialGDKSpanId> SpatialEventTracer::GetFromStack() const
{
	const int32 Size = SpanIdStack.Num();
	if (Size == 0)
	{
		return {};
	}

	return SpanIdStack[Size - 1];
}

bool SpatialEventTracer::IsStackEmpty() const
{
	return SpanIdStack.Num() == 0;
}

void SpatialEventTracer::AddLatentPropertyUpdateSpanId(const TWeakObjectPtr<UObject>& Object, const FSpatialGDKSpanId& SpanId)
{
	if (!IsEnabled())
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
		*ExistingSpanId = CreateSpan(SpanIdAllocator.GetBuffer(), SpanIdAllocator.GetNumSpanIds()).GetValue();
	}
}

TOptional<FSpatialGDKSpanId> SpatialEventTracer::PopLatentPropertyUpdateSpanId(const TWeakObjectPtr<UObject>& Object)
{
	if (!IsEnabled())
	{
		return {};
	}

	FSpatialGDKSpanId* ExistingSpanId = ObjectSpanIdStacks.Find(Object);
	if (ExistingSpanId == nullptr)
	{
		return {};
	}

	FSpatialGDKSpanId TempSpanId = *ExistingSpanId;
	ObjectSpanIdStacks.Remove(Object);

	return TempSpanId;
}

} // namespace SpatialGDK
