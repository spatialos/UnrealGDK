// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "MaxSamplingProbabilityFilterFalseTest.h"

AMaxSamplingProbabilityFilterFalseTest::AMaxSamplingProbabilityFilterFalseTest()
{
	Author = "Matthew Sandford";
	Description =
		TEXT("Test checking that spans are created with max sampling probability and not events are created with all filters to \"false\"");

	WorkerDefinition = FWorkerDefinition::Client(1);
}

void AMaxSamplingProbabilityFilterFalseTest::FinishEventTraceTest()
{
	AssertValue_Int(GetTraceSpanCount(TraceItemCountCategory::Client), EComparisonMethod::Greater_Than, 0,
					TEXT("Test produced client spans."));
	AssertValue_Int(GetTraceSpanCount(TraceItemCountCategory::Worker), EComparisonMethod::Greater_Than, 0,
					TEXT("Test produced worker spans."));
	AssertValue_Int(GetTraceSpanCount(TraceItemCountCategory::Runtime), EComparisonMethod::Greater_Than, 0,
					TEXT("Test produced runtime spans."));
	AssertValue_Int(GetTraceEventCount(TraceItemCountCategory::Client), EComparisonMethod::Equal_To, 0,
					TEXT("Test produced zero client events."));
	AssertValue_Int(GetTraceEventCount(TraceItemCountCategory::Worker), EComparisonMethod::Equal_To, 0,
					TEXT("Test produced zero worker events."));
	AssertValue_Int(GetTraceEventCount(TraceItemCountCategory::Runtime), EComparisonMethod::Equal_To, 0,
					TEXT("Test produced zero runtime events."));
}
