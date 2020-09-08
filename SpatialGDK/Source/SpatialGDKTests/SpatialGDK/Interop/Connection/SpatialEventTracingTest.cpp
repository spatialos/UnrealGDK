// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "Tests/TestDefinitions.h"

#include "Containers/Map.h"
#include "Engine/Engine.h"
#include "Engine/EngineTypes.h"
#include "EngineClasses/SpatialGameInstance.h"
#include "GameFramework/GameStateBase.h"
#include "Interop/Connection/SpatialConnectionManager.h"
#include "Interop/Connection/SpatialEventTracer.h"
#include "Templates/SharedPointer.h"
#include "Tests/AutomationCommon.h"

#include <WorkerSDK/improbable/c_trace.h>

#define EVENT_TRACING_TEST(TestName) GDK_TEST(Core, SpatialEventTracer, TestName)

using namespace SpatialGDK;
using namespace worker::c;

namespace
{
FName SendRPCEventName = "unreal_gdk.send_rpc";

struct TestTraceEvent
{
	FName EventName;
	FString SpanIdString;
	TArray<FString> CauseSpanIdStrings;
};

struct EventTracingTestData
{
	UWorld* TestWorld;
	FDateTime WorldStartTime;
	static TArray<FName> FilterEventNames;
	static TMap<FString, TestTraceEvent> TestTraceEvents;

	static void TraceCallback(void* UserData, const Trace_Item* Item)
	{
		SpatialEventTracer* EventTracer = static_cast<SpatialEventTracer*>(UserData);

		if (Item->item_type == TRACE_ITEM_TYPE_EVENT)
		{
			const Trace_Event& Event = Item->item.event;
			FString SpanIdString = SpatialEventTracer::SpanIdToString(Event.span_id);

			FName EventName = FName(FString(Event.type));
			if (FilterEventNames.Num() == 0 || FilterEventNames.Contains(EventName))
			{
				TestTraceEvent& TestEvent = EventTracingTestData::TestTraceEvents.FindOrAdd(SpanIdString);
				TestEvent.EventName = EventName;
				TestEvent.SpanIdString = SpanIdString;
			}
			else
			{
				EventTracingTestData::TestTraceEvents.Remove(SpanIdString);
			}
		}
		else
		{
			const Trace_Span& Span = Item->item.span;

			FString SpanIdString = SpatialEventTracer::SpanIdToString(Span.id);
			TestTraceEvent& TestEvent = EventTracingTestData::TestTraceEvents.FindOrAdd(SpanIdString);
			TestEvent.SpanIdString = SpanIdString;
			for (uint64 i = 0; i < Span.cause_count; ++i)
			{
				TestEvent.CauseSpanIdStrings.Add(SpatialEventTracer::SpanIdToString(Span.causes[i]));
			}
		}
	}
};

TArray<FName> EventTracingTestData::FilterEventNames = {};
TMap<FString, TestTraceEvent> EventTracingTestData::TestTraceEvents = {};

struct TestDataDeleter
{
	void operator()(EventTracingTestData* Data) const noexcept { delete Data; }
};

TSharedPtr<EventTracingTestData> MakeNewTestData()
{
	TSharedPtr<EventTracingTestData> Data(new EventTracingTestData);
	return Data;
}

// Copied from AutomationCommon::GetAnyGameWorld().
UWorld* GetAnyGameWorld()
{
	UWorld* World = nullptr;
	const TIndirectArray<FWorldContext>& WorldContexts = GEngine->GetWorldContexts();
	for (const FWorldContext& Context : WorldContexts)
	{
		if ((Context.WorldType == EWorldType::PIE || Context.WorldType == EWorldType::Game) && (Context.World() != nullptr))
		{
			World = Context.World();
			break;
		}
	}

	return World;
}
} // anonymous namespace

DEFINE_LATENT_AUTOMATION_COMMAND_ONE_PARAMETER(FSetupWorld, TSharedPtr<EventTracingTestData>, Data);
bool FSetupWorld::Update()
{
	Data->TestWorld = GetAnyGameWorld();

	if (Data->TestWorld && Data->TestWorld->AreActorsInitialized())
	{
		AGameStateBase* GameState = Data->TestWorld->GetGameState();
		if (GameState && GameState->HasMatchStarted())
		{
			Data->WorldStartTime = FDateTime::Now();
			return true;
		}
	}

	return false;
}

DEFINE_LATENT_AUTOMATION_COMMAND_ONE_PARAMETER(FStartSendRPCTest, TSharedPtr<EventTracingTestData>, Data);
bool FStartSendRPCTest::Update()
{
	USpatialGameInstance* GameInstance = Cast<USpatialGameInstance>(Data->TestWorld->GetGameInstance());
	USpatialConnectionManager* ConnectionManager = GameInstance->GetSpatialConnectionManager();
	SpatialEventTracer* EventTracer = ConnectionManager->GetEventTracer();

	EventTracer->RestartWithCallback(&EventTracingTestData::TraceCallback);

	EventTracingTestData::FilterEventNames.Add("SendRPCEventName");

	return true;
}

DEFINE_LATENT_AUTOMATION_COMMAND_TWO_PARAMETER(FLoopSendRPCTest, TSharedPtr<EventTracingTestData>, Data, float, TestTime);
bool FLoopSendRPCTest::Update()
{
	if (Data->WorldStartTime + FTimespan::FromSeconds(TestTime) > FDateTime::Now())
	{
		return false;
	}
	return true;
}

DEFINE_LATENT_AUTOMATION_COMMAND_TWO_PARAMETER(FFinishSendRPCTest, FAutomationTestBase*, Test, TSharedPtr<EventTracingTestData>, Data);
bool FFinishSendRPCTest::Update()
{
	bool bSuccess = true;

	for (const auto& Pair : EventTracingTestData::TestTraceEvents)
	{
		const TestTraceEvent& TraceEvent = Pair.Value;

		if (TraceEvent.EventName != SendRPCEventName)
		{
			continue;
		}

		if (TraceEvent.CauseSpanIdStrings.Num() == 0)
		{
			bSuccess = false;
			break;
		}

		for (const FString& CauseSpanIdString : TraceEvent.CauseSpanIdStrings)
		{
			const TestTraceEvent* CauseTraceEvent = EventTracingTestData::TestTraceEvents.Find(CauseSpanIdString);
			if (CauseTraceEvent == nullptr || CauseTraceEvent->EventName == SendRPCEventName)
			{
				bSuccess = false;
				break;
			}
		}

		if (!bSuccess)
		{
			break;
		}
	}

	Test->TestTrue(FString::Printf(TEXT("Send RPC trace events look good!")), bSuccess);

	return true;
}

EVENT_TRACING_TEST(GIVEN_a_game_scenario_WHEN_send_rpc_traces_are_created_THEN_they_have_appropriate_causes)
{
	AutomationOpenMap("/Game/Content/Maps/oflgym");

	TSharedPtr<EventTracingTestData> Data = MakeNewTestData();

	ADD_LATENT_AUTOMATION_COMMAND(FSetupWorld(Data));
	ADD_LATENT_AUTOMATION_COMMAND(FStartSendRPCTest(Data));
	ADD_LATENT_AUTOMATION_COMMAND(FLoopSendRPCTest(Data, 20.0f));
	ADD_LATENT_AUTOMATION_COMMAND(FFinishSendRPCTest(this, Data));

	return true;
}
