// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "SpatialTestTearOff.h"

#include "ReplicatedTearOffActor.h"
#include "SpatialGDKFunctionalTests/SpatialGDK/TestActors/ReplicatedTestActorBase.h"

#include "Kismet/GameplayStatics.h"

/**
 * This test automates the QA flow that checks if Actors can be prevented from replicating with bTearOff.
 * NOTE: Due to adding wait steps before checking certain parts of the test, this test runs slow, in roughly 30 seconds. This can be improved by modifying the WorkerWaitForTimeStepDefinition
 *       to wait for a smaller amount of time. 
 *
 * The test includes 1 Server and 2 Client workers.
 * The flow is as follows:
 * - Setup:
 *  - A ReplicatedTearOffActor must be placed in the map that is running the test, as a start-up actor.
 *  - All workers set a reference to the start-up actor.
 * - Test:
 *  - The server modifies a replicated property of the start-up Actor.
 *  - The clients check that the modification was ignored.
 *  - The server spawns a ReplicatedTearOffActor.
 *  - The clients check that the spawned ReplicatedTearOffActor was not replicated at all.
 *  - The server spawns a ReplicatedTestActorBase, which is not torn off by deafult.
 *  - The clients check that the spawned ReplicatedTestActorBase is correctly replicated.
 *  - The server calls TearOff on the ReplicatedTestActorBase and changes its location.
 *  - The clients check that the location update is ignored.
 * - Clean-up:
 *  - The 2 spawned Actors are deleted.
 */

ASpatialTestTearOff::ASpatialTestTearOff()
	: Super()
{
	Author = "Andrei";
	Description = TEXT("Test TearOff prevents Actors from replicating");

	ReplicatedTestActorBaseInitialLocation = FVector(150.0f, 150.0f, 80.0f);
	ReplicatedTestActorBaseMoveLocationBeforeTearOff = FVector(-150.0f, -150.0f, 80.0f);
	ReplicatedTestActorBaseMoveLocationAfterTearOff = FVector(30.0f, -20.0f, 80.0f);

}

