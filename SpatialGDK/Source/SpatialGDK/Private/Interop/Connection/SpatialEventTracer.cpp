// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "Interop/Connection/SpatialEventTracer.h"

#include <string>

#include "HAL/PlatformFile.h"
#include "HAL/PlatformFilemanager.h"
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
			UE_LOG(LogSpatialEventTracer, Error, TEXT("Failed to serialize to with error code %d (%s)"), Code, Trace_GetLastError());
		}

		if (FPlatformAtomics::AtomicRead_Relaxed(&EventTracer->FlushOnWriteAtomic))
		{
			int64_t Flushresult = Io_Stream_Flush(Stream);
			if (Flushresult == -1)
			{
				UE_LOG(LogSpatialEventTracer, Error, TEXT("Failed to flush stream with error code %d (%s)"), Code,
					   Io_Stream_GetLastError(Stream));
			}
		}
	}
	else
	{
		EventTracer->BytesWrittenToStream = EventTracer->MaxFileSize;
	}
}

SpatialScopedActiveSpanId::SpatialScopedActiveSpanId(SpatialEventTracer* InEventTracer, const FSpatialGDKSpanId& InCurrentSpanId)
	: CurrentSpanId(InCurrentSpanId)
	, EventTracer(nullptr)
{
	if (InEventTracer == nullptr)
	{
		return;
	}

	EventTracer = InEventTracer->GetWorkerEventTracer();
	Trace_EventTracer_SetActiveSpanId(EventTracer, CurrentSpanId.GetConstId());
}

SpatialScopedActiveSpanId::~SpatialScopedActiveSpanId()
{
	if (EventTracer != nullptr)
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

	UEventTracingSamplingSettings* SamplingSettings = Settings->GetEventTracingSamplingSettings();

	UE_LOG(LogSpatialEventTracer, Log, TEXT("Setting event tracing sampling probability. Probability: %f."),
		   SamplingSettings->SamplingProbability);

	TArray<Trace_SpanSamplingProbability> SpanSamplingProbabilities;
	TArray<std::string> AnsiStrings; // Worker requires ansi const char*
	for (const auto& Pair : SamplingSettings->EventSamplingModeOverrides)
	{
		const FString& EventName = Pair.Key.ToString();
		UE_LOG(LogSpatialEventTracer, Log, TEXT("Adding trace event sampling override. Event: %s Probability: %f."), *EventName,
			   Pair.Value);
		int32 Index = AnsiStrings.Add((const char*)TCHAR_TO_ANSI(*EventName));
		SpanSamplingProbabilities.Add({ AnsiStrings[Index].c_str(), Pair.Value });
	}

	SamplingParameters.probabilistic_parameters.default_probability = SamplingSettings->SamplingProbability;
	SamplingParameters.probabilistic_parameters.probability_count = SpanSamplingProbabilities.Num();
	SamplingParameters.probabilistic_parameters.probabilities = SpanSamplingProbabilities.GetData();

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
	FUserSpanId UserSpanId;
	UserSpanId.Data.SetNum(TRACE_SPAN_ID_SIZE_BYTES);
	FMemory::Memcpy(UserSpanId.Data.GetData(), SpanId.GetConstId(), TRACE_SPAN_ID_SIZE_BYTES);
	return UserSpanId;
}

FSpatialGDKSpanId SpatialEventTracer::UserSpanIdToGDKSpanId(const FUserSpanId& UserSpanId)
{
	if (!UserSpanId.IsValid())
	{
		return {};
	}

	FSpatialGDKSpanId TraceSpanId;
	FMemory::Memcpy(TraceSpanId.GetId(), UserSpanId.Data.GetData(), TRACE_SPAN_ID_SIZE_BYTES);
	return TraceSpanId;
}

void SpatialEventTracer::StreamDeleter::operator()(Io_Stream* StreamToDestroy) const
{
	Io_Stream_Destroy(StreamToDestroy);
}

void SpatialEventTracer::BeginOpsForFrame()
{
	for (auto& ConsumedKey : EntityComponentsConsumed)
	{
		EntityComponentSpanIds.Remove(ConsumedKey);
	}
	EntityComponentsConsumed.Empty(EntityComponentsConsumed.Num());
}

void SpatialEventTracer::AddEntity(const Worker_AddEntityOp& Op, const FSpatialGDKSpanId& SpanId)
{
	Worker_EntityId EntityId = Op.entity_id;
	TraceEvent(RECEIVE_CREATE_ENTITY_EVENT_NAME, "", SpanId.GetConstId(), /* NumCauses */ 1,
			   [EntityId](FSpatialTraceEventDataBuilder& EventBuilder) {
				   EventBuilder.AddEntityId(EntityId);
			   });
}

void SpatialEventTracer::RemoveEntity(const Worker_RemoveEntityOp& Op, const FSpatialGDKSpanId& SpanId)
{
	Worker_EntityId EntityId = Op.entity_id;
	TraceEvent(RECEIVE_REMOVE_ENTITY_EVENT_NAME, "", SpanId.GetConstId(), /* NumCauses */ 1,
			   [EntityId](FSpatialTraceEventDataBuilder& EventBuilder) {
				   EventBuilder.AddEntityId(EntityId);
			   });
}

