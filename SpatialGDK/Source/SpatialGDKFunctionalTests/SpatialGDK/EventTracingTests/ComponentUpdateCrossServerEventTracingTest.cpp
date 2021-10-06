// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "ComponentUpdateCrossServerEventTracingTest.h"

AComponentUpdateCrossServerEventTracingTest::AComponentUpdateCrossServerEventTracingTest()
{
	Author = "Matthew Sandford + Ollie Balaam";
	Description = TEXT("Test checking the component update trace events have appropriate causes in CrossServer contexts");

	FilterEventNames = { ComponentUpdateEventName, ReceiveOpEventName };
	WorkerDefinition = FWorkerDefinition::Client(1);
}

void AComponentUpdateCrossServerEventTracingTest::FinishEventTraceTest()
{
	CheckResult Test = CheckCauses(ReceiveOpEventName, ComponentUpdateEventName);

	bool bSuccess = Test.NumTested > 0 && Test.NumFailed == 0;
	AssertTrue(bSuccess,
			   FString::Printf(TEXT("Component update trace events have the expected causes. Events Tested: %d, Events Failed: %d"),
							   Test.NumTested, Test.NumFailed));

	FinishStep();
}
