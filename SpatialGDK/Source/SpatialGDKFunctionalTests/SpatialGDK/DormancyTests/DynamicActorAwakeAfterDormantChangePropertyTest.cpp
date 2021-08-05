// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "DynamicActorAwakeAfterDormantChangePropertyTest.h"

#include "DormancyTestActor.h"

// This test checks whether changes on a dormant actor are replicated when the actor becomes awake.

ADynamicActorAwakeAfterDormantChangePropertyTest::ADynamicActorAwakeAfterDormantChangePropertyTest()
{
	Author = "Matthew Sandford";
}

void ADynamicActorAwakeAfterDormantChangePropertyTest::PrepareTest()
{
	Super::PrepareTest();

	// Step 1 - Spawn dormancy actor and change NetDormancy to DORM_DormantAll
	AddStep(TEXT("ServerSpawnDormancyActor"), FWorkerDefinition::Server(1), nullptr, [this]() {
		AActor* Actor = CreateDormancyTestActor();
		Actor->SetNetDormancy(DORM_DormantAll);
		FinishStep();
	});

	// Step 2 - Client check NetDormancy is DORM_DormantAll
	AddStep(
		TEXT("ClientRequireDormancyTestState"), FWorkerDefinition::AllClients, nullptr, nullptr,
		[this](float DeltaTime) {
			RequireDormancyActorCount(1);
			RequireDormancyTestState(DORM_DormantAll, /*TestRepProperty*/ 0, /*ActorCount*/ 1);
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

	// Step 5 - Client check TestIntProp is still 0
	AddStep(
		TEXT("ClientRequireDormancyTestState"), FWorkerDefinition::AllClients, nullptr, nullptr,
		[this](float DeltaTime) {
			RequireDormancyTestState(DORM_DormantAll, /*TestRepProperty*/ 0, /*ActorCount*/ 1);
			FinishStep();
		},
		5.0f);

	// Step 6 - Server set NetDormancy to DORM_Awake
	AddStep(TEXT("ServerModifyNetDormancy"), FWorkerDefinition::Server(1), nullptr, [this]() {
		for (TActorIterator<ADormancyTestActor> Iter(GetWorld()); Iter; ++Iter)
		{
			ADormancyTestActor* DormancyTestActor = *Iter;
			DormancyTestActor->SetNetDormancy(DORM_Awake);
		}
		FinishStep();
	});

	// Step 7 - Client check NetDormancy is DORM_Awake and TestIntProp is 1
	AddStep(
		TEXT("ClientRequireDormancyTestState"), FWorkerDefinition::AllClients, nullptr, nullptr,
		[this](float DeltaTime) {
			RequireDormancyTestState(DORM_Awake, /*TestRepProperty*/ 1, /*ActorCount*/ 1);
			FinishStep();
		},
		5.0f);
}