void SpatialEventTracer::AuthorityChange(const Worker_ComponentSetAuthorityChangeOp& Op, const FSpatialGDKSpanId& SpanId)
{
	Worker_EntityId EntityId = Op.entity_id;
	Worker_ComponentSetId ComponentSetId = Op.component_set_id;
	Worker_Authority Authority = static_cast<Worker_Authority>(Op.authority);

	TraceEvent(AUTHORITY_CHANGE_EVENT_NAME, "", SpanId.GetConstId(), /* NumCauses */ 1,
			   [EntityId, ComponentSetId, Authority](FSpatialTraceEventDataBuilder& EventBuilder) {
				   EventBuilder.AddEntityId(EntityId);
				   EventBuilder.AddComponentSetId(ComponentSetId);
				   EventBuilder.AddAuthority(Authority);
			   });
}

void SpatialEventTracer::AddComponent(const Worker_AddComponentOp& Op, const FSpatialGDKSpanId& SpanId)
{
	TArray<FSpatialGDKSpanId>& StoredSpanIds = EntityComponentSpanIds.FindOrAdd({ Op.entity_id, Op.data.component_id });
	StoredSpanIds.Push(SpanId);
}

void SpatialEventTracer::RemoveComponent(const Worker_RemoveComponentOp& Op, const FSpatialGDKSpanId& SpanId)
{
	EntityComponentSpanIds.Remove({ Op.entity_id, Op.component_id });
}

void SpatialEventTracer::UpdateComponent(const Worker_ComponentUpdateOp& Op, const FSpatialGDKSpanId& SpanId)
{
	TArray<FSpatialGDKSpanId>& StoredSpanIds = EntityComponentSpanIds.FindOrAdd({ Op.entity_id, Op.update.component_id });
	StoredSpanIds.Push(SpanId);
}

void SpatialEventTracer::CommandRequest(const Worker_CommandRequestOp& Op, const FSpatialGDKSpanId& SpanId)
{
	FSpatialGDKSpanId& StoredSpanId = RequestSpanIds.FindOrAdd({ Op.request_id });
	checkf(StoredSpanId.IsNull(), TEXT("CommandResponse received multiple times for request id %lld"), Op.request_id);
	StoredSpanId = SpanId;
}

TArray<FSpatialGDKSpanId> SpatialEventTracer::GetAndConsumeSpansForComponent(const EntityComponentId& Id)
{
	const TArray<FSpatialGDKSpanId>* StoredSpanIds = EntityComponentSpanIds.Find(Id);
	if (StoredSpanIds == nullptr)
	{
		return {};
	}
	EntityComponentsConsumed.Push(Id); // Consume on frame boundary instead, as these can have multiple uses.
	return *StoredSpanIds;
}

FSpatialGDKSpanId SpatialEventTracer::GetAndConsumeSpanForRequestId(Worker_RequestId RequestId)
{
	const FSpatialGDKSpanId* SpanId = RequestSpanIds.Find(RequestId);
	if (SpanId == nullptr)
	{
		return {};
	}
	FSpatialGDKSpanId GDKSpanId(*SpanId);
	RequestSpanIds.Remove(RequestId);
	return GDKSpanId;
}

void SpatialEventTracer::AddToStack(const FSpatialGDKSpanId& SpanId)
{
	SpanIdStack.Add(SpanId);
}

FSpatialGDKSpanId SpatialEventTracer::PopFromStack()
{
	if (SpanIdStack.Num() == 0)
	{
		return {};
	}
	return SpanIdStack.Pop();
}

FSpatialGDKSpanId SpatialEventTracer::GetFromStack() const
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
	FSpatialGDKSpanId* ExistingSpanId = ObjectSpanIdStacks.Find(Object);
	if (ExistingSpanId == nullptr)
	{
		ObjectSpanIdStacks.Add(Object, SpanId);
	}
	else
	{
		FSpatialGDKSpanId CauseSpanIds[2] = { SpanId, *ExistingSpanId };
		uint8_t* Causes = reinterpret_cast<uint8_t*>(&CauseSpanIds);
		const UObject* ObjectPtr = Object.Get();

		*ExistingSpanId = TraceEvent(MERGE_PROPERTY_UPDATE_EVENT_NAME, "", Causes, /* NumCauses */ 2,
									 [ObjectPtr](FSpatialTraceEventDataBuilder& EventBuilder) {
										 EventBuilder.AddObject(ObjectPtr);
									 });
	}
}

FSpatialGDKSpanId SpatialEventTracer::PopLatentPropertyUpdateSpanId(const TWeakObjectPtr<UObject>& Object)
{
	FSpatialGDKSpanId* ExistingSpanId = ObjectSpanIdStacks.Find(Object);
	if (ExistingSpanId == nullptr)
	{
		return {};
	}

	FSpatialGDKSpanId TempSpanId = *ExistingSpanId;
	ObjectSpanIdStacks.Remove(Object);

	return TempSpanId;
}

void SpatialEventTracer::SetFlushOnWrite(bool bValue)
{
	FPlatformAtomics::AtomicStore_Relaxed(&FlushOnWriteAtomic, bValue ? 1 : 0);
}

} // namespace SpatialGDK
