// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "UserSendRPCEventTracingTest.h"

AUserSendRPCEventTracingTest::AUserSendRPCEventTracingTest()
{
	Author = "Matthew Sandford";
	Description = TEXT("Test checking user event traces can cause rpcs send events");

	FilterEventNames = { PushRPCEventName, UserSendRPCEventName };
	WorkerDefinition = FWorkerDefinition::Client(1);
}

void AUserSendRPCEventTracingTest::FinishEventTraceTest()
{
	TArray<FString> UserEventSpanIds;
	TArray<FString> SendRPCCauseSpanIds;

	for (const auto& Pair : TraceEvents)
	{
		const FString& SpanIdString = Pair.Key;
		const FName& EventName = Pair.Value;

		if (EventName == UserSendRPCEventName)
		{
			UserEventSpanIds.Add(SpanIdString);
		}
		else if (EventName == PushRPCEventName)
		{
			TArray<FString>* Causes = TraceSpans.Find(SpanIdString);
			if (Causes != nullptr)
			{
				SendRPCCauseSpanIds += *Causes;
			}
		}
	}

	int EventsTested = UserEventSpanIds.Num();

	for (const FString& CauseSpanId : SendRPCCauseSpanIds)
	{
		UserEventSpanIds.Remove(CauseSpanId);
	}

	int EventsFailed = UserEventSpanIds.Num();
	bool bSuccess = EventsTested > 0 && EventsFailed == 0;
	AssertTrue(bSuccess, FString::Printf(TEXT("User event have caused the expected send RPC events. Events Tested: %d, Events Failed: %d"),
										 EventsTested, EventsFailed));

	FinishStep();
}
