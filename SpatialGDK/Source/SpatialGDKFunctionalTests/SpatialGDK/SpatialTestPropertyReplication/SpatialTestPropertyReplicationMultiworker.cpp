// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "SpatialTestPropertyReplicationMultiworker.h"
#include "Kismet/GameplayStatics.h"
#include "ReplicatedTestActor.h"

/**
 * "This tests that the data debug mode detects changes on non-auth servers.
 * This test contains 2 Servers and 2 Client workers.
 *
 * The flow is as follows:
 * - Setup:
 *  - The authorative server spawns one ReplicatedTestActor
 * - Test:
 *  - All workers check that they can see exactly 1 ReplicatedTestActor.
 *  - The authorative server changes the ReplicatedProperty of the ReplicatedTestActor from "0" to "99".
 *  - All workers check that the ReplicatedProperty is now set to "99".
 *  - The non-auth server changes the ReplicatedProperty of the ReplicatedTestActor to "55" which generates an expected error.
 *  - Auth server check that the ReplicatedProperty is still set to "99"
 * - Clean-up:
 *  - ReplicatedTestActor is destroyed using the RegisterAutoDestroyActor helper function.
 */

ASpatialTestPropertyReplicationMultiworker::ASpatialTestPropertyReplicationMultiworker()
	: Super()
{
	Author = "Victoria Bloom";
	Description = TEXT("This tests that the data debug mode detects changes on non-auth servers.");
}

void ASpatialTestPropertyReplicationMultiworker::PrepareTest()
{
	Super::PrepareTest();

	if (HasAuthority())
	{
		AddExpectedLogError(TEXT("Changed actor without authority with name ReplicatedTestActor_0 of type ReplicatedTestActor, property changed without authority was TestReplicatedProperty!"), 1, true);
	}

	AddStep(
		TEXT("The auth server spawns one ReplicatedTestActor"), FWorkerDefinition::Server(1), nullptr,
		[this]() {
			TestActor =
				GetWorld()->SpawnActor<AReplicatedTestActor>(FVector(0.0f, 0.0f, 50.0f), FRotator::ZeroRotator, FActorSpawnParameters());
			RegisterAutoDestroyActor(TestActor);

			FinishStep();
		},
		nullptr, 5.0f);

	AddStep(
		TEXT("All workers check that they can see exactly 1 ReplicatedTestActor"), FWorkerDefinition::AllWorkers, nullptr, nullptr,
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
		TEXT("The auth server changes the ReplicatedProperty of the ReplicatedTestActor from 0 to 99"), FWorkerDefinition::Server(1),
		[this]() -> bool {
			return IsValid(TestActor);
		},
		[this]() {
			TestActor->TestReplicatedProperty = 99;

			FinishStep();
		});

	AddStep(
		TEXT("All workers check that the ReplicatedProperty is now set to 99"), FWorkerDefinition::AllWorkers,
		[this]() -> bool {
			return IsValid(TestActor);
		},
		nullptr,
		[this](float DeltaTime) {
			RequireEqual_Int(TestActor->TestReplicatedProperty, 99, TEXT("The ReplicatedProperty should equal 99."));
			FinishStep();
		},
		5.0f);

	// This step generates an expected error "Changed actor without authority! ReplicatedTestActor_0 ReplicatedTestActor"
	AddStep(
		TEXT("The non-auth server changes the ReplicatedProperty of the ReplicatedTestActor to 55"), FWorkerDefinition::Server(2),
		[this]() -> bool {
			return IsValid(TestActor);
		},
		[this]() {
			TestActor->TestReplicatedProperty = 55;

			FinishStep();
		});

	AddStep(
		TEXT("Auth server check that the ReplicatedProperty is still set to 99"), FWorkerDefinition::Server(1),
		[this]() -> bool {
			return IsValid(TestActor);
		},
		nullptr,
		[this](float DeltaTime) {
			RequireEqual_Int(TestActor->TestReplicatedProperty, 99, TEXT("The ReplicatedProperty should equal 99."));
			FinishStep();
		},
		5.0f);
}
