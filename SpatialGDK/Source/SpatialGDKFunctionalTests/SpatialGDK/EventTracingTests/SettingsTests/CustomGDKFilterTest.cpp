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
	const int32 EventCount = GetTraceEventCount(TraceItemCountCategory::Client);
	const int32 EventTypeCount = GetTraceEventCount(TraceItemCountCategory::Client, { "user.send_rpc" });

	bool bTestSuccess = EventTypeCount > 0 && EventTypeCount == EventCount;
	AssertTrue(bTestSuccess, TEXT("Test produced client event of only one type."));
}
