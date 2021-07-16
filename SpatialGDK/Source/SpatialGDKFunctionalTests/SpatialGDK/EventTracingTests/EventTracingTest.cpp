// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "EventTracingTest.h"

#include "EngineClasses/SpatialGameInstance.h"
#include "Interop/Connection/SpatialConnectionManager.h"
#include "Interop/Connection/SpatialEventTracer.h"
#include "Interop/Connection/SpatialWorkerConnection.h"
#include "SpatialCommonTypes.h"

#include <WorkerSDK/improbable/c_io.h>
#include <WorkerSDK/improbable/c_trace.h>

DEFINE_LOG_CATEGORY(LogEventTracingTest);

using namespace SpatialGDK;

const FName AEventTracingTest::ReceiveOpEventName = "worker.receive_op";
const FName AEventTracingTest::PropertyChangedEventName = "unreal_gdk.property_changed";
const FName AEventTracingTest::ReceivePropertyUpdateEventName = "unreal_gdk.receive_property_update";
const FName AEventTracingTest::PushRPCEventName = "unreal_gdk.push_rpc";
const FName AEventTracingTest::ReceiveRPCEventName = "unreal_gdk.receive_rpc";
const FName AEventTracingTest::ApplyRPCEventName = "unreal_gdk.apply_rpc";
const FName AEventTracingTest::ComponentUpdateEventName = "unreal_gdk.component_update";
const FName AEventTracingTest::UserProcessRPCEventName = "user.process_rpc";
const FName AEventTracingTest::UserReceivePropertyEventName = "user.receive_property";
const FName AEventTracingTest::UserReceiveComponentPropertyEventName = "user.receive_component_property";
const FName AEventTracingTest::UserSendPropertyEventName = "user.send_property";
const FName AEventTracingTest::UserSendComponentPropertyEventName = "user.send_component_property";
const FName AEventTracingTest::UserSendRPCEventName = "user.send_rpc";

const FName AEventTracingTest::UserSendCrossServerPropertyEventName = "user.send_cross_server_property";
const FName AEventTracingTest::UserSendCrossServerRPCEventName = "user.send_cross_server_rpc";
const FName AEventTracingTest::UserReceiveCrossServerPropertyEventName = "user.receive_cross_server_property";
const FName AEventTracingTest::UserReceiveCrossServerRPCEventName = "user.receive_cross_server_rpc";
const FName AEventTracingTest::ApplyCrossServerRPCName = "unreal_gdk.apply_cross_server_rpc";
const FName AEventTracingTest::SendCrossServerRPCName = "unreal_gdk.send_cross_server_rpc";
const FName AEventTracingTest::ReceiveCrossServerRPCName = "unreal_gdk.receive_cross_server_rpc";

AEventTracingTest::AEventTracingTest()
{
	Author = "Matthew Sandford";
	Description = TEXT("Base class for event tracing tests");
}

void AEventTracingTest::PrepareTest()
{
	Super::PrepareTest();

	AddStep(
		TEXT("StartEventTracingTest"), WorkerDefinition, nullptr,
		[this]() {
			StartEventTracingTest();
			FinishStep();
		},
		nullptr);

	AddStep(
		TEXT("SetFlushMode"), FWorkerDefinition::AllWorkers, nullptr,
		[this]() {
			USpatialGameInstance* GameInstance = GetGameInstance<USpatialGameInstance>();
			USpatialConnectionManager* ConnectionManager = GameInstance->GetSpatialConnectionManager();
			SpatialEventTracer* EventTracer = ConnectionManager->GetWorkerConnection()->GetEventTracer();
			if (EventTracer)
			{
				EventTracer->SetFlushOnWrite(true);
			}
			FinishStep();
		},
		nullptr);

	AddStep(TEXT("WaitForTestToEnd"), WorkerDefinition, nullptr, nullptr, [this](float DeltaTime) {
		WaitForTestToEnd();
		FinishStep();
	});

	AddStep(
		TEXT("GatherData"), WorkerDefinition, nullptr,
		[this]() {
			GatherData();
			FinishStep();
		},
		nullptr);

	AddStep(
		TEXT("FinishEventTraceTest"), WorkerDefinition, nullptr,
		[this]() {
			FinishEventTraceTest();
			FinishStep();
		},
		nullptr);
}

void AEventTracingTest::StartEventTracingTest()
{
	TestStartTime = FDateTime::Now();
}

void AEventTracingTest::WaitForTestToEnd()
{
	const bool bTimerFinished = FDateTime::Now() > TestStartTime + FTimespan::FromSeconds(TestTime);
	RequireEqual_Bool(bTimerFinished, true, TEXT("Waiting for test to end."));
}

