// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "VisibilityTest.h"
#include "ReplicatedVisibilityTestActor.h"
#include "SpatialFunctionalTestFlowController.h"
#include "SpatialGDKFunctionalTests/SpatialGDK/TestActors/TestMovementCharacter.h"

#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"

/**
 * This test tests if a bHidden Actor is replicating properly to Server and Clients.
 *
 * The test includes a single server and two client workers.
 * The flow is as follows:
 *  - Setup:
 *	  - One cube actor already placed in the level at Location FVector(0.0f, 0.0f, 80.0f).
 *    - The Server spawns a TestMovementCharacter and makes Client 1 possess it.
 *  - Test:
 *    - Each worker tests if it can initially see the AReplicatedVisibilityTestActor.
 *    - After ensuring possession happened, the Server moves Client 1's Character to a remote location, so it cannot see the
 *AReplicatedVisibilityTestActor.
 *    - After ensuring movement replicated correctly, Client 1 checks it can no longer see the AReplicatedVisibilityTestActor.
 *	  - The Server sets the AReplicatedVisibilityTestActor to hidden.
 *	  - All Clients check they can no longer see the AReplicatedVisibilityTestActor.
 *	  - The Server moves the character of Client 1 back close to its spawn location, so that the AReplicatedVisibilityTestActor is in its
 *interest area.
 *	  - All Clients check they can still not see the AReplicatedVisibilityTestActor.
 *	  - The Server sets the AReplicatedVisibilityTestActor to not be hidden.
 *	  - All Clients check they can now see the AReplicatedVisibilityTestActor.
 *  - Cleanup:
 *    - Client 1 repossesses its default pawn.
 *    - The spawned Character is destroyed.
 */

const static float StepTimeLimit = 10.0f;

AVisibilityTest::AVisibilityTest()
	: Super()
{
	Author = "Evi";
	Description = TEXT("Test Actor Visibility");

	CharacterSpawnLocation = FVector(0.0f, 120.0f, 40.0f);
	CharacterRemoteLocation = FVector(20000.0f, 20000.0f, 50.0f);
}

int AVisibilityTest::GetNumberOfVisibilityTestActors()
{
	int Counter = 0;
	for (TActorIterator<AReplicatedVisibilityTestActor> Iter(GetWorld()); Iter; ++Iter)
	{
		Counter++;
	}

	return Counter;
}

