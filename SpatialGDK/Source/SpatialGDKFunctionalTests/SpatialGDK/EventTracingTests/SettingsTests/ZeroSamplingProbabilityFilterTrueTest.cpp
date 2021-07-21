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
	AssertValue_Int(GetTraceSpanCount(TraceItemCountCategory::Client), EComparisonMethod::Equal_To, 0,
					TEXT("Test produced zero client spans."));
	AssertValue_Int(GetTraceSpanCount(TraceItemCountCategory::Worker), EComparisonMethod::Equal_To, 0,
					TEXT("Test produced zero worker spans."));
	AssertValue_Int(GetTraceSpanCount(TraceItemCountCategory::Runtime), EComparisonMethod::Equal_To, 0,
					TEXT("Test produced zero runtime spans."));
	AssertValue_Int(GetTraceEventCount(TraceItemCountCategory::Client), EComparisonMethod::Greater_Than, 0,
					TEXT("Test produced client events."));
	AssertValue_Int(GetTraceEventCount(TraceItemCountCategory::Worker), EComparisonMethod::Greater_Than, 0,
					TEXT("Test produced worker events."));
	AssertValue_Int(GetTraceEventCount(TraceItemCountCategory::Runtime), EComparisonMethod::Greater_Than, 0,
					TEXT("Test produced runtime events."));
}
