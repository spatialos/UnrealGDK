// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "MergeComponentFieldEventTracingTest.h"
#include "Engine/World.h"

#include "SpatialFunctionalTestFlowController.h"

AMergeComponentFieldEventTracingTest::AMergeComponentFieldEventTracingTest()
{
	Author = "Matthew Sandford";
	Description = TEXT("Test checking the process RPC trace events have appropriate causes");

	FilterEventNames = { MergeComponentFieldUpdateEventName, ReceiveOpEventName };
	WorkerDefinition = FWorkerDefinition::Client(1);
}

void AMergeComponentFieldEventTracingTest::FinishEventTraceTest()
{
	int EventsTested = 0;
	int EventsFailed = 0;
	for (const auto& Pair : TraceEvents)
	{
		const FString SpanIdString = Pair.Key;
		const FName EventName = Pair.Value;

		if (EventName != MergeComponentFieldUpdateEventName)
		{
			continue;
		}

		EventsTested++;

		if (!CheckEventTraceCause(SpanIdString, { ReceiveOpEventName }))
		{
			EventsFailed++;
		}
	}

	bool bSuccess = EventsTested > 0 && EventsFailed == 0;
	AssertTrue(bSuccess,
			   FString::Printf(TEXT("Merge component field trace events have the expected causes. Events Tested: %d, Events Failed: %d"),
							   EventsTested, EventsFailed));

	FinishStep();
}
