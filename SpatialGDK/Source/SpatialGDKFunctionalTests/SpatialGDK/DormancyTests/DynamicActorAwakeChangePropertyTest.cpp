// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "DynamicActorAwakeChangePropertyTest.h"

#include "DormancyTestActor.h"

// This test checks that changing the value of a replicated property on an actor whose NetDormancy is DORM_Awake will result in the property
// being replicated.

ADynamicActorAwakeChangePropertyTest::ADynamicActorAwakeChangePropertyTest()
{
	Author = "Matthew Sandford";
}

void ADynamicActorAwakeChangePropertyTest::PrepareTest()
{
	Super::PrepareTest();

	// Step 1 - Spawn dormancy actor and set NetDormancy to DORM_Awake
	AddStep(TEXT("ServerSpawnDormancyActor"), FWorkerDefinition::Server(1), nullptr, [this]() {
		AActor* Actor = CreateDormancyTestActor();
		Actor->SetNetDormancy(DORM_Awake);
		RegisterAutoDestroyActor(Actor);
		FinishStep();
	});

	// Step 2 - Client check NetDormancy is DORM_Awake
	AddStep(
		TEXT("ClientRequireDormancyTestState"), FWorkerDefinition::AllClients, nullptr, nullptr,
		[this](float DeltaTime) {
			RequireDormancyActorCount(1);
			RequireDormancyTestState(DORM_Awake, /*TestRepProperty*/ 0, /*ActorCount*/ 1);
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

	// Step 4 - Client check TestIntProp is 1
	AddStep(
		TEXT("ClientRequireDormancyTestState"), FWorkerDefinition::AllClients, nullptr, nullptr,
		[this](float DeltaTime) {
			RequireDormancyTestState(DORM_Awake, /*TestRepProperty*/ 1, /*ActorCount*/ 1);
			FinishStep();
		},
		5.0f);
}