void AEventTracingTest::GatherData()
{
	USpatialGameInstance* GameInstance = GetGameInstance<USpatialGameInstance>();
	USpatialConnectionManager* ConnectionManager = GameInstance->GetSpatialConnectionManager();
	SpatialEventTracer* EventTracer = ConnectionManager->GetWorkerConnection()->GetEventTracer();
	if (EventTracer == nullptr)
	{
		return;
	}

	FString EventsFolderPath = EventTracer->GetFolderPath();

	IFileManager& FileManager = IFileManager::Get();

	TArray<FString> Files;
	FileManager.FindFilesRecursive(Files, *EventsFolderPath, *FString("*.etlog"), true, false);

	if (Files.Num() < 2)
	{
		UE_LOG(LogEventTracingTest, Error, TEXT("Could not find all required event tracing files"));
		return;
	}

	struct FileCreationTime
	{
		FString FilePath;
		FDateTime CreationTime;
	};

	TArray<FileCreationTime> FileCreationTimes;
	for (const FString& File : Files)
	{
		FileCreationTimes.Add({ File, FileManager.GetTimeStamp(*File) });
	}

	FileCreationTimes.Sort([](const FileCreationTime& A, const FileCreationTime& B) {
		return A.CreationTime > B.CreationTime;
	});

	int RequiredRuntime = 1;
	int RequiredClients = GetRequiredClients();
	int RequiredWorkers = GetRequiredWorkers();

	int FoundRuntime = 0;
	int FoundClient = 0;
	int FoundWorker = 0;

	bool FoundAllEventTraceLogs = false;

	for (const FileCreationTime& FileCreation : FileCreationTimes)
	{
		if (FoundClient != RequiredClients && FileCreation.FilePath.Contains(TEXT("client")))
		{
			GatherDataFromFile(FileCreation.FilePath, TraceSource::Client);
			FoundClient++;
		}

		if (FoundWorker != RequiredWorkers && FileCreation.FilePath.Contains(TEXT("worker")))
		{
			GatherDataFromFile(FileCreation.FilePath, TraceSource::Worker);
			FoundWorker++;
		}

		if (FoundRuntime != RequiredRuntime && FileCreation.FilePath.Contains(TEXT("runtime")))
		{
			GatherDataFromFile(FileCreation.FilePath, TraceSource::Runtime);
			FoundRuntime++;
		}

		if (FoundClient == RequiredClients && FoundWorker == RequiredWorkers && FoundRuntime == RequiredRuntime)
		{
			FoundAllEventTraceLogs = true;
			break;
		}
	}

	if (!FoundAllEventTraceLogs)
	{
		UE_LOG(LogEventTracingTest, Error, TEXT("Could not find all required event tracing files"));
	}
}

void AEventTracingTest::GatherDataFromFile(const FString& FilePath, const TraceSource Source)
{
	struct StreamDeleter
	{
		void operator()(Io_Stream* StreamToDestroy) const { Io_Stream_Destroy(StreamToDestroy); };
	};

	TUniquePtr<Io_Stream, StreamDeleter> Stream;
	Stream.Reset(Io_CreateFileStream(TCHAR_TO_ANSI(*FilePath), Io_OpenMode::IO_OPEN_MODE_READ));

	TraceItemsData& SourceTraceItems = TraceItems.FindOrAdd(Source);

	int64_t BytesRead = 1;
	while (BytesRead > 0)
	{
		Trace_Item* Item = Trace_Item_GetThreadLocal();
		BytesRead = Trace_DeserializeItemFromStream(Stream.Get(), Item);

		if (BytesRead == 0)
		{
			break;
		}

		if (BytesRead == WORKER_RESULT_FAILURE)
		{
			UE_LOG(LogEventTracingTest, Error, TEXT("Failed to deserialize with error code %lld (%s)"), BytesRead,
				   ANSI_TO_TCHAR(Trace_GetLastError()));
			break;
		}

		if (Item != nullptr)
		{
			if (Item->item_type == TRACE_ITEM_TYPE_EVENT)
			{
				const Trace_Event& Event = Item->item.event;
				FName EventName = FName(*FString(Event.type));

				if (FilterEventNames.Num() == 0 || FilterEventNames.Contains(EventName))
				{
					FString SpanIdString = FSpatialGDKSpanId::ToString(Event.span_id);
					FName& CachedEventName = SourceTraceItems.SpanEvents.FindOrAdd(SpanIdString);
					CachedEventName = EventName;

					SourceTraceItems.EventCount.FindOrAdd(EventName)++;
				}
			}
			else if (Item->item_type == TRACE_ITEM_TYPE_SPAN)
			{
				const Trace_Span& Span = Item->item.span;

				FString SpanIdString = FSpatialGDKSpanId::ToString(Span.id);
				TArray<FString>& Causes = SourceTraceItems.Spans.FindOrAdd(SpanIdString);
				for (uint64 i = 0; i < Span.cause_count; ++i)
				{
					const int32 ByteOffset = i * TRACE_SPAN_ID_SIZE_BYTES;
					FSpatialGDKSpanId SpanId(Span.causes + ByteOffset);
					if (!SpanId.IsNull())
					{
						Causes.Add(FSpatialGDKSpanId::ToString(SpanId.GetId()));
					}
				}
			}
		}
	}

	Stream = nullptr;
}

