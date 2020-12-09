// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "UserSendPropertyEventTracingTest.h"

AUserSendPropertyEventTracingTest::AUserSendPropertyEventTracingTest()
{
	Author = "Matthew Sandford";
	Description = TEXT("Test checking user event traces can cause send property update events");

	FilterEventNames = { UserSendPropertyEventName, UserSendComponentPropertyEventName, PropertyChangedEventName };
	WorkerDefinition = FWorkerDefinition::Client(1);
}

void AUserSendPropertyEventTracingTest::FinishEventTraceTest()
{
	TArray<FString> UserEventSpanIds;
	TArray<FString> SendPropertyUpdatesCauseSpanIds;

	for (const auto& Pair : TraceEvents)
	{
		const FString& SpanIdString = Pair.Key;
		const FName& EventName = Pair.Value;

		if (EventName == UserSendPropertyEventName || EventName == UserSendComponentPropertyEventName)
		{
			UserEventSpanIds.Add(SpanIdString);
		}
		else if (EventName == PropertyChangedEventName)
		{
			TArray<FString>* Causes = TraceSpans.Find(SpanIdString);
			if (Causes != nullptr)
			{
				SendPropertyUpdatesCauseSpanIds += *Causes;
			}
		}
	}

	int EventsTested = UserEventSpanIds.Num();

	for (const FString& CauseSpanId : SendPropertyUpdatesCauseSpanIds)
	{
		UserEventSpanIds.Remove(CauseSpanId);
	}

	int EventsFailed = UserEventSpanIds.Num();
	bool bSuccess = EventsTested > 0 && EventsFailed == 0;
	AssertTrue(bSuccess, FString::Printf(
							 TEXT("User event have caused the expected send property update events. Events Tested: %d, Events Failed: %d"),
							 EventsTested, EventsFailed));

	FinishStep();
}
