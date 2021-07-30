// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "SpatialTestPropertyReplicationSubobject.h"
#include "Kismet/GameplayStatics.h"
#include "ReplicatedTestActorSubobject.h"

/**
 * "This tests that the data debug mode detects property changes on non-auth servers for subobjects.
 * This test contains 2 Servers and 2 Client workers.
 *
 * The flow is as follows:
 * - Setup:
 *  - The authorative server spawns one ReplicatedTestActorSubobject (in it's OnAuthorityGained it will spawn a subobject of type
 * ReplicatedTestActor )
 * - Test:
 *  - All workers check that they can see exactly 1 ReplicatedTestActor and 1 ReplicatedTestActorSubobject.
 *  - The authorative server changes the replicated property of the replicated subobject.
 *  - All workers check that the replicated property of the replicated subobject has changed.
 *  - The non-auth server changes the the replicated property of the replicated subobject which generates an expected error.
 *  - The client changes the replicated properties which generates an expected errors.
 *  - Auth server check that the replicated property of the replicated subobject has not changed.
 * - Clean-up:
 *  - ReplicatedTestActor is destroyed using the RegisterAutoDestroyActor helper function.
 */

ASpatialTestPropertyReplicationSubobject::ASpatialTestPropertyReplicationSubobject()
	: Super()
{
	Author = "Victoria Bloom";
	Description = TEXT("This tests that the data debug mode detects property changes on non-auth servers for replicated subobjects.");
}

void ASpatialTestPropertyReplicationSubobject::PrepareTest()
{
	Super::PrepareTest();

	if (HasAuthority())
	{
		// Subobject property change 
		AddExpectedLogError(TEXT("ReplicatedTestActor, property changed without authority was TestReplicatedProperty!"), 2);
	}

	AddStep(
		TEXT("The auth server spawns one ReplicatedTestActorSubobject"), FWorkerDefinition::Server(1), nullptr,
		[this]() {
			TestActor = GetWorld()->SpawnActor<AReplicatedTestActorSubobject>(FVector(0.0f, 0.0f, 50.0f), FRotator::ZeroRotator,
																			  FActorSpawnParameters());
			RegisterAutoDestroyActor(TestActor);

			FinishStep();
		},
		nullptr, 5.0f);

	AddStep(
		TEXT("All workers check that they can see exactly 1 ReplicatedTestActorSubobject and 1 ReplicatedTestActorBasic"),
		FWorkerDefinition::AllWorkers, nullptr, nullptr,
		[this](float DeltaTime) {
			TArray<AActor*> FoundReplicatedTestActors;
			UGameplayStatics::GetAllActorsOfClass(GetWorld(), AReplicatedTestActorSubobject::StaticClass(), FoundReplicatedTestActors);
			TArray<AActor*> FoundReplicatedSubTestActors;
			UGameplayStatics::GetAllActorsOfClass(GetWorld(), AReplicatedTestActor::StaticClass(), FoundReplicatedSubTestActors);

			RequireEqual_Int(FoundReplicatedTestActors.Num(), 1,
							 TEXT("The number of AReplicatedTestActorSubobject found in the world should equal 1."));

			RequireEqual_Int(FoundReplicatedSubTestActors.Num(), 1,
							 TEXT("The number of AReplicatedTestActorBasic found in the world should equal 1."));

			if (FoundReplicatedTestActors.Num() == 1 && FoundReplicatedSubTestActors.Num() == 1)
			{
				TestActor = Cast<AReplicatedTestActorSubobject>(FoundReplicatedTestActors[0]);
				AReplicatedTestActor* TestSubActor = Cast<AReplicatedTestActor>(FoundReplicatedSubTestActors[0]);
				RequireTrue(IsValid(TestActor), TEXT("The TestActor must be Valid (usable : non-null and not pending kill)."));
				RequireTrue(IsValid(TestSubActor), TEXT("The TestSubActor must be Valid (usable : non-null and not pending kill)."));
				FinishStep();
			}
		},
		5.0f);

	AddStep(
		TEXT("The auth server changes the replicated properties"), FWorkerDefinition::Server(1),
		[this]() -> bool {
			return IsValid(TestActor);
		},
		[this]() {
			TestActor->ReplicatedSubActor->TestReplicatedProperty = 999;

			FinishStep();
		});

	AddStep(
		TEXT("All workers check that the replicated properties have changed"), FWorkerDefinition::AllWorkers,
		[this]() -> bool {
			return IsValid(TestActor);
		},
		nullptr,
		[this](float DeltaTime) {
			RequireEqual_Int(TestActor->ReplicatedSubActor->TestReplicatedProperty, 999,
							 TEXT("The ReplicatedIntProperty on subobject should equal 999."));
			FinishStep();
		},
		5.0f);

	// This step generates an expected error for the replicated subobject 
	AddStep(
		TEXT("The non-auth server changes the replicated properties"), FWorkerDefinition::Server(2),
		[this]() -> bool {
			return IsValid(TestActor);
		},
		[this]() {
			// Subobject property
			TestActor->ReplicatedSubActor->TestReplicatedProperty = 555;

			FinishStep();
		});

	// This step generates an expected errors for the replicated subobject
	AddStep(
		TEXT("The client changes the replicated properties"), FWorkerDefinition::Client(1),
		[this]() -> bool {
		return IsValid(TestActor);
		},
		[this]() {
		// Subobject property
		TestActor->ReplicatedSubActor->TestReplicatedProperty = 222;

		FinishStep();
		});

	AddStep(
		TEXT("Auth server check that the replicated properties still have their original values"), FWorkerDefinition::Server(1),
		[this]() -> bool {
			return IsValid(TestActor);
		},
		nullptr,
		[this](float DeltaTime) {
			RequireEqual_Int(TestActor->ReplicatedSubActor->TestReplicatedProperty, 999,
							 TEXT("The ReplicatedIntProperty on subobject should equal 999."));
			FinishStep();
		},
		5.0f);
}
