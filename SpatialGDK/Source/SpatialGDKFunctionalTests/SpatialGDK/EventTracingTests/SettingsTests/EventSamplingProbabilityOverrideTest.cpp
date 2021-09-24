// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "EventSamplingProbabilityOverrideTest.h"

#include "SpatialGDKFunctionalTests/SpatialGDK/ConfigurationAssets/EventSamplingProbabilityOverrideSettings.h"

AEventSamplingProbabilityOverrideTest::AEventSamplingProbabilityOverrideTest()
{
	Author = "Matthew Sandford";
	Description = TEXT("Test checking that spans are created with zero sampling probability but an event sampling probability overriden");

	WorkerDefinition = FWorkerDefinition::Client(1);
}

void AEventSamplingProbabilityOverrideTest::FinishEventTraceTest()
{
	bool bAllSpansForOverridenEventLogged = true;

	ForEachTraceSource([&bAllSpansForOverridenEventLogged](const TraceItemsData& SourceTraceItems) {
		for (const auto& Pair : SourceTraceItems.SpanEvents)
		{
			const FString& SpanIdString = Pair.Key;
			const FName& EventName = Pair.Value;

			if (EventName == UEventSamplingProbabilityOverrideSettings::OverridenEventType)
			{
				bAllSpansForOverridenEventLogged &= SourceTraceItems.Spans.Contains(SpanIdString);
			}
		}
		return false;
	});

	AssertTrue(bAllSpansForOverridenEventLogged, TEXT("Test produced spans for overriden event."));
}
