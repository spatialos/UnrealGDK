// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "ProcessCrossServerRPCEventTracingTest.h"

AProcessCrossServerRPCEventTracingTest::AProcessCrossServerRPCEventTracingTest()
{
	Author = "Danny Birch";
	Description = TEXT("Test checking the process RPC trace events have appropriate causes for cross-server RPCs");

	FilterEventNames = { ReceiveCrossServerRPCName, ApplyCrossServerRPCName };
	WorkerDefinition = FWorkerDefinition::Server(1);
}

void AProcessCrossServerRPCEventTracingTest::FinishEventTraceTest()
{
	CheckResult Test = CheckCauses(ReceiveCrossServerRPCName, ApplyCrossServerRPCName);

	bool bSuccess = Test.NumTested > 0 && Test.NumFailed == 0;
	AssertTrue(bSuccess, FString::Printf(TEXT("Process RPC trace events have the expected causes. Events Tested: %d, Events Failed: %d"),
										 Test.NumTested, Test.NumFailed));

	FinishStep();
}
