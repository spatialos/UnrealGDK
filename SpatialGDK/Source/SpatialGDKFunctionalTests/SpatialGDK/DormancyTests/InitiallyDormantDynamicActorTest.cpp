// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "InitiallyDormantDynamicActorTest.h"

// This test checks that a spawned actor that sets its NetDormancy to DORM_Initial in its constructor will be DORM_Initial on the server
// but DORM_DormantAll on clients.

AInitiallyDormantDynamicActorTest::AInitiallyDormantDynamicActorTest()
{
	Author = "Matthew Sandford";
}

void AInitiallyDormantDynamicActorTest::PrepareTest()
{
	Super::PrepareTest();

	// Step 1 - Spawn dormancy actor
	AddStep(TEXT("ServerSpawnDormancyActor"), FWorkerDefinition::Server(1), nullptr, [this]() {
		AActor* Actor = CreateDormancyTestActor();
		RegisterAutoDestroyActor(Actor);
		FinishStep();
	});

	// Step 2 - Server check NetDormancy is DORM_Initial
	AddStep(
		TEXT("ClientRequireDormancyTestState"), FWorkerDefinition::Server(1), nullptr, nullptr,
		[this](float DeltaTime) {
			RequireDormancyActorCount(1);
			RequireDormancyTestState(DORM_Initial, /*TestRepProperty*/ 0, /*ActorCount*/ 1);
			FinishStep();
		},
		5.0f);

	// Step 3 - Client check NetDormancy is DORM_DormantAll
	AddStep(
		TEXT("ClientRequireDormancyTestState"), FWorkerDefinition::AllClients, nullptr, nullptr,
		[this](float DeltaTime) {
			RequireDormancyTestState(DORM_DormantAll, /*TestRepProperty*/ 0, /*ActorCount*/ 1);
			FinishStep();
		},
		5.0f);
}