int32 AEventTracingTest::GetTraceSpanCount(const TraceSource Source) const
{
	const TraceItemsData* SourceTraceItems = TraceItems.Find(Source);
	return SourceTraceItems != nullptr ? SourceTraceItems->Spans.Num() : 0;
}

int32 AEventTracingTest::GetTraceEventCount(const TraceSource Source) const
{
	const TraceItemsData* SourceTraceItems = TraceItems.Find(Source);
	if (SourceTraceItems == nullptr)
	{
		return 0;
	}

	int32 TotalCount = 0;
	for (const auto& Pair : SourceTraceItems->EventCount)
	{
		TotalCount += Pair.Value;
	}
	return TotalCount;
}

int32 AEventTracingTest::GetTraceEventCount(const TraceSource Source, const FName TraceEventType) const
{
	const TraceItemsData* SourceTraceItems = TraceItems.Find(Source);
	if (SourceTraceItems == nullptr)
	{
		return 0;
	}

	const int32* Count = SourceTraceItems->EventCount.Find(TraceEventType);
	return Count != nullptr ? *Count : 0;
}

FString AEventTracingTest::FindRootSpanId(const FString& InputSpanId) const
{
	FString Cause = InputSpanId;

	// Get the cause SpanId of the InputSpanId
	ForEachTraceSource([&InputSpanId, &Cause](const TraceItemsData& SourceTraceItems) {
		const TArray<FString>* Causes = SourceTraceItems.Spans.Find(InputSpanId);
		if (Causes != nullptr)
		{
			for (const FString& CauseSpanId : *Causes)
			{
				if (!CauseSpanId.IsEmpty())
				{
					Cause = CauseSpanId;
					return true;
				}
			}
		}
		return false;
	});

	// If Cause equals SpanId then the SpanId is a root SpanId
	// If not we will keep going down the span chain
	if (Cause != InputSpanId)
	{
		const FString NextCause = FindRootSpanId(Cause);
		return NextCause == Cause ? Cause : NextCause;
	}
	return InputSpanId;
}

void AEventTracingTest::ForEachTraceSource(TFunctionRef<bool(const TraceItemsData& SourceTraceItems)> Predicate) const
{
	for (int32 i = 0; i < TraceSource::Count; ++i)
	{
		const TraceSource Source = static_cast<TraceSource>(i);
		const TraceItemsData* SourceTraceItems = TraceItems.Find(Source);
		if (SourceTraceItems != nullptr)
		{
			if (Predicate(*SourceTraceItems))
			{
				break;
			}
		}
	}
}

bool AEventTracingTest::CheckEventTraceCause(const FString& SpanIdString, const TArray<FName>& CauseEventNames,
											 int MinimumCauses /*= 1*/) const
{
	bool bSuccess = true;
	ForEachTraceSource([this, &SpanIdString, &MinimumCauses, &CauseEventNames, &bSuccess](const TraceItemsData& SourceTraceItems) {
		const TArray<FString>* Causes = SourceTraceItems.Spans.Find(SpanIdString);
		if (Causes == nullptr)
		{
			return false;
		}

		if (Causes->Num() < MinimumCauses)
		{
			bSuccess = false;
			return true;
		}

		for (const FString& CauseSpanIdString : *Causes)
		{
			ForEachTraceSource([&CauseEventNames, &bSuccess, &CauseSpanIdString](const TraceItemsData& SubSourceTraceItems) {
				const FName* CauseEventName = SubSourceTraceItems.SpanEvents.Find(CauseSpanIdString);
				if (CauseEventName == nullptr)
				{
					return false;
				}
				bSuccess &= CauseEventNames.Contains(*CauseEventName);
				return true;
			});
		}
		return true;
	});

	return bSuccess;
}

AEventTracingTest::CheckResult AEventTracingTest::CheckCauses(FName From, FName To) const
{
	int EventsTested = 0;
	int EventsFailed = 0;

	ForEachTraceSource([this, From, To, &EventsTested, &EventsFailed](const TraceItemsData& SourceTraceItems) {
		for (const auto& Pair : SourceTraceItems.SpanEvents)
		{
			const FString& SpanIdString = Pair.Key;
			const FName& EventName = Pair.Value;

			if (EventName != To)
			{
				continue;
			}

			EventsTested++;

			if (!CheckEventTraceCause(SpanIdString, { From }))
			{
				EventsFailed++;
			}
		}
		return false;
	});

	return CheckResult{ EventsTested, EventsFailed };
	}
