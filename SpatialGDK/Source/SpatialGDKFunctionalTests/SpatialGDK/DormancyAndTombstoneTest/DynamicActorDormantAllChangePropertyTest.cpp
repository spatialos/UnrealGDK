// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "DynamicActorDormantAllChangeProperty.h"
#include "DormancyTestActor.h"
#include "GameFramework/PlayerController.h"
#include "EngineUtils.h"

// The test checks that modifying a replicated property on an actor that has NetDormancy DORM_DormantAll will result in the property not being replicated.

ADynamicActorDormantAllChangeProperty::ADynamicActorDormantAllChangeProperty()
{
	Author = "Matthew Sandford";
}

void ADynamicActorDormantAllChangeProperty::PrepareTest()
{
	Super::PrepareTest();

	// Step 1 - Spawn dormancy actor and check NetDormancy is DORM_Initial
	AddStep(TEXT("ServerSpawnDormancyActor"), FWorkerDefinition::Server(1), nullptr, [this]() {
		AActor* Actor = CreateDormancyTestActor();
		Actor->SetNetDormancy(DORM_DormantAll);
		FinishStep();
	});

	// Step 2 -  Client Check NetDormancy is DORM_Initial
	AddStep(
		TEXT("ClientCheckDormancyAndRepProperty"), FWorkerDefinition::AllClients, nullptr, nullptr,
		[this](float DeltaTime) {
		CheckDormancyAndRepProperty(DORM_DormantAll, 0);
		FinishStep();
	},
		5.0f);

	// Step 3 - Server set TestIntProp to 1
	AddStep(TEXT("ServerModifyNetDormancy"), FWorkerDefinition::Server(1), nullptr, [this]() {
		for (TActorIterator<ADormancyTestActor> Iter(GetWorld()); Iter; ++Iter)
		{
			ADormancyTestActor* DormancyTestActor = *Iter;
			DormancyTestActor->TestIntProp = 1;
		}
		FinishStep();
	});

	// Step 4 - Client Check TestIntProp is 0
	AddStep(
		TEXT("ClientCheckDormancyAndRepProperty"), FWorkerDefinition::AllClients, nullptr, nullptr,
		[this](float DeltaTime) {
		CheckDormancyAndRepProperty(DORM_DormantAll, 0);
		FinishStep();
	},
	5.0f);

	// Step 5 - Delete the test actor on the server.
	AddStep(TEXT("ServerDeleteActor"), FWorkerDefinition::Server(1), nullptr, [this]() {
		DestroyDormancyTestActors();
		FinishStep();
	});

}
