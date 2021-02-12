// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "PropertyUpdateEventTracingTest.h"

APropertyUpdateEventTracingTest::APropertyUpdateEventTracingTest()
{
	Author = "Matthew Sandford";
	Description = TEXT("Test checking the property update trace events have appropriate causes");

	FilterEventNames = { ReceivePropertyUpdateEventName, ReceiveOpEventName };
	WorkerDefinition = FWorkerDefinition::Client(1);
}

void APropertyUpdateEventTracingTest::FinishEventTraceTest()
{
	CheckResult Test = CheckCauses(ReceiveOpEventName, ReceivePropertyUpdateEventName);

	bool bSuccess = Test.NumTested > 0 && Test.NumFailed == 0;
	AssertTrue(bSuccess,
			   FString::Printf(TEXT("Process property update events have the expected causes. Events Tested: %d, Events Failed: %d"),
							   Test.NumTested, Test.NumFailed));

	FinishStep();
}
