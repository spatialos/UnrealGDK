// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "CommandRequestEventTracingTest.h"
#include "Engine/World.h"

#include "SpatialFunctionalTestFlowController.h"

ACommandRequestEventTracingTest::ACommandRequestEventTracingTest()
{
	Author = "Matthew Sandford";
	Description = TEXT("Test checking the command request trace events have appropriate causes");

	FilterEventNames = { CommandRequestEventName, ReceiveOpEventName };
	WorkerDefinition = FWorkerDefinition::Client(1);
}

void ACommandRequestEventTracingTest::FinishEventTraceTest()
{
	Super::FinishEventTraceTest();

	int EventsTested = 0;
	int EventsFailed = 0;
	for (const auto& Pair : TraceEvents)
	{
		const FString SpanIdString = Pair.Key;
		const FName EventName = Pair.Value;

		if (EventName != CommandRequestEventName)
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
	AssertTrue(bSuccess, FString::Printf(TEXT(" Command request trace events have the expected causes. Events Tested: %d, Events Failed: %d"),
										 EventsTested, EventsFailed));

	FinishStep();
}
