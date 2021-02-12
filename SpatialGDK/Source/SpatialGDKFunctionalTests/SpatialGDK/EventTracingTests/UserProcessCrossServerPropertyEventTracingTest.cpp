// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "UserProcessCrossServerPropertyEventTracingTest.h"

AUserProcessCrossServerPropertyEventTracingTest::AUserProcessCrossServerPropertyEventTracingTest()
{
	Author = "Danny Birch";
	Description = TEXT("Test checking user event traces can be caused by receive property update events for cross-server properties");

	FilterEventNames = { UserReceiveCrossServerPropertyEventName, ReceivePropertyUpdateEventName };
	WorkerDefinition = FWorkerDefinition::Client(1);
}

void AUserProcessCrossServerPropertyEventTracingTest::FinishEventTraceTest()
{
	CheckResult Test = CheckCauses(ReceivePropertyUpdateEventName, UserReceiveCrossServerPropertyEventName);

	bool bSuccess = Test.NumTested > 0 && Test.NumFailed == 0;
	AssertTrue(
		bSuccess,
		FString::Printf(
			TEXT("User events have been caused by the expected receive property update events. Events Tested: %d, Events Failed: %d"),
			Test.NumTested, Test.NumFailed));

	FinishStep();
}
