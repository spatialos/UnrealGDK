// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "DynamicSubobjectsTest.h"
#include "ReplicatedGASTestActor.h"
#include "SpatialFunctionalTestFlowController.h"
#include "SpatialGDKFunctionalTests/SpatialGDK/TestActors/TestMovementCharacter.h"

#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"
#include <SpatialGDK\Public\SpatialGDKSettings.h>

/**
 * Tests if the dynamic Subobject of the AReplicatedGASTestActor is not duplicated on Clients when leaving
 * and re-entering interest.
 *
 * The test includes a single server and one client worker.
 * The flow is as follows:
 *  - Setup:
 *	  - One cube actor already placed in the level at Location FVector(0.0f, 0.0f, 80.0f) needs to be a startup actor - bNetLoadOnClient =
 *true.
 *    - The Server spawns a TestMovementCharacter and makes Client 1 possess it.
 *  - Test:
 *    - Each worker tests if it can initially see the AReplicatedGASTestActor.
 *    - Repeat the following steps MaxDynamicallyAttachedSubobjectsPerClass + 1 times:
 *		- After ensuring possession happened, the Server moves Client 1's Character to a remote location, so it cannot see the
 *AReplicatedGASTestActor.
 *		- After ensuring movement replicated correctly, Client 1 checks it can no longer see the AReplicatedGASTestActor.
 *		- The Server moves the character of Client 1 back close to its spawn location, so that the AReplicatedGASTestActor is
 *in its interest area.
 *	  - If the "Too many dynamic sub objects" error does not appears in the log the test is successful.
 *  - Cleanup:
 *    - Client 1 repossesses its default pawn.
 *    - The spawned Character is destroyed.
 */

const static float StepTimeLimit = 10.0f;

ADynamicSubobjectsTest::ADynamicSubobjectsTest()
	: Super()
{
	Author = "Evi";
	Description = TEXT("Test Dynamic Subobjects Duplication in Client");

	CharacterSpawnLocation = FVector(0.0f, 120.0f, 40.0f);
	CharacterRemoteLocation = FVector(20000.0f, 20000.0f, 40.0f); // Outside of the interest range of the client
}

