// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "SpatialTestPropertyReplication.h"
#include "Kismet/GameplayStatics.h"
#include "ReplicatedTestActor.h"

/**
 * This tests that an Actor can replicate a property across the network during play.
 * This test contains 1 Server and 2 Client workers.
 *
 * The flow is as follows:
 * - Setup:
 *  - The Server spawns one ReplicatedTestActor.
 * - Test:
 *  - Both Clients check that they can see exactly 1 ReplicatedTestActor.
 *  - The Server changes the ReplicatedProperty of the ReplicatedTestActor from "0" to "99".
 *  - Both Clients check that the ReplicatedProperty is now set to "99".
 * - Clean-up:
 *  - ReplicatedTestActor is destroyed using the RegisterAutoDestroyActor helper function.
 */

ASpatialTestPropertyReplication::ASpatialTestPropertyReplication()
	: Super()
{
	Author = "Ollie Balaam";
	Description = TEXT("This tests that an Actor can replicate a property across the network during play.");
}

void ASpatialTestPropertyReplication::PrepareTest()
{
	Super::PrepareTest();

	AddStep(TEXT("The Server spawns one ReplicatedTestActor"), FWorkerDefinition::Server(1), nullptr, [this]() {
		TestActor =
			GetWorld()->SpawnActor<AReplicatedTestActor>(FVector(100.0f, 100.0f, 80.0f), FRotator::ZeroRotator, FActorSpawnParameters());
		RegisterAutoDestroyActor(TestActor);

		FinishStep();
	});

	AddStep(
		TEXT("Both Clients check that they can see exactly 1 ReplicatedTestActor"), FWorkerDefinition::AllClients, nullptr, nullptr,
		[this](float DeltaTime) {
			TArray<AActor*> FoundReplicatedTestActors;
			UGameplayStatics::GetAllActorsOfClass(GetWorld(), AReplicatedTestActor::StaticClass(), FoundReplicatedTestActors);

			RequireEqual_Int(FoundReplicatedTestActors.Num(), 1,
							 TEXT("The number of AReplicatedTestActor found in the world should equal 1."));

			if (FoundReplicatedTestActors.Num() == 1)
			{
				TestActor = Cast<AReplicatedTestActor>(FoundReplicatedTestActors[0]);
				RequireTrue(IsValid(TestActor), TEXT("The TestActor must be Valid (usable : non-null and not pending kill)."));
				FinishStep();
			}
		},
		5.0f);

	AddStep(
		TEXT("The Server changes the ReplicatedProperty of the ReplicatedTestActor from 0 to 99"), FWorkerDefinition::Server(1),
		[this]() -> bool {
			return IsValid(TestActor);
		},
		[this]() {
			TestActor->TestReplicatedProperty = 99;

			FinishStep();
		});

	AddStep(
		TEXT("Both Clients check that the ReplicatedProperty is now set to 99"), FWorkerDefinition::AllClients,
		[this]() -> bool {
			return IsValid(TestActor);
		},
		nullptr,
		[this](float DeltaTime) {
			RequireEqual_Int(TestActor->TestReplicatedProperty, 99,
							 TEXT("The ReplicatedProperty should equal 99."));
			FinishStep();
		},
		5.0f);
}
