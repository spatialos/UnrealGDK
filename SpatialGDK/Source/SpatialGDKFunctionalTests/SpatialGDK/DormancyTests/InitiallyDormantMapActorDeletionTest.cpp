// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "InitiallyDormantMapActorDeletionTest.h"

#include "InitiallyDormantMapActorTest.h"

#include "DormancyTestActor.h"

// Tests that an initially dormant startup actor, which remains dormant, can be deleted on the client.

AInitiallyDormantMapActorDeletionTest::AInitiallyDormantMapActorDeletionTest()
{
	Author = "Joshua Huburn";
}

void AInitiallyDormantMapActorDeletionTest::PrepareTest()
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

	// Step 3 - Delete the test actor on the server.
	AddStep(TEXT("ServerDeleteActor"), FWorkerDefinition::Server(1), nullptr, [this]() {
		DestroyDormancyTestActors();
		FinishStep();
	});

	// Step 4 - Observe the test actor has been deleted on the client.
	AddStep(
		TEXT("ClientCheckActorDestroyed"), FWorkerDefinition::AllClients, nullptr, nullptr,
		[this](float DeltaTime) {
			RequireEqual_Int(CountActors<ADormancyTestActor>(GetWorld()), 0, TEXT("Number of TestDormancyActors in world"));
			FinishStep();
		},
		5.0f);
}
