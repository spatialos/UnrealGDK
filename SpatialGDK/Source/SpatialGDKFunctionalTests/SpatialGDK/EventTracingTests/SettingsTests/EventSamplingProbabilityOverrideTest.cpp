// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "EventSamplingProbabilityOverrideTest.h"

#include "SpatialGDKFunctionalTests/SpatialGDK/Config/EventSamplingProbabilityOverrideSettings.h"

AEventSamplingProbabilityOverrideTest::AEventSamplingProbabilityOverrideTest()
{
	Author = "Matthew Sandford";
	Description = TEXT("Test checking that spans are created with zero sampling probability but an event sampling probability overriden");

	WorkerDefinition = FWorkerDefinition::Client(1);
}

void AEventSamplingProbabilityOverrideTest::FinishEventTraceTest()
{
	bool bAllSpansForOverridenEventLogged = true;
	for (const auto& Pair : SpanEvents)
	{
		const FString& SpanIdString = Pair.Key;
		const FName& EventName = Pair.Value;

		if (EventName == UEventSamplingProbabilityOverrideSettings::OverridenEventType)
		{
			bAllSpansForOverridenEventLogged &= TraceSpans.Contains(SpanIdString);
		}
	}

	AssertTrue(bAllSpansForOverridenEventLogged, TEXT("Test produced spans for overriden event."));
}
