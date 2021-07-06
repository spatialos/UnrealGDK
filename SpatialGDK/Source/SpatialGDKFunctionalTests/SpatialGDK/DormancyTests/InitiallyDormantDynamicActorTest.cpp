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
		FinishStep();
	});

	// Step 2 - Server check NetDormancy is DORM_Initial
	AddStep(
		TEXT("ClientCheckDormancyAndRepProperty"), FWorkerDefinition::Server(1), nullptr, nullptr,
		[this](float DeltaTime) {
			CheckDormancyActorCount(1);
			CheckDormancyAndRepProperty(DORM_Initial, 0);
			FinishStep();
		},
		5.0f);

	// Step 3 - Client check NetDormancy is DORM_DormantAll
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
