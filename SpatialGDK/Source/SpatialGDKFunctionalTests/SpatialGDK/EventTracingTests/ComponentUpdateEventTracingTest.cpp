// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "ComponentUpdateEventTracingTest.h"

#include "EventTracingTestConstants.h"

AComponentUpdateEventTracingTest::AComponentUpdateEventTracingTest()
{
	Author = "Matthew Sandford";
	Description = TEXT("Test checking the component update trace events have appropriate causes");

	FilterEventNames = { UEventTracingTestConstants::GetComponentUpdateEventName(), UEventTracingTestConstants::GetReceiveOpEventName(),
						 UEventTracingTestConstants::GetMergeComponentUpdateEventName() };
	WorkerDefinition = FWorkerDefinition::Client(1);
}

void AComponentUpdateEventTracingTest::FinishEventTraceTest()
{
	FName ComponentUpdateEventName = UEventTracingTestConstants::GetComponentUpdateEventName();
	FName ReceiveOpEventName = UEventTracingTestConstants::GetReceiveOpEventName();
	FName MergeComponentUpdateEventName = UEventTracingTestConstants::GetMergeComponentUpdateEventName();

	int EventsTested = 0;
	int EventsFailed = 0;
	for (const auto& Pair : TraceEvents)
	{
		const FString& SpanIdString = Pair.Key;
		const FName& EventName = Pair.Value;

		if (EventName != ComponentUpdateEventName)
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
			   FString::Printf(TEXT("Component update trace events have the expected causes. Events Tested: %d, Events Failed: %d"),
							   EventsTested, EventsFailed));

	FinishStep();
}
