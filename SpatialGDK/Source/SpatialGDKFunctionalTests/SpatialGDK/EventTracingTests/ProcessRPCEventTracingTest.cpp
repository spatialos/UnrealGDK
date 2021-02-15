// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "ProcessRPCEventTracingTest.h"

AProcessRPCEventTracingTest::AProcessRPCEventTracingTest()
{
	Author = "Matthew Sandford";
	Description = TEXT("Test checking the process RPC trace events have appropriate causes");

	FilterEventNames = { ReceiveRPCEventName, ReceiveOpEventName, ApplyRPCEventName };
	WorkerDefinition = FWorkerDefinition::Server(1);
}

void AProcessRPCEventTracingTest::FinishEventTraceTest()
{
	{
		CheckResult Test = CheckCauses(ReceiveRPCEventName, ApplyRPCEventName);

		bool bSuccess = Test.NumTested > 0 && Test.NumFailed == 0;
		AssertTrue(bSuccess,
				   FString::Printf(
					   TEXT("Process RPC (receive->apply) trace events have the expected causes. Events Tested: %d, Events Failed: %d"),
					   Test.NumTested, Test.NumFailed));
	}
	{
		CheckResult Test = CheckCauses(ReceiveOpEventName, ReceiveRPCEventName);

		bool bSuccess = Test.NumTested > 0 && Test.NumFailed == 0;
		AssertTrue(
			bSuccess,
			FString::Printf(TEXT("Process RPC (op->receive) trace events have the expected causes. Events Tested: %d, Events Failed: %d"),
							Test.NumTested, Test.NumFailed));
	}

	FinishStep();
}
