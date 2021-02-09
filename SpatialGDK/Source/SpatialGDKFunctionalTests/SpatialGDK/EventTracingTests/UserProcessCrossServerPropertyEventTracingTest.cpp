// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "UserProcessCrossServerPropertyEventTracingTest.h"

AUserProcessCrossServerPropertyEventTracingTest::AUserProcessCrossServerPropertyEventTracingTest()
{
	Author = "Danny Birch";
	Description = TEXT("Test checking user event traces can be caused by receive property update events for cross-server properties");

	FilterEventNames = { UserReceiveCrossServerPropertyEventName, ReceivePropertyUpdateEventName };
	WorkerDefinition = FWorkerDefinition::Client(1);
}

void AUserProcessCrossServerPropertyEventTracingTest::FinishEventTraceTest()
{
	int EventsTested = 0;
	int EventsFailed = 0;
	for (const auto& Pair : TraceEvents)
	{
		const FString& SpanIdString = Pair.Key;
		const FName& EventName = Pair.Value;

		if (EventName != UserReceiveCrossServerPropertyEventName)
		{
			continue;
		}

		EventsTested++;

		if (!CheckEventTraceCause(SpanIdString, { ReceivePropertyUpdateEventName }, 1))
		{
			EventsFailed++;
		}
	}

	bool bSuccess = EventsTested > 0 && EventsFailed == 0;
	AssertTrue(
		bSuccess,
		FString::Printf(
			TEXT("User events have been caused by the expected receive property update events. Events Tested: %d, Events Failed: %d"),
			EventsTested, EventsFailed));

	FinishStep();
}
