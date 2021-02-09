// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "ProcessCrossServerRPCEventTracingTest.h"

AProcessCrossServerRPCEventTracingTest::AProcessCrossServerRPCEventTracingTest()
{
	Author = "Danny Birch";
	Description = TEXT("Test checking the process RPC trace events have appropriate causes for cross-server RPCs");

	FilterEventNames = { ReceiveCrossServerRPCName, ApplyCrossServerRPCName };
	WorkerDefinition = FWorkerDefinition::Server(1);
}

void AProcessCrossServerRPCEventTracingTest::FinishEventTraceTest()
{
	int EventsTested = 0;
	int EventsFailed = 0;
	for (const auto& Pair : TraceEvents)
	{
		const FString& SpanIdString = Pair.Key;
		const FName& EventName = Pair.Value;

		if (EventName == ReceiveCrossServerRPCName)
		{
			continue;
		}

		EventsTested++;

		if (EventName == ApplyCrossServerRPCName)
		{
			if (!CheckEventTraceCause(SpanIdString, { ReceiveCrossServerRPCName }, true))
			{
				EventsFailed++;
			}
		}
// 		else // EventName == ReceiveRPCEventName
// 		{
// 			if (!CheckEventTraceCause(SpanIdString, { ReceiveOpEventName }, true))
// 			{
// 				EventsFailed++;
// 			}
// 		}
	}

	bool bSuccess = EventsTested > 0 && EventsFailed == 0;
	AssertTrue(bSuccess, FString::Printf(TEXT("Process RPC trace events have the expected causes. Events Tested: %d, Events Failed: %d"),
										 EventsTested, EventsFailed));

	FinishStep();
}
