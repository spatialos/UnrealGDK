// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "InitiallyDormantMapActorChangePropertyAndFlushTest.h"

#include "DormancyTestActor.h"

// This test checks that if a replicated property is modifed on an initially dormant actor but FlushNetDormancy is then called on that actor
// that the property is replicated.

AInitiallyDormantMapActorChangePropertyAndFlushTest::AInitiallyDormantMapActorChangePropertyAndFlushTest()
{
	Author = "Matthew Sandford";
}

void AInitiallyDormantMapActorChangePropertyAndFlushTest::PrepareTest()
{
	Super::PrepareTest();

	// Step 1 - Check actor exists and that it's NetDormancy and TestIntProp are correct on the server.
	AddStep(TEXT("ServerCheckkDormancyAndRepProperty"), FWorkerDefinition::Server(1), nullptr, [this]() {
		CheckDormancyActorCount(1);
		CheckDormancyAndRepProperty(DORM_Initial, 0);
		FinishStep();
	});

	// Step 2 - Check dormancy and TestIntProp on client.
	AddStep(
		TEXT("ClientCheckDormancyAndRepProperty"), FWorkerDefinition::AllClients, nullptr, nullptr,
		[this](float DeltaTime) {
			CheckDormancyActorCount(1);
			CheckDormancyAndRepProperty(DORM_Initial, 0);
			FinishStep();
		},
		5.0f);

	// Step 3 - Modify the TestIntProp and call FlushNetDormancy.
	AddStep(TEXT("ServerModifyRepPropertyValueAndFlush"), FWorkerDefinition::Server(1), nullptr, [this]() {
		for (TActorIterator<ADormancyTestActor> Iter(GetWorld()); Iter; ++Iter)
		{
			ADormancyTestActor* DormancyTestActor = *Iter;
			DormancyTestActor->TestIntProp = 1;
			DormancyTestActor->FlushNetDormancy();
		}
		FinishStep();
	});

	// Step 4 - Check dormancy and TestIntProp on client.
	AddStep(
		TEXT("ClientCheckDormancyAndRepProperty"), FWorkerDefinition::AllClients, nullptr, nullptr,
		[this](float DeltaTime) {
			CheckDormancyAndRepProperty(DORM_DormantAll, 1);
			FinishStep();
		},
		5.0f);
}
