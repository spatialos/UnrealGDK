// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "PropertyUpdateEventTracingTest.h"

#include "EventTracingTestConstants.h"

APropertyUpdateEventTracingTest::APropertyUpdateEventTracingTest()
{
	Author = "Matthew Sandford";
	Description = TEXT("Test checking the property update trace events have appropriate causes");

	FilterEventNames = { UEventTracingTestConstants::GetReceivePropertyUpdateEventName(),
						 UEventTracingTestConstants::GetReceiveOpEventName(),
						 UEventTracingTestConstants::GetMergeComponentUpdateEventName() };
	WorkerDefinition = FWorkerDefinition::Client(1);
}

void APropertyUpdateEventTracingTest::FinishEventTraceTest()
{
	FName ReceivePropertyUpdateEventName = UEventTracingTestConstants::GetReceivePropertyUpdateEventName();
	FName ReceiveOpEventName = UEventTracingTestConstants::GetReceiveOpEventName();
	FName MergeComponentUpdateEventName = UEventTracingTestConstants::GetMergeComponentUpdateEventName();

	int EventsTested = 0;
	int EventsFailed = 0;
	for (const auto& Pair : TraceEvents)
	{
		const FString& SpanIdString = Pair.Key;
		const FName& EventName = Pair.Value;

		if (EventName != ReceivePropertyUpdateEventName)
		{
			continue;
		}

		EventsTested++;

		if (!CheckEventTraceCause(SpanIdString, { ReceiveOpEventName, MergeComponentUpdateEventName }))
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
