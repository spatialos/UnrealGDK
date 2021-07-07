// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "InitiallyDormantMapActorTest.h"

#include "DormancyTestActor.h"

// This test tests checks that replicated properties changed on an initially dormant actor do not get replicated.

AInitiallyDormantMapActorTest::AInitiallyDormantMapActorTest()
{
	Author = "Matthew Sandford";
}

void AInitiallyDormantMapActorTest::PrepareTest()
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

	// Step 3 - Modify TestIntProp on server.
	AddStep(TEXT("ServerModifyRepPropertyValue"), FWorkerDefinition::Server(1), nullptr, [this]() {
		for (TActorIterator<ADormancyTestActor> Iter(GetWorld()); Iter; ++Iter)
		{
			ADormancyTestActor* DormancyTestActor = *Iter;
			DormancyTestActor->TestIntProp = 1;
		}
		FinishStep();
	});

	// Step 4 - Check dormancy and TestIntProp on client.
	AddStep(
		TEXT("ClientCheckDormancyAndRepProperty"), FWorkerDefinition::AllClients, nullptr, nullptr,
		[this](float DeltaTime) {
			CheckDormancyAndRepProperty(DORM_Initial, 0);
			FinishStep();
		},
		5.0f);

	// Step 5 - Modify call FlushNetDormancy.
	AddStep(TEXT("ServerModifyRepPropertyValueAndFlush"), FWorkerDefinition::Server(1), nullptr, [this]() {
		for (TActorIterator<ADormancyTestActor> Iter(GetWorld()); Iter; ++Iter)
		{
			ADormancyTestActor* DormancyTestActor = *Iter;
			DormancyTestActor->TestIntProp = 1;
			DormancyTestActor->FlushNetDormancy();
		}
		FinishStep();
	});

	// Step 6 - Check dormancy and TestIntProp on client.
	AddStep(
		TEXT("ClientCheckDormancyAndRepProperty"), FWorkerDefinition::AllClients, nullptr, nullptr,
		[this](float DeltaTime) {
		CheckDormancyAndRepProperty(DORM_DormantAll, 1);
		FinishStep();
	},
		5.0f);

	// Step 7 - Delete the test actor on the server.
	AddStep(TEXT("ServerDeleteActor"), FWorkerDefinition::Server(1), nullptr, [this]() {
		DestroyDormancyTestActors();
		FinishStep();
	});

	// Step 8 - Observe the test actor has been deleted on the client.
	AddStep(
		TEXT("ClientCheckActorDestroyed"), FWorkerDefinition::AllClients, nullptr, nullptr,
		[this](float DeltaTime) {
		CheckDormancyActorCount(0);
		FinishStep();
	},
		5.0f);
}
