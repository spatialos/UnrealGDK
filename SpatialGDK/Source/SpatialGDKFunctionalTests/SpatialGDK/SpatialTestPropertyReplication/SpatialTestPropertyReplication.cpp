// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "SpatialTestPropertyReplication.h"
#include "Kismet/GameplayStatics.h"
#include "ReplicatedTestActor.h"

/**
 * This is an example test. It's cited in https://brevi.link/how-to-test-unrealgdk.
 * It tests that an Actor can replicate a property across the network during play.
 * This test contains 1 Server and 3 Client workers.
 *
 * The flow is as follows:
 * - Setup:
 *  - The Server spawns one ReplicatedTestActor.
 * - Test:
 *  - All Clients check that they can see exactly 1 ReplicatedTestActor.
 *  - The Server changes the ReplicatedProperty of the ReplicatedTestActor from "0" to "99".
 *  - All  Clients check that the ReplicatedProperty is now set to "99".
 * - Clean-up:
 *  - ReplicatedTestActor is destroyed using the RegisterAutoDestroyActor helper function.
 */

ASpatialTestPropertyReplication::ASpatialTestPropertyReplication()
	: Super()
{
	Author = "Ollie Balaam (oliverbalaam@improbable.io)";
	Description = TEXT("This tests that an Actor can replicate a property across the network during play. It is an example test intended to teach the basics of the UnrealGDK Functional Test Framework. It's accompanied by this document: https://brevi.link/how-to-test-unrealgdk");
}

void ASpatialTestPropertyReplication::PrepareTest()
{
	Super::PrepareTest();

AddStep(
		TEXT("Check PIE override settings"), FWorkerDefinition::AllServers, nullptr,
		[this]() {
			int32 ExpectedNumberOfClients = 3;
			int32 RequiredNumberOfClients = GetNumRequiredClients();
		RequireEqual_Int(RequiredNumberOfClients, ExpectedNumberOfClients, TEXT("Expected a certain number of clients to be required."));
		int32 ActualNumberOfClients = GetNumberOfClientWorkers();
		RequireEqual_Int(ActualNumberOfClients , ExpectedNumberOfClients, TEXT("Expected a certain number of clients to actually connect."));
		FinishStep();
		},
		nullptr, 5.0f);

	AddStep(TEXT("The Server spawns one ReplicatedTestActor"), FWorkerDefinition::Server(1), nullptr, [this]() {
		TestActor =
			GetWorld()->SpawnActor<AReplicatedTestActor>(FVector(0.0f, 0.0f, 50.0f), FRotator::ZeroRotator, FActorSpawnParameters());
		RegisterAutoDestroyActor(TestActor);

		FinishStep();
	}, nullptr, 5.0f);

	AddStep(
		TEXT("All Clients check that they can see exactly 1 ReplicatedTestActor"), FWorkerDefinition::AllClients, nullptr, nullptr,
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
		TEXT("All Clients check that the ReplicatedProperty is now set to 99"), FWorkerDefinition::AllClients,
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
