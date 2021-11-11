// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "DynamicActorDormantAllChangePropertyTest.h"

#include "DormancyTestActor.h"
#include "EngineClasses/SpatialNetDriver.h"

// The test checks that modifying a replicated property on an actor that has NetDormancy DORM_DormantAll will result in the property not
// being replicated.

ADynamicActorDormantAllChangePropertyTest::ADynamicActorDormantAllChangePropertyTest()
{
	Author = "Matthew Sandford";
}

void ADynamicActorDormantAllChangePropertyTest::PrepareTest()
{
	Super::PrepareTest();

	// Step 1 - Spawn dormancy actor and check NetDormancy is DORM_DormantAll
	AddStep(TEXT("ServerSpawnDormancyActor"), FWorkerDefinition::Server(1), nullptr, [this]() {
		AActor* Actor = SpawnActor<ADormancyTestActor>();
		Actor->SetNetDormancy(DORM_DormantAll);
		FinishStep();
	});

	// Step 2 - Wait for actor channel to be ReadyForDormancy
	AddStep(
		TEXT("ServerWaitForActorChannelReadyForDormancy"), FWorkerDefinition::Server(1), nullptr, nullptr,
		[this](float DeltaTime) {
			bool bIsReadyForDormancy = true;

			UWorld* World = GetWorld();
			USpatialNetDriver* SpatialNetDriver = Cast<USpatialNetDriver>(World->GetNetDriver());
			if (SpatialNetDriver != nullptr)
			{
				int32 Count = 0;
				for (TActorIterator<ADormancyTestActor> Iter(GetWorld()); Iter; ++Iter)
				{
					const ADormancyTestActor* DormancyTestActor = *Iter;
					if (DormancyTestActor != nullptr)
					{
						const int64 EntityId = SpatialNetDriver->GetActorEntityId(*DormancyTestActor);
						bIsReadyForDormancy = SpatialNetDriver->IsDormantEntity(EntityId);
						Count++;
					}
				}
				RequireTrue(Count > 0, TEXT("Number of DormancyTestActors in world"));
			}

			RequireTrue(bIsReadyForDormancy, TEXT("Ready for dormancy"));
			FinishStep();
		},
		5.0f);

	// Step 3 - Client check NetDormancy is DORM_DormantAll
	AddStep(
		TEXT("ClientRequireDormancyTestState"), FWorkerDefinition::AllClients, nullptr, nullptr,
		[this](float DeltaTime) {
			RequireEqual_Int(CountActors<ADormancyTestActor>(GetWorld()), 1, TEXT("Number of TestDormancyActors in world"));
			RequireDormancyTestState(DORM_DormantAll, /*TestRepProperty*/ 0, /*ActorCount*/ 1);
			FinishStep();
		},
		5.0f);

	// Step 4 - Server set TestIntProp to 1
	AddStep(TEXT("ServerModifyNetDormancy"), FWorkerDefinition::Server(1), nullptr, [this]() {
		for (TActorIterator<ADormancyTestActor> Iter(GetWorld()); Iter; ++Iter)
		{
			ADormancyTestActor* DormancyTestActor = *Iter;
			DormancyTestActor->TestIntProp = 1;
		}
		FinishStep();
	});

	// Step 5 - Give chance for property to be replicated to clients
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

	// Step 6 - Client check TestIntProp is 0
	AddStep(
		TEXT("ClientRequireDormancyTestState"), FWorkerDefinition::AllClients, nullptr, nullptr,
		[this](float DeltaTime) {
			RequireDormancyTestState(DORM_DormantAll, /*TestRepProperty*/ 0, /*ActorCount*/ 1);
			FinishStep();
		},
		5.0f);
}
