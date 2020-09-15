// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "PropertyUpdateEventTracingTest.h"

APropertyUpdateEventTracingTest::APropertyUpdateEventTracingTest()
{
	Author = "Matthew Sandford";
	Description = TEXT("Test checking the property update trace events have appropriate causes");

	FilterEventNames = { PropertyUpdateEventName, ReceiveOpEventName, MergeComponentFieldUpdateEventName };
	WorkerDefinition = FWorkerDefinition::Client(1);
}

void APropertyUpdateEventTracingTest::FinishEventTraceTest()
{
	int EventsTested = 0;
	int EventsFailed = 0;
	for (const auto& Pair : TraceEvents)
	{
		const FString& SpanIdString = Pair.Key;
		const FName EventName = Pair.Value;

		if (EventName != PropertyUpdateEventName)
		{
			continue;
		}

		EventsTested++;

		if (!CheckEventTraceCause(SpanIdString, { ReceiveOpEventName, MergeComponentFieldUpdateEventName }))
		{
			EventsFailed++;
		}
	}

	bool bSuccess = EventsTested > 0 && EventsFailed == 0;
	AssertTrue(bSuccess,
			   FString::Printf(TEXT("Process property update events have the expected causes. Events Tested: %d, Events Failed: %d"),
							   EventsTested, EventsFailed));

	FinishStep();
}
