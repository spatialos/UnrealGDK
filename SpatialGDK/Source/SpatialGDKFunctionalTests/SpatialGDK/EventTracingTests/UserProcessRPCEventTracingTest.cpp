// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "UserProcessRPCEventTracingTest.h"

AUserProcessRPCEventTracingTest::AUserProcessRPCEventTracingTest()
{
	Author = "Matthew Sandford";
	Description = TEXT("Test checking user event traces can be caused by rpcs process events");

	FilterEventNames = { UserProcessRPCEventName, ApplyRPCEventName };
	WorkerDefinition = FWorkerDefinition::Client(1);
}

void AUserProcessRPCEventTracingTest::FinishEventTraceTest()
{
	CheckResult Test = CheckCauses(ApplyRPCEventName, UserProcessRPCEventName);

	bool bSuccess = Test.NumTested > 0 && Test.NumFailed == 0;
	AssertTrue(bSuccess,
			   FString::Printf(TEXT("User event have been caused by the expected process RPC events. Events Tested: %d, Events Failed: %d"),
							   Test.NumTested, Test.NumFailed));

	FinishStep();
}
