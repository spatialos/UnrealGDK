// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "InitiallyDormantMapActorTest.h"

#include "DormancyTestActor.h"
#include "TimerManager.h"

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
		RequireEqual_Int(CountActors<ADormancyTestActor>(GetWorld()), 1, TEXT("Number of TestDormancyActors in world"));
		RequireDormancyTestState(DORM_Initial, /*TestRepProperty*/ 0, /*ActorCount*/ 1);
		FinishStep();
	});

	// Step 2 - Check dormancy and TestIntProp on client.
	AddStep(
		TEXT("ClientRequireDormancyTestState"), FWorkerDefinition::AllClients, nullptr, nullptr,
		[this](float DeltaTime) {
			RequireEqual_Int(CountActors<ADormancyTestActor>(GetWorld()), 1, TEXT("Number of TestDormancyActors in world"));
			RequireDormancyTestState(DORM_Initial, /*TestRepProperty*/ 0, /*ActorCount*/ 1);
			FinishStep();
		},
		5.0f);

	// Step 3 - Modify TestIntProp on server.
	AddStep(TEXT("ServerModifyRepPropertyValue"), FWorkerDefinition::Server(1), nullptr, [this]() {
		for (ADormancyTestActor* DormancyTestActor : TActorRange<ADormancyTestActor>(GetWorld()))
		{
			DormancyTestActor->TestIntProp = 1;
		}
		FinishStep();
	});

	// Step 4 - Give chance for property to be replicated to clients
	AddStep(
		TEXT("ClientWaitForReplication"), FWorkerDefinition::AllClients, nullptr,
		[this]() {
			FTimerHandle DelayTimerHandle;
			FTimerManager& TimerManager = GetWorld()->GetTimerManager();
			TimerManager.SetTimer(
				DelayTimerHandle,
				[this]() {
					FinishStep();
				},
				0.5f, false);
		},
		nullptr, 5.0f);

	// Step 5 - Check dormancy and TestIntProp on client.
	AddStep(
		TEXT("ClientRequireDormancyTestState"), FWorkerDefinition::AllClients, nullptr, nullptr,
		[this](float DeltaTime) {
			RequireDormancyTestState(DORM_Initial, /*TestRepProperty*/ 0, /*ActorCount*/ 1);
			FinishStep();
		},
		5.0f);

	// Step 6 - Modify call FlushNetDormancy.
	AddStep(TEXT("ServerModifyRepPropertyValueAndFlush"), FWorkerDefinition::Server(1), nullptr, [this]() {
		for (ADormancyTestActor* DormancyTestActor : TActorRange<ADormancyTestActor>(GetWorld()))
		{
			DormancyTestActor->TestIntProp = 1;
			DormancyTestActor->FlushNetDormancy();
		}
		FinishStep();
	});

	// Step 7 - Check dormancy and TestIntProp on client.
	AddStep(
		TEXT("ClientRequireDormancyTestState"), FWorkerDefinition::AllClients, nullptr, nullptr,
		[this](float DeltaTime) {
			RequireDormancyTestState(DORM_DormantAll, /*TestRepProperty*/ 1, /*ActorCount*/ 1);
			FinishStep();
		},
		5.0f);

	// Step 8 - Delete the test actor on the server.
	AddStep(TEXT("ServerDeleteActor"), FWorkerDefinition::Server(1), nullptr, [this]() {
		DestroyDormancyTestActors();
		FinishStep();
	});

	// Step 9 - Observe the test actor has been deleted on the client.
	AddStep(
		TEXT("ClientCheckActorDestroyed"), FWorkerDefinition::AllClients, nullptr, nullptr,
		[this](float DeltaTime) {
			RequireEqual_Int(CountActors<ADormancyTestActor>(GetWorld()), 0, TEXT("Number of TestDormancyActors in world"));
			FinishStep();
		},
		5.0f);
}
