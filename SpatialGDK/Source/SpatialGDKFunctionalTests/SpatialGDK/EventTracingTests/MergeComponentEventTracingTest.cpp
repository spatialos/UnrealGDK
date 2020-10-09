// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "MergeComponentEventTracingTest.h"

AMergeComponentEventTracingTest::AMergeComponentEventTracingTest()
{
	Author = "Matthew Sandford";
	Description = TEXT("Test checking the merge component field trace events have appropriate causes");

	FilterEventNames = { MergeComponentUpdateEventName, ReceiveOpEventName };
	WorkerDefinition = FWorkerDefinition::Client(1);
}

void AMergeComponentEventTracingTest::FinishEventTraceTest()
{
	int EventsTested = 0;
	int EventsFailed = 0;
	for (const auto& Pair : TraceEvents)
	{
		const FString& SpanIdString = Pair.Key;
		const FName EventName = Pair.Value;

		if (EventName != MergeComponentUpdateEventName)
		{
			continue;
		}

		EventsTested++;

		if (!CheckEventTraceCause(SpanIdString, { MergeComponentUpdateEventName, ReceiveOpEventName }, 2))
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
