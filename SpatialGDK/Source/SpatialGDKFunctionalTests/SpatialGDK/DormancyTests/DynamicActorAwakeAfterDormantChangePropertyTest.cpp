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
		TEXT("ClientRequireDormancyAndRepProperty"), FWorkerDefinition::AllClients, nullptr, nullptr,
		[this](float DeltaTime) {
			RequireDormancyActorCount(1);
			RequireDormancyAndRepProperty(DORM_DormantAll, 0);
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
			FTimerManager& TimerManager = GetWorld()->GetTimerManager();
			TimerManager.SetTimer(
				DelayTimerHandle, []() {}, 0.5f, false);
		},
		[this](float DeltaTime) {
			FTimerManager& TimerManager = GetWorld()->GetTimerManager();
			bool bTimerActive = TimerManager.IsTimerActive(DelayTimerHandle);
			RequireEqual_Bool(bTimerActive, false, TEXT("Wait for replication"));
			FinishStep();
		},
		5.0f);

	// Step 5 - Client check TestIntProp is still 0
	AddStep(
		TEXT("ClientRequireDormancyAndRepProperty"), FWorkerDefinition::AllClients, nullptr, nullptr,
		[this](float DeltaTime) {
			RequireDormancyAndRepProperty(DORM_DormantAll, 0);
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
		TEXT("ClientRequireDormancyAndRepProperty"), FWorkerDefinition::AllClients, nullptr, nullptr,
		[this](float DeltaTime) {
			RequireDormancyAndRepProperty(DORM_Awake, 1);
			FinishStep();
		},
		5.0f);

	// Step 8 - Delete the test actor on the server.
	AddStep(TEXT("ServerDeleteActor"), FWorkerDefinition::Server(1), nullptr, [this]() {
		DestroyDormancyTestActors();
		FinishStep();
	});
}