void ASpatialTestTearOff::BeginPlay()
{
	Super::BeginPlay();

	// Step definition for a 5 second timer
	FSpatialFunctionalTestStepDefinition WorkerWaitForTimeStepDefinition;
	WorkerWaitForTimeStepDefinition.bIsNativeDefinition = true;
	WorkerWaitForTimeStepDefinition.TimeLimit = 15.0f;
	WorkerWaitForTimeStepDefinition.NativeStartEvent.BindLambda([this]()
	{
		TimerHelper = 0.0f;
	});
	WorkerWaitForTimeStepDefinition.NativeTickEvent.BindLambda([this](float DeltaTime)
	{
		TimerHelper += DeltaTime;
		if (TimerHelper > 5.0f)
		{
			FinishStep();
		}
	});

	// All workers set a reference to the ReplicatedTearOffActor placed in the map.
	AddStep(TEXT("SpatialTestTearOffUniversalStartupActorReferenceSetup"), FWorkerDefinition::AllWorkers, nullptr, [this]()
		{
			TArray<AActor*> ReplicatedTearOffActors;
			UGameplayStatics::GetAllActorsOfClass(GetWorld(), AReplicatedTearOffActor::StaticClass(), ReplicatedTearOffActors);

			checkf(ReplicatedTearOffActors.Num() == 1, TEXT("Exactly one ReplicatedTearOffActor is expected at this stage"));

			StartupTearOffActor = Cast<AReplicatedTearOffActor>(ReplicatedTearOffActors[0]);
			if (IsValid(StartupTearOffActor))
			{
				FinishStep();
			}
		});

	// This is required to make the test pass in Native since calling TearOff does not immediately stop the Actor from replicating.
	AddStepFromDefinition(WorkerWaitForTimeStepDefinition, FWorkerDefinition::AllWorkers);

	// The server changes the value of the replicated property of the ReplicatedTearOffActor that was initially placed in the map.
	AddStep(TEXT("SpatialTestTearOffServerChangeStartupActorDefaultProperty"), FWorkerDefinition::Server(1), nullptr, [this]()
		{
			if (StartupTearOffActor)
			{
				StartupTearOffActor->TestInteger = 1;
				FinishStep();
			}
		});

	// Allow for potential replication to propagate before checking that the property update was ignored.
	AddStepFromDefinition(WorkerWaitForTimeStepDefinition, FWorkerDefinition::AllWorkers);

	// Clients check that the server update to the replicated property was ignored, as expected.
	AddStep(TEXT("SpatialTestTearOffClientsCheckStartupActorPropertyValue"), FWorkerDefinition::AllClients, nullptr, [this]()
		{
			if (StartupTearOffActor && StartupTearOffActor->TestInteger == 0)
			{
				FinishStep();
			}
		});

	// The server dynamically spawns a ReplicatedTearOffActor.
	AddStep(TEXT("SpatialTestTearOffServerSpawnReplicatedTearOffActor"), FWorkerDefinition::Server(1), nullptr, [this]()
		{
			AReplicatedTearOffActor* SpawnedTearOffActor = GetWorld()->SpawnActor<AReplicatedTearOffActor>(FVector(50.0f, 50.0f, 80.0f), FRotator::ZeroRotator, FActorSpawnParameters());
			RegisterAutoDestroyActor(SpawnedTearOffActor);

			// Make sure the Actor was correctly spawned
			if (IsValid(SpawnedTearOffActor))
			{
				FinishStep();
			}
		});

	// Allow for potential replication to propagate before checking that the SpawnedTearOffActor was not received.
	AddStepFromDefinition(WorkerWaitForTimeStepDefinition, FWorkerDefinition::AllWorkers);

	// Clients check that they have not received the newly spawned Actor
	AddStep(TEXT("SpatialTestTearOffUniversalSpawnedActorReferenceSetup"), FWorkerDefinition::AllClients, nullptr, [this]()
		{
			TArray<AActor*> ReplicatedTearOffActors;
			UGameplayStatics::GetAllActorsOfClass(GetWorld(), AReplicatedTearOffActor::StaticClass(), ReplicatedTearOffActors);

			// Only the StartupTearOffActor is expected to exist in the world at this point.
			if (ReplicatedTearOffActors.Num() == 1)
			{
				FinishStep();
			}
		});

	// Spawn a replicated Actor that does not call TearOff on BeginPlay().
	AddStep(TEXT("SpatialTestTearOffServerSpawnReplicatedActor"), FWorkerDefinition::Server(1), nullptr, [this]()
		{
			SpawnedReplicatedActorBase = GetWorld()->SpawnActor<AReplicatedTestActorBase>(ReplicatedTestActorBaseInitialLocation, FRotator::ZeroRotator, FActorSpawnParameters());
			RegisterAutoDestroyActor(SpawnedReplicatedActorBase);
			FinishStep();
		});

	// Make sure the spawned actor is correctly replicated and set a reference to it.
	AddStep(TEXT("SpatialTestTearOffClientsSpawnedActorReferenceSetup"), FWorkerDefinition::AllClients, nullptr, nullptr, [this](float DeltaTime)
		{
			TArray<AActor*> ReplicatedActors;
			UGameplayStatics::GetAllActorsOfClass(GetWorld(), AReplicatedTestActorBase::StaticClass(), ReplicatedActors);

			// Since ReplicatedTearOffActor inherits from AReplicatedTestActorBase, 2 AReplicatedTestActorBase are expected at this point:
			// The start-up ReplicatedTearOfActor, and the newly spawned AReplicatedTestActorBase object.
			if (ReplicatedActors.Num() == 2)
			{
				// Ensure the reference to the correct Actor is set.
				if (ReplicatedActors[0]->IsA(AReplicatedTearOffActor::StaticClass()))
				{
					SpawnedReplicatedActorBase = Cast<AReplicatedTestActorBase>(ReplicatedActors[1]);
				}
				else
				{
					SpawnedReplicatedActorBase = Cast<AReplicatedTestActorBase>(ReplicatedActors[0]);
				}

				if (IsValid(SpawnedReplicatedActorBase) && SpawnedReplicatedActorBase->GetActorLocation().Equals(ReplicatedTestActorBaseInitialLocation, 1.0f))
				{
					FinishStep();
				}
			}
		}, 5.0f);

	// Move the spawned actor to make sure its movement is being replicated correctly.
	AddStep(TEXT("SpatialTestTearOffSeverMoveSpawnedActor"), FWorkerDefinition::Server(1), nullptr, [this]()
		{
			SpawnedReplicatedActorBase->SetActorLocation(ReplicatedTestActorBaseMoveLocationBeforeTearOff);

			FinishStep();
		});

	// Clients check that the spawned actor correctly replicates movement before calling TearOff.
	AddStep(TEXT("SpatialTestTearOffClientsCheckMovement"), FWorkerDefinition::AllClients, nullptr, nullptr, [this](float DeltaTime)
		{
			if (SpawnedReplicatedActorBase->GetActorLocation().Equals(ReplicatedTestActorBaseMoveLocationBeforeTearOff, 1.0f))
			{
				FinishStep();
			}
		}, 5.0f);

	// Tear off the spawned replicated ReplicatedTestActorBase.
	AddStep(TEXT("SpatialTestTearOffServerSetTearOffAndMoveActor"), FWorkerDefinition::Server(1), nullptr, [this]()
		{
			SpawnedReplicatedActorBase->TearOff();

			FinishStep();
		});

	// Wait for the TearOff to propagate.
	AddStep(TEXT("SpatialTestTearOffAllWorkersCheckTearOff"), FWorkerDefinition::AllWorkers, nullptr, nullptr, [this](float DeltaTime)
		{
			if (SpawnedReplicatedActorBase->GetTearOff())
			{
				FinishStep();
			}
		}, 5.0f);

	// Now that the Actor is TornOff, move it from the server.
	AddStep(TEXT("SpatialTestTearOfFServerMoveTornOffActor"), FWorkerDefinition::Server(1), nullptr, [this]()
		{
			if (SpawnedReplicatedActorBase->SetActorLocation(ReplicatedTestActorBaseMoveLocationAfterTearOff))
			{
				FinishStep();
			}
		});

	// Allow for potential replication to propagate before checking that the location update for SpawnedReplicatedActorBase was ignored.
	AddStepFromDefinition(WorkerWaitForTimeStepDefinition, FWorkerDefinition::AllWorkers);

	// Clients check that the Location update was ignored.
	AddStep(TEXT("SpatialTestTearOffClientsLocationUpdateWasIgnored"), FWorkerDefinition::AllClients, nullptr, [this]()
		{
			if (SpawnedReplicatedActorBase->GetActorLocation().Equals(ReplicatedTestActorBaseMoveLocationBeforeTearOff, 1.0f))
			{
				FinishStep();
			}
		}, nullptr, 2.0f);
}
