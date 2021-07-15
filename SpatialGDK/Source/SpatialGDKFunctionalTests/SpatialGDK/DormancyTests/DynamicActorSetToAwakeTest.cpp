// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "DynamicActorSetToAwakeTest.h"

// This test checks that changing NetDormancy to DORM_Awake on the server on a dynamically created actor that sets it's NetDormancy to
// DORM_Initial in it's constructor will result in the NetDormancy of the actor on clients being set to DORM_Awake.

ADynamicActorSetToAwakeTest::ADynamicActorSetToAwakeTest()
{
	Author = "Matthew Sandford";
}

void ADynamicActorSetToAwakeTest::PrepareTest()
{
	Super::PrepareTest();

	// Step 1 - Spawn dormancy actor and set dormancy to DORM_Awake.
	AddStep(TEXT("ServerSpawnDormancyActor"), FWorkerDefinition::Server(1), nullptr, [this]() {
		AActor* Actor = CreateDormancyTestActor();
		Actor->SetNetDormancy(DORM_Awake);
		RegisterAutoDestroyActor(Actor);
		FinishStep();
	});

	// Step 2 - Server check NetDormancy is DORM_Awake
	AddStep(
		TEXT("ServerRequireDormancyTestState"), FWorkerDefinition::Server(1), nullptr, nullptr,
		[this](float DeltaTime) {
			RequireDormancyActorCount(1);
			RequireDormancyTestState(DORM_Awake, /*TestRepProperty*/ 0, /*ActorCount*/ 1);
			FinishStep();
		},
		5.0f);

	// Step 3 - Client check NetDormancy is DORM_Awake
	AddStep(
		TEXT("ClientRequireDormancyTestState"), FWorkerDefinition::AllClients, nullptr, nullptr,
		[this](float DeltaTime) {
			RequireDormancyTestState(DORM_Awake, /*TestRepProperty*/ 0, /*ActorCount*/ 1);
			FinishStep();
		},
		5.0f);
}
