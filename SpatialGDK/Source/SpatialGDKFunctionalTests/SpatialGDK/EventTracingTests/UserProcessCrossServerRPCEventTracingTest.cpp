// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "UserProcessCrossServerRPCEventTracingTest.h"

AUserProcessCrossServerRPCEventTracingTest::AUserProcessCrossServerRPCEventTracingTest()
{
	Author = "Danny Birch";
	Description = TEXT("Test checking user event traces can be caused by rpcs process events for cross-server RPCs");

	FilterEventNames = { UserReceiveCrossServerRPCEventName, ApplyCrossServerRPCName };
	WorkerDefinition = FWorkerDefinition::Client(1);
}

void AUserProcessCrossServerRPCEventTracingTest::FinishEventTraceTest()
{
	int EventsTested = 0;
	int EventsFailed = 0;
	for (const auto& Pair : TraceEvents)
	{
		const FString& SpanIdString = Pair.Key;
		const FName& EventName = Pair.Value;

		if (EventName != UserReceiveCrossServerRPCEventName)
		{
			continue;
		}

		EventsTested++;

		if (!CheckEventTraceCause(SpanIdString, { ApplyCrossServerRPCName }, 1))
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
