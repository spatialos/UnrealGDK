// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "SpatialTestExample.h"
#include "Kismet/GameplayStatics.h"
#include "TestExampleActor.h"

/**
 * This test tests an Actor's replication.
 * This test contains 1 Server and 2 Client workers.
 *
 * The flow is as follows:
 * - Setup:
 *	- The Server spawns a replicated Actor.
 * - Test:
 *	- The Clients check if the replicated Actor is visible for them.
 *  - The Server changes a replicated property of the Actor.
 *  - The Clients check that the property was correctly replicated.
 * - Clean-up:
 *	- The spawned replicated Actor is destroyed.
 */

ASpatialTestExample::ASpatialTestExample()
	: Super()
{
	Author = "Ollie and Ben";
	Description = TEXT("Example Test");
}

void ASpatialTestExample::PrepareTest()
{
	Super::PrepareTest();

	AddStep(TEXT("Example Test Server spawns the TestExampleActor"), FWorkerDefinition::Server(1), nullptr, [this]() {
		TestActor =
			GetWorld()->SpawnActor<ATestExampleActor>(FVector(100.0f, 100.0f, 80.0f), FRotator::ZeroRotator, FActorSpawnParameters());
		RegisterAutoDestroyActor(TestActor);

		FinishStep();
	});

	AddStep(
		TEXT("Example Test Clients check Actor replication"), FWorkerDefinition::AllClients, nullptr, nullptr,
		[this](float DeltaTime) {
			TArray<AActor*> FoundTestExampleActors;
			UGameplayStatics::GetAllActorsOfClass(GetWorld(), ATestExampleActor::StaticClass(), FoundTestExampleActors);

			if (FoundTestExampleActors.Num() == 1)
			{
				TestActor = Cast<ATestExampleActor>(FoundTestExampleActors[0]);
				if (IsValid(TestActor))
				{
					FinishStep();
				}
			}
		},
		5.0f);

	AddStep(
		TEXT("Example Test Server Modifies replicated property"), FWorkerDefinition::Server(1),
		[this]() -> bool {
			return IsValid(TestActor);
		},
		[this]() {
			TestActor->ExampleReplicatedProperty = 99;

			FinishStep();
		});

	AddStep(
		TEXT("Example Test Clients check modified property"), FWorkerDefinition::AllClients,
		[this]() -> bool {
			return IsValid(TestActor);
		},
		nullptr,
		[this](float DeltaTime) {
			if (TestActor->ExampleReplicatedProperty == 99)
			{
				FinishStep();
			}
		},
		5.0f);
}
