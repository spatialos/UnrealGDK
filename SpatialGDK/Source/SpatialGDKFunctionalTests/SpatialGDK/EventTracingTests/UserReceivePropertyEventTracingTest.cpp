// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "UserReceivePropertyEventTracingTest.h"

AUserReceivePropertyEventTracingTest::AUserReceivePropertyEventTracingTest()
{
	Author = "Matthew Sandford";
	Description = TEXT("Test checking user event traces can be caused by receive property update events");

	FilterEventNames = { UserReceivePropertyEventName, UserReceiveComponentPropertyEventName, ReceivePropertyUpdateEventName };
	WorkerDefinition = FWorkerDefinition::Client(1);
}

void AUserReceivePropertyEventTracingTest::FinishEventTraceTest()
{
	{
		CheckResult Test = CheckCauses(ReceivePropertyUpdateEventName, UserReceivePropertyEventName);
		bool bSuccess = Test.NumTested > 0 && Test.NumFailed == 0;
		AssertTrue(bSuccess, FString::Printf(TEXT("User events (receive->property) have been caused by the expected receive property "
												  "update events. Events Tested: %d, Events Failed: %d"),
											 Test.NumTested, Test.NumFailed));
	}
	{
		CheckResult Test = CheckCauses(ReceivePropertyUpdateEventName, UserReceiveComponentPropertyEventName);
		bool bSuccess = Test.NumTested > 0 && Test.NumFailed == 0;
		AssertTrue(bSuccess, FString::Printf(TEXT("User events (receive->component property) have been caused by the expected receive "
												  "property update events. Events Tested: %d, Events Failed: %d"),
											 Test.NumTested, Test.NumFailed));
	}

	FinishStep();
}
