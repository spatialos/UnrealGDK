// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "EventTracingTest.h"

#include "EngineClasses/SpatialGameInstance.h"
#include "Interop/Connection/SpatialConnectionManager.h"
#include "Interop/Connection/SpatialEventTracer.h"
#include "Interop/Connection/SpatialWorkerConnection.h"
#include "SpatialCommonTypes.h"

#include <WorkerSDK/improbable/c_io.h>
#include <WorkerSDK/improbable/c_trace.h>

using namespace SpatialGDK;

const FName AEventTracingTest::ReceiveOpEventName = "worker.receive_op";
const FName AEventTracingTest::PropertyChangedEventName = "unreal_gdk.property_changed";
const FName AEventTracingTest::ReceivePropertyUpdateEventName = "unreal_gdk.receive_property_update";
const FName AEventTracingTest::PushRPCEventName = "unreal_gdk.push_rpc";
const FName AEventTracingTest::ProcessRPCEventName = "unreal_gdk.process_rpc";
const FName AEventTracingTest::ComponentUpdateEventName = "unreal_gdk.component_update";
const FName AEventTracingTest::MergeComponentUpdateEventName = "unreal_gdk.merge_component_update";
const FName AEventTracingTest::UserProcessRPCEventName = "user.process_rpc";
const FName AEventTracingTest::UserReceivePropertyEventName = "user.receive_property";
const FName AEventTracingTest::UserReceiveComponentPropertyEventName = "user.receive_component_property";
const FName AEventTracingTest::UserSendPropertyEventName = "user.send_property";
const FName AEventTracingTest::UserSendComponentPropertyEventName = "user.send_component_property";
const FName AEventTracingTest::UserSendRPCEventName = "user.send_rpc";

AEventTracingTest::AEventTracingTest()
{
	Author = "Matthew Sandford";
	Description = TEXT("Base class for event tracing tests");

	SetNumRequiredClients(1);
}

void AEventTracingTest::PrepareTest()
{
	Super::PrepareTest();

	AddStep(
		TEXT("StartEventTracingTest"), WorkerDefinition, nullptr,
		[this]() {
			StartEventTracingTest();
		},
		nullptr);

	AddStep(TEXT("WaitForTestToEnd"), WorkerDefinition, nullptr, nullptr, [this](float DeltaTime) {
		WaitForTestToEnd();
	});

	AddStep(
		TEXT("GatherData"), WorkerDefinition, nullptr,
		[this]() {
			GatherData();
		},
		nullptr);

	AddStep(
		TEXT("FinishEventTraceTest"), WorkerDefinition, nullptr,
		[this]() {
			FinishEventTraceTest();
		},
		nullptr);
}

void AEventTracingTest::StartEventTracingTest()
{
	TestStartTime = FDateTime::Now();
	FinishStep();
}

void AEventTracingTest::WaitForTestToEnd()
{
	if (TestStartTime + FTimespan::FromSeconds(TestTime) > FDateTime::Now())
	{
		return;
	}

	FinishStep();
}

void AEventTracingTest::FinishEventTraceTest()
{
	FinishStep();
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
	FileManager.FindFiles(Files, *EventsFolderPath, *FString(".trace"));

	struct FileCreationTime
	{
		FString FilePath;
		FDateTime CreationTime;
	};

	TArray<FileCreationTime> FileCreationTimes;
	for (const FString& File : Files)
	{
		FString FilePath = FPaths::Combine(EventsFolderPath, File);
		FileCreationTimes.Add({ FilePath, FileManager.GetTimeStamp(*FilePath) });
	}

	FileCreationTimes.Sort([](const FileCreationTime& A, const FileCreationTime& B) {
		return A.CreationTime > B.CreationTime;
	});

	if (FileCreationTimes.Num() >= 2)
	{
		GatherDataFromFile(FileCreationTimes[0].FilePath);
		GatherDataFromFile(FileCreationTimes[1].FilePath);
	}

	FinishStep();
}

void AEventTracingTest::GatherDataFromFile(const FString& FilePath)
{
	struct StreamDeleter
	{
		void operator()(Io_Stream* StreamToDestroy) const { Io_Stream_Destroy(StreamToDestroy); };
	};

	TUniquePtr<Io_Stream, StreamDeleter> Stream;
	Stream.Reset(Io_CreateFileStream(TCHAR_TO_ANSI(*FilePath), Io_OpenMode::IO_OPEN_MODE_READ));

	uint32_t BytesToRead = 1;
	int8_t ReturnCode = 1;
	while (BytesToRead != 0 && ReturnCode == 1)
	{
		BytesToRead = Trace_GetNextSerializedItemSize(Stream.Get());

		Trace_Item* Item = Trace_Item_GetThreadLocal();
		if (BytesToRead != 0)
		{
			ReturnCode = Trace_DeserializeItemFromStream(Stream.Get(), Item, BytesToRead);
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
					FName& CachedEventName = TraceEvents.FindOrAdd(SpanIdString);
					CachedEventName = EventName;
				}
			}
			else if (Item->item_type == TRACE_ITEM_TYPE_SPAN)
			{
				const Trace_Span& Span = Item->item.span;

				FString SpanIdString = FSpatialGDKSpanId::ToString(Span.id);
				TArray<FString>& Causes = TraceSpans.FindOrAdd(SpanIdString);
				for (uint64 i = 0; i < Span.cause_count; ++i)
				{
					const int32 ByteOffset = i * TRACE_SPAN_ID_SIZE_BYTES;
					Causes.Add(FSpatialGDKSpanId::ToString(Span.causes + ByteOffset));
				}
			}
		}
	}

	Stream = nullptr;
}

bool AEventTracingTest::CheckEventTraceCause(const FString& SpanIdString, const TArray<FName>& CauseEventNames, int MinimumCauses /*= 1*/)
{
	TArray<FString>* Causes = TraceSpans.Find(SpanIdString);
	if (Causes == nullptr || Causes->Num() < MinimumCauses)
	{
		return false;
	}

	for (const FString& CauseSpanIdString : *Causes)
	{
		const FName* CauseEventName = TraceEvents.Find(CauseSpanIdString);
		if (CauseEventName == nullptr)
		{
			return false;
		}
		if (!CauseEventNames.Contains(*CauseEventName))
		{
			return false;
		}
	}

	return true;
}