void ADynamicSubobjectsTest::PrepareTest()
{
	Super::PrepareTest();

	const int DynamicComponentsPerClass = GetDefault<USpatialGDKSettings>()->MaxDynamicallyAttachedSubobjectsPerClass;
	StepTimer = 0.0f;

	{ // Step 0 - The server spawn a TestMovementCharacter and makes Client 1 possess it.
		AddStep(TEXT("DynamicSubobjectsTestSetup"), FWorkerDefinition::Server(1), nullptr, [this]() {
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

	{ // Step 1 - All workers check if they have one AReplicatedGASTestActor in the world, and set a reference to it.
		AddStep(
			TEXT("DynamicSubobjectsTestAllWorkers"), FWorkerDefinition::AllWorkers, nullptr, nullptr,
			[this](float DeltaTime) {
				TArray<AActor*> FoundActors;
				UGameplayStatics::GetAllActorsOfClass(GetWorld(), AReplicatedGASTestActor::StaticClass(), FoundActors);

				if (FoundActors.Num() == 1)
				{
					TestActor = Cast<AReplicatedGASTestActor>(FoundActors[0]);

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
			TEXT("DynamicSubobjectsTestClientCheckPossesion"), FWorkerDefinition::Client(1), nullptr, nullptr,
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

	for (int i = 0; i < DynamicComponentsPerClass + 1; ++i)
	{
		{ // Step 3 - Server moves the TestMovementCharacter of Client 1 to a remote location, so that it does not see the
		  // AReplicatedGASTestActor.
			AddStep(TEXT("DynamicSubobjectsTestServerMoveClient1"), FWorkerDefinition::Server(1), nullptr, [this, i]() {
				if (ClientOneSpawnedPawn->SetActorLocation(CharacterRemoteLocation))
				{
					if (ClientOneSpawnedPawn->GetActorLocation().Equals(CharacterRemoteLocation, 1.0f))
					{
						FinishStep();
					}
				}
			});
		}

		{ // Step 4 - Client 1 makes sure that the movement was correctly replicated
			AddStep(
				TEXT("DynamicSubobjectsTestClientCheckFirstMovement"), FWorkerDefinition::Client(1), nullptr, nullptr,
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

		{ // Step 5 - Server increases AReplicatedGASTestActor's TestIntProperty to enable checking if the client is out of interest later.
			AddStep(TEXT("DynamicSubobjectsTestServerIncreasesIntValue"), FWorkerDefinition::Server(1), nullptr, [this, i]() {
				TArray<AActor*> FoundActors;
				UGameplayStatics::GetAllActorsOfClass(GetWorld(), AReplicatedGASTestActor::StaticClass(), FoundActors);
				if (FoundActors.Num() == 1)
				{
					TestActor = Cast<AReplicatedGASTestActor>(FoundActors[0]);
					TestActor->TestIntProperty = i + 1;
				}
				if (TestActor->TestIntProperty == i + 1)
				{
					FinishStep();
				}
			});
		}

		{ // Step 6 - Client 1 checks it can no longer see the AReplicatedGASTestActor
			AddStep(
				TEXT("DynamicSubobjectsTestClientCheckIntValueIncreased"), FWorkerDefinition::Client(1), nullptr, nullptr,
				[this, i](float DeltaTime) {
					TArray<AActor*> FoundActors;
					UGameplayStatics::GetAllActorsOfClass(GetWorld(), AReplicatedGASTestActor::StaticClass(), FoundActors);
					if (FoundActors.Num() == 1)
					{
						TestActor = Cast<AReplicatedGASTestActor>(FoundActors[0]);

						RequireNotEqual_Int(TestActor->TestIntProperty, i + 1, TEXT("Check TestIntProperty didn't get replicated"));
						StepTimer += DeltaTime;
						if (StepTimer >= 0.5f)
						{
							FinishStep();
							StepTimer = 0.0f; // reset for the next time
						}
					}
				},
				StepTimeLimit);
		}

		{ // Step7 - Server moves Client 1 close to the cube.
			AddStep(TEXT("DynamicSubobjectsTestServerMoveClient1CloseToCube"), FWorkerDefinition::Server(1), nullptr, [this]() {
				if (ClientOneSpawnedPawn->SetActorLocation(CharacterSpawnLocation))
				{
					if (ClientOneSpawnedPawn->GetActorLocation().Equals(CharacterSpawnLocation, 1.0f))
					{
						FinishStep();
					}
				}
			});
		}

		{ // Step 8 - Client 1 checks that the movement was replicated correctly.
			AddStep(
				TEXT("DynamicSubobjectsTestClientCheckSecondMovement"), FWorkerDefinition::Client(1), nullptr, nullptr,
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

		{ // Step 9 - Client 1 checks it can see the AReplicatedGASTestActor
			AddStep(
				TEXT("DynamicSubobjectsTestClientCheckIntValueIncreased2"), FWorkerDefinition::Client(1), nullptr, nullptr,
				[this, i](float DeltaTime) {
					TArray<AActor*> FoundActors;
					UGameplayStatics::GetAllActorsOfClass(GetWorld(), AReplicatedGASTestActor::StaticClass(), FoundActors);
					if (FoundActors.Num() == 1)
					{
						TestActor = Cast<AReplicatedGASTestActor>(FoundActors[0]);
						if (TestActor->TestIntProperty == i + 1)
						{
							FinishStep();
						}
					}
				},
				StepTimeLimit);
		}
	}

	{ // Step 10 - Server Cleanup.
		AddStep(TEXT("DynamicSubobjectsTestServerCleanup"), FWorkerDefinition::Server(1), nullptr, [this]() {
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