void AVisibilityTest::PrepareTest()
{
	Super::PrepareTest();

	{ // Step 0 - The server spawn a TestMovementCharacter and makes Client 1 possess it.
		AddStep(TEXT("VisibilityTestServerSetup"), FWorkerDefinition::Server(1), nullptr, [this]() {
			ASpatialFunctionalTestFlowController* ClientOneFlowController = GetFlowController(ESpatialFunctionalTestWorkerType::Client, 1);
			APlayerController* PlayerController = Cast<APlayerController>(ClientOneFlowController->GetOwner());

			if (IsValid(PlayerController))
			{
				ClientOneSpawnedPawn =
					GetWorld()->SpawnActor<ATestMovementCharacter>(CharacterSpawnLocation, FRotator::ZeroRotator, FActorSpawnParameters());
				RegisterAutoDestroyActor(ClientOneSpawnedPawn);

				ClientOneDefaultPawn = PlayerController->GetPawn();

				PlayerController->Possess(ClientOneSpawnedPawn);

				FinishStep();
			}
		});
	}

	{ // Step 1 - All workers check if they have one ReplicatedVisibilityTestActor in the world, and set a reference to it.
		AddStep(
			TEXT("VisibilityTestAllWorkersCheckVisibility"), FWorkerDefinition::AllWorkers, nullptr, nullptr,
			[this](float DeltaTime) {
				TArray<AActor*> FoundActors;
				UGameplayStatics::GetAllActorsOfClass(GetWorld(), AReplicatedVisibilityTestActor::StaticClass(), FoundActors);

				if (FoundActors.Num() == 1)
				{
					TestActor = Cast<AReplicatedVisibilityTestActor>(FoundActors[0]);

					if (IsValid(TestActor))
					{
						FinishStep();
					}
				}
			},
			StepTimeLimit);
	}

	{ // Step 2 - Client 1 checks if it has correctly possessed the TestMovementCharacter.
		AddStep(
			TEXT("VisibilityTestClientCheckPossesion"), FWorkerDefinition::Client(1), nullptr, nullptr,
			[this](float DeltaTime) {
				ASpatialFunctionalTestFlowController* FlowController = GetLocalFlowController();
				APlayerController* PlayerController = Cast<APlayerController>(FlowController->GetOwner());
				ATestMovementCharacter* PlayerCharacter = Cast<ATestMovementCharacter>(PlayerController->GetPawn());
				if (IsValid(PlayerController))
				{
					if (PlayerCharacter == PlayerController->AcknowledgedPawn)
					{
						FinishStep();
					}
				}
			},
			StepTimeLimit);
	}

	{ // Step 4 - Server moves the TestMovementCharacter of Client 1 to a remote location, so that it does not see the
	  // AReplicatedVisibilityTestActor.
		AddStep(TEXT("VisibilityTestServerMoveClient1"), FWorkerDefinition::Server(1), nullptr, [this]() {
			if (ClientOneSpawnedPawn->SetActorLocation(CharacterRemoteLocation))
			{
				if (ClientOneSpawnedPawn->GetActorLocation().Equals(CharacterRemoteLocation, 1.0f))
				{
					FinishStep();
				}
			}
		});
	}

	{ // Step 5 - Client 1 makes sure that the movement was correctly replicated
		AddStep(
			TEXT("VisibilityTestClientCheckFirstMovement"), FWorkerDefinition::Client(1), nullptr, nullptr,
			[this](float DeltaTime) {
				ASpatialFunctionalTestFlowController* FlowController = GetLocalFlowController();
				APlayerController* PlayerController = Cast<APlayerController>(FlowController->GetOwner());
				ATestMovementCharacter* PlayerCharacter = Cast<ATestMovementCharacter>(PlayerController->GetPawn());

				if (IsValid(PlayerCharacter))
				{
					if (PlayerCharacter->GetActorLocation().Equals(CharacterRemoteLocation, 1.0f))
					{
						FinishStep();
					}
				}
			},
			StepTimeLimit);
	}

	{ // Step 6 - Client 1 checks that it can no longer see the AReplicatedVisibilityTestActor.
		AddStep(
			TEXT("VisibilityTestClient1CheckReplicatedActorsBeforeActorHidden"), FWorkerDefinition::Client(1), nullptr, nullptr,
			[this](float DeltaTime) {
				if (GetNumberOfVisibilityTestActors() == 0 && !IsValid(TestActor))
				{
					FinishStep();
				}
			},
			StepTimeLimit);
	}

	{ // Step 7 - Server Set AReplicatedVisibilityTestActor to be hidden.
		AddStep(TEXT("VisibilityTestServerSetActorHidden"), FWorkerDefinition::Server(1), nullptr, [this]() {
			if (IsValid(TestActor))
			{
				TestActor->SetHidden(true);
				FinishStep();
			}
		});
	}

	{ // Step 8 - Clients check that the AReplicatedVisibilityTestActor is no longer replicated.
		AddStep(TEXT("VisibilityTestClientCheckReplicatedActorsAfterSetActorHidden"), FWorkerDefinition::AllClients, nullptr, nullptr,
				[this](float DeltaTime) {
					if (GetNumberOfVisibilityTestActors() == 0 && !IsValid(TestActor))
					{
						FinishStep();
					}
				});
	}

	{ // Step 9 - Server moves Client 1 close to the cube.
		AddStep(TEXT("VisibilityTestServerMoveClient1CloseToCube"), FWorkerDefinition::Server(1), nullptr, [this]() {
			if (ClientOneSpawnedPawn->SetActorLocation(CharacterSpawnLocation))
			{
				if (ClientOneSpawnedPawn->GetActorLocation().Equals(CharacterSpawnLocation, 1.0f))
				{
					FinishStep();
				}
			}
		});
	}

	{ // Step 10 - Client 1 checks that the movement was replicated correctly.
		AddStep(
			TEXT("VisibilityTestClientCheckSecondMovement"), FWorkerDefinition::Client(1), nullptr, nullptr,
			[this](float DeltaTime) {
				ASpatialFunctionalTestFlowController* FlowController = GetLocalFlowController();
				APlayerController* PlayerController = Cast<APlayerController>(FlowController->GetOwner());
				ATestMovementCharacter* PlayerCharacter = Cast<ATestMovementCharacter>(PlayerController->GetPawn());

				if (IsValid(PlayerCharacter))
				{
					if (PlayerCharacter->GetActorLocation().Equals(CharacterSpawnLocation, 1.0f))
					{
						FinishStep();
					}
				}
			},
			StepTimeLimit);
	}

	{ // Step 11 - Clients check that they can still not see the AReplicatedVisibilityTestActor
		AddStep(
			TEXT("VisibilityTestClientCheckFinalReplicatedActors"), FWorkerDefinition::AllClients, nullptr, nullptr,
			[this](float DeltaTime) {
				if (GetNumberOfVisibilityTestActors() == 0 && !IsValid(TestActor))
				{
					FinishStep();
				}
			},
			StepTimeLimit);
	}

	{ // Step 12 - Server Set AReplicatedVisibilityTestActor to not be hidden anymore.
		AddStep(TEXT("VisibilityTestServerSetActorNotHidden"), FWorkerDefinition::Server(1), nullptr, [this]() {
			if (IsValid(TestActor))
			{
				TestActor->SetHidden(false);
				FinishStep();
			}
		});
	}

	{ // Step 13 - Clients check that the AReplicatedVisibilityTestActor is being replicated again.
		AddStep(TEXT("VisibilityTestClientCheckFinalReplicatedNonHiddenActors"), FWorkerDefinition::AllClients, nullptr, nullptr,
				[this](float DeltaTime) {
					if (GetNumberOfVisibilityTestActors() == 1)
					{
						FinishStep();
					}
				});
	}

	{ // Step 14 - Server Cleanup.
		AddStep(TEXT("VisibilityTestServerCleanup"), FWorkerDefinition::Server(1), nullptr, [this]() {
			// Possess the original pawn, so that the spawned character can get destroyed correctly
			ASpatialFunctionalTestFlowController* ClientOneFlowController = GetFlowController(ESpatialFunctionalTestWorkerType::Client, 1);
			APlayerController* PlayerController = Cast<APlayerController>(ClientOneFlowController->GetOwner());

			if (IsValid(PlayerController))
			{
				PlayerController->Possess(ClientOneDefaultPawn);

				FinishStep();
			}
		});
	}
}
