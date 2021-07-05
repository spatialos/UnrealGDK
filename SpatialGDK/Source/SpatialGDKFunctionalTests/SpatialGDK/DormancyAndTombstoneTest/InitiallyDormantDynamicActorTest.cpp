// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "InitiallyDormantDynamicActorTest.h"
#include "DormancyTestActor.h"
#include "EngineUtils.h"

 // This test checks that a spawned actor that set's it's NetDormancy to DORM_Initial in it's constructor will be DORM_Initial on the server but DORM_DormantAll on clients.

AInitiallyDormantDynamicActorTest::AInitiallyDormantDynamicActorTest()
{
	Author = "Matthew Sandford";
}

void AInitiallyDormantDynamicActorTest::PrepareTest()
{
	Super::PrepareTest();

	// Step 1 - Spawn dormancy actor and set dormancy to DORM_Awake.
	AddStep(TEXT("ServerSpawnDormancyActor"), FWorkerDefinition::Server(1), nullptr, [this]() {
		AActor* Actor = CreateDormancyTestActor();
		FinishStep();
	});

	// Step 2 - Check dormancy and TestIntProp on server.
	AddStep(
		TEXT("ClientCheckDormancyAndRepProperty"), FWorkerDefinition::Server(1), nullptr, nullptr,
		[this](float DeltaTime) {
		CheckDormancyAndRepProperty(DORM_Initial, 0);
		FinishStep();
	},
		5.0f);

	// Step 3 - Check dormancy and TestIntProp on client.
	AddStep(
		TEXT("ClientCheckDormancyAndRepProperty"), FWorkerDefinition::AllClients, nullptr, nullptr,
		[this](float DeltaTime) {
		CheckDormancyAndRepProperty(DORM_DormantAll, 0);
		FinishStep();
	},
		5.0f);

	// Step 4 - Delete the test actor on the server.
	AddStep(TEXT("ServerDeleteActor"), FWorkerDefinition::Server(1), nullptr, [this]() {
		DestroyDormancyTestActors();
		FinishStep();
	});
}
