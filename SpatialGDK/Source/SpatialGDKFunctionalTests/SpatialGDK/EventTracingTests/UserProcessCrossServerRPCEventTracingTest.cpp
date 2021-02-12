// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "UserProcessCrossServerRPCEventTracingTest.h"

AUserProcessCrossServerRPCEventTracingTest::AUserProcessCrossServerRPCEventTracingTest()
{
	Author = "Danny Birch";
	Description = TEXT("Test checking user event traces can be caused by rpcs process events for cross-server RPCs");

	FilterEventNames = { UserReceiveCrossServerRPCEventName, ApplyCrossServerRPCName };
	WorkerDefinition = FWorkerDefinition::Client(1);
}

void AUserProcessCrossServerRPCEventTracingTest::FinishEventTraceTest()
{
	CheckResult Test = CheckCauses(ApplyCrossServerRPCName, UserReceiveCrossServerRPCEventName);

	bool bSuccess = Test.NumTested > 0 && Test.NumFailed == 0;
	AssertTrue(bSuccess,
			   FString::Printf(TEXT("User event have been caused by the expected process RPC events. Events Tested: %d, Events Failed: %d"),
							   Test.NumTested, Test.NumFailed));

	FinishStep();
}
