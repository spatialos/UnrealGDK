// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "UserProcessRPCEventTracingTest.h"

AUserProcessRPCEventTracingTest::AUserProcessRPCEventTracingTest()
{
	Author = "Matthew Sandford";
	Description = TEXT("Test checking user event traces can be caused by rpcs process events");

	FilterEventNames = { UserProcessRPCEventName, ProcessRPCEventName };
	WorkerDefinition = FWorkerDefinition::Client(1);
}

void AUserProcessRPCEventTracingTest::FinishEventTraceTest()
{
	int EventsTested = 0;
	int EventsFailed = 0;
	for (const auto& Pair : TraceEvents)
	{
		const FString& SpanIdString = Pair.Key;
		const FName EventName = Pair.Value;

		if (EventName != UserProcessRPCEventName)
		{
			continue;
		}

		EventsTested++;

		if (!CheckEventTraceCause(SpanIdString, { ProcessRPCEventName }, 1))
		{
			EventsFailed++;
		}
	}

	bool bSuccess = EventsTested > 0 && EventsFailed == 0;
	AssertTrue(bSuccess,
			   FString::Printf(TEXT("User event have been caused by the expected process RPC events. Events Tested: %d, Events Failed: %d"),
							   EventsTested, EventsFailed));

	FinishStep();
}
