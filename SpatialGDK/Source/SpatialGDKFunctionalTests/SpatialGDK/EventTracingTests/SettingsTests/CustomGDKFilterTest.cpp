// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "CustomGDKFilterTest.h"

ACustomGDKFilterTest::ACustomGDKFilterTest()
{
	Author = "Matthew Sandford";
	Description = TEXT("Test checking that custom GDK filters are applied correctly.");

	WorkerDefinition = FWorkerDefinition::Client(1);
}

void ACustomGDKFilterTest::FinishEventTraceTest()
{
	const int32 EventCount = GetTraceEventCount(TraceSource::Client);
	const int32 EventTypeCount = GetTraceEventCount(TraceSource::Client, { "user.send_rpc" });

	AssertValue_Int(EventTypeCount, EComparisonMethod::Greater_Than, 0, TEXT("Test produced client event of only specific type."));
	AssertValue_Int(EventTypeCount, EComparisonMethod::Equal_To, EventCount, TEXT("Test produced client event of only one type."));
}
