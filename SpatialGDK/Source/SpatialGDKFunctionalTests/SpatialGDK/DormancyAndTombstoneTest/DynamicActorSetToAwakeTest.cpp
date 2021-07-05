// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "DynamicActorSetToAwakeTest.h"
#include "DormancyTestActor.h"
#include "EngineUtils.h"

// This test checks that changing NetDormancy to DORM_Awake on the server on a dynamically created actor that sets it's NetDormancy to DORM_Initial in it's constructor
// will result in the NetDormancy of the actor on clients being set to DORM_Awake.

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
		FinishStep();
	});

	// Step 2 - Check dormancy and TestIntProp on server.
	AddStep(
		TEXT("ServerCheckDormancyAndRepProperty"), FWorkerDefinition::Server(1), nullptr, nullptr,
		[this](float DeltaTime) {
		CheckDormancyAndRepProperty(DORM_Awake, 0);
		FinishStep();
	},
		5.0f);

	// Step 3 - Check dormancy and TestIntProp on client.
	AddStep(
		TEXT("ClientCheckDormancyAndRepProperty"), FWorkerDefinition::AllClients, nullptr, nullptr,
		[this](float DeltaTime) {
		CheckDormancyAndRepProperty(DORM_Awake, 0);
		FinishStep();
	},
		5.0f);

	// Step 4 - Delete the test actor on the server.
	AddStep(TEXT("ServerDeleteActor"), FWorkerDefinition::Server(1), nullptr, [this]() {
		DestroyDormancyTestActors();
		FinishStep();
	});
}
