// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "ZeroSamplingProbabilityFilterTrueTest.h"

AZeroSamplingProbabilityFilterTrueTest::AZeroSamplingProbabilityFilterTrueTest()
{
	Author = "Matthew Sandford";
	Description =
		TEXT("Test checking that spans are not created with zero sampling probability and events are created with all filters to \"true\"");

	WorkerDefinition = FWorkerDefinition::Client(1);
}

void AZeroSamplingProbabilityFilterTrueTest::FinishEventTraceTest()
{
	// Runtime creates some root spans itself which will be unaffected by changing the sampling probability.
	// We want to disregard these spans when running this test and check not other spans are produced.
	// The following logic handles this by look for root spanId and ignore those that start in the runtime.

	const TraceItemsData* RuntimeTraceItems = TraceItems.Find(TraceSource::Runtime);
	auto CountSpansWithNonRuntimeRoots = [this, RuntimeTraceItems](const TraceItemsData* SourceTraceItems, int32& OutCount)
	{
		OutCount = 0;
		if (SourceTraceItems != nullptr && RuntimeTraceItems != nullptr)
		{
			TMap<FString, TArray<FString>> SpansCopy = SourceTraceItems->Spans;
			for (const auto& Pair : SourceTraceItems->Spans)
			{
				const FString RootSpanId = FindRootSpanId(Pair.Key);
				if (RuntimeTraceItems->Spans.Contains(RootSpanId))
				{
					SpansCopy.Remove(Pair.Key);
				}
			}
			OutCount = SpansCopy.Num();
		}
	};

	int32 ClientSpans = 0;
	int32 WorkerSpans = 0;
	int32 RuntimeSpans = 0;

	CountSpansWithNonRuntimeRoots(TraceItems.Find(TraceSource::Client), ClientSpans);
	CountSpansWithNonRuntimeRoots(TraceItems.Find(TraceSource::Worker), WorkerSpans);
	CountSpansWithNonRuntimeRoots(TraceItems.Find(TraceSource::Runtime), RuntimeSpans);

	AssertValue_Int(ClientSpans, EComparisonMethod::Equal_To, 0, TEXT("Test produced zero client spans."));
	AssertValue_Int(WorkerSpans, EComparisonMethod::Equal_To, 0, TEXT("Test produced zero worker spans."));
	AssertValue_Int(RuntimeSpans, EComparisonMethod::Equal_To, 0, TEXT("Test produced zero runtime spans."));
	AssertValue_Int(GetTraceEventCount(TraceSource::Client), EComparisonMethod::Greater_Than, 0,
					TEXT("Test produced client events."));
	AssertValue_Int(GetTraceEventCount(TraceSource::Worker), EComparisonMethod::Greater_Than, 0,
					TEXT("Test produced worker events."));
	AssertValue_Int(GetTraceEventCount(TraceSource::Runtime), EComparisonMethod::Greater_Than, 0,
					TEXT("Test produced runtime events."));
}
