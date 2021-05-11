// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "DynamicSubobjectsTest.h"
#include "ReplicatedGASTestActor.h"
#include "SpatialFunctionalTestFlowController.h"
#include "SpatialGDKFunctionalTests/SpatialGDK/TestActors/TestMovementCharacter.h"
#include "SpatialGDKSettings.h"

#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"

#include "Components/SceneComponent.h"

/**
 * Tests if the dynamic sub-object of the AReplicatedGASTestActor is not duplicated on Clients when leaving
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
 *
 *
 * Then, a second test case is tested with a further iteration.
 * This tests that
 * 1. AReplicatedGASTestActor moves out of the client's interest
 * 2. AReplicatedGASTestActor has all their existing components removed
 * 3. AReplicatedGASTestActor moves into the client's interest
 * 4. The client sees AReplicatedGASTestActor no longer has any components
 * This extra test case is implemented in steps 6.1 and 9.1
 */

const static float StepTimeLimit = 10.0f;

ADynamicSubobjectsTest::ADynamicSubobjectsTest()
	: Super()
{
	Author = "Evi&Arthur&Miron";
	Description = TEXT("Test Dynamic Subobjects Duplication in Client");

	CharacterSpawnLocation = FVector(0.0f, 120.0f, 40.0f);
	CharacterRemoteLocation = FVector(20000.0f, 20000.0f, 40.0f); // Outside of the interest range of the client
}

void ADynamicSubobjectsTest::PrepareTest()
{
	Super::PrepareTest();

	const int DynamicComponentsPerClass = GetDefault<USpatialGDKSettings>()->MaxDynamicallyAttachedSubobjectsPerClass;
	StepTimer = 0.0f;

	// Step 0 - The server spawn a TestMovementCharacter and makes Client 1 possess it.
	AddStep(TEXT("DynamicSubobjectsTestSetup"), FWorkerDefinition::Server(1), nullptr, [this]() {
		ASpatialFunctionalTestFlowController* ClientOneFlowController = GetFlowController(ESpatialFunctionalTestWorkerType::Client, 1);
		APlayerController* PlayerController = Cast<APlayerController>(ClientOneFlowController->GetOwner());

		if (AssertIsValid(PlayerController, TEXT("PlayerController should be valid")))
		{
			ClientOneSpawnedPawn =
				GetWorld()->SpawnActor<ATestMovementCharacter>(CharacterSpawnLocation, FRotator::ZeroRotator, FActorSpawnParameters());
			RegisterAutoDestroyActor(ClientOneSpawnedPawn);

			ClientOneDefaultPawn = PlayerController->GetPawn();

			PlayerController->Possess(ClientOneSpawnedPawn);

			FinishStep();
		}
	});

	// Step 1 - All workers check if they have one AReplicatedGASTestActor in the world, and set a reference to it.
	AddStep(
		TEXT("DynamicSubobjectsTestAllWorkers"), FWorkerDefinition::AllWorkers, nullptr, nullptr,
		[this](float DeltaTime) {
			TestActor = GetReplicatedTestActor();
			if (TestActor)
			{
				FinishStep();
			}
		},
		StepTimeLimit);

	// Step 2 - Client 1 checks if it has correctly possessed the TestMovementCharacter.
	AddStep(
		TEXT("DynamicSubobjectsTestClientCheckPossesion"), FWorkerDefinition::Client(1), nullptr, nullptr,
		[this](float DeltaTime) {
			APlayerController* PlayerController = GetFlowPlayerController();
			APawn* PlayerCharacter = GetFlowPawn();
			if (AssertIsValid(PlayerCharacter, TEXT("PlayerCharacter should be valid"))
				&& PlayerCharacter == PlayerController->AcknowledgedPawn)
			{
				FinishStep();
			}
		},
		StepTimeLimit);

	for (int i = 0; i < DynamicComponentsPerClass + 2; ++i)
	{
		const bool LastStepLoop = i == DynamicComponentsPerClass + 1;

		// Step 3 - Server moves the TestMovementCharacter of Client 1 to a remote location, so that it does not see the
		// AReplicatedGASTestActor.
		AddStep(TEXT("DynamicSubobjectsTestServerMoveClient1"), FWorkerDefinition::Server(1), nullptr, [this]() {
			if (ClientOneSpawnedPawn->SetActorLocation(CharacterRemoteLocation)
				&& AssertEqual_Vector(ClientOneSpawnedPawn->GetActorLocation(), CharacterRemoteLocation,
									  TEXT("Client pawn was not moved to remote location"), 1.0f))
			{
				FinishStep();
			}
		});

		// Step 4 - Client 1 makes sure that the movement was correctly replicated
		AddStep(
			TEXT("DynamicSubobjectsTestClientCheckFirstMovement"), FWorkerDefinition::Client(1), nullptr, nullptr,
			[this](float DeltaTime) {
				APawn* PlayerCharacter = GetFlowPawn();

				if (AssertIsValid(PlayerCharacter, TEXT("PlayerCharacter should not be nullptr"))
					&& AssertEqual_Vector(PlayerCharacter->GetActorLocation(), CharacterRemoteLocation,
										  TEXT("Character was not moved to remote location"), 1.0f))
				{
					FinishStep();
				}
			},
			StepTimeLimit);

		// Step 5 - Server increases AReplicatedGASTestActor's TestIntProperty to enable checking if the client is out of interest later.
		AddStep(TEXT("DynamicSubobjectsTestServerIncreasesIntValue"), FWorkerDefinition::Server(1), nullptr, [this, i]() {
			if (AssertIsValid(TestActor, TEXT("Test actor should be valid")))
			{
				TestActor->TestIntProperty = i + 1;
				FinishStep();
			}
		});

		// Step 6 - Client 1 checks it can no longer see the AReplicatedGASTestActor
		AddStep(
			TEXT("DynamicSubobjectsTestClientCheckIntValueIncreased"), FWorkerDefinition::Client(1), nullptr, nullptr,
			[this, i](float DeltaTime) {
				if (AssertIsValid(TestActor, TEXT("Test actor should be valid")))
				{
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

		if (LastStepLoop)
		{
			// step 6.1 - Server removes all components for secondary test case
			AddStep(
				TEXT("DynamicSubobjectsTestServerDestroyActorComponents"), FWorkerDefinition::Server(1), nullptr, nullptr,
				[this](float DeltaTime) {
					if (AssertIsValid(TestActor, TEXT("Test actor should be valid")))
					{
						TArray<USceneComponent*> AllSceneComps;
						TestActor->GetComponents<USceneComponent>(AllSceneComps);

						RequireCompare_Int(AllSceneComps.Num(), EComparisonMethod::Greater_Than_Or_Equal_To, 1,
										   TEXT("For this test, DynamicSubobjectTestActor should have at least 1 component"));

						// delete all the components on the actor
						for (int j = 0; j < AllSceneComps.Num(); j++)
						{
							AllSceneComps[j]->DestroyComponent();
						}
						FinishStep();
					}
				},
				StepTimeLimit);
		}

		// Step7 - Server moves Client 1 close to the cube.
		AddStep(TEXT("DynamicSubobjectsTestServerMoveClient1CloseToCube"), FWorkerDefinition::Server(1), nullptr, [this]() {
			if (ClientOneSpawnedPawn->SetActorLocation(CharacterSpawnLocation)
				&& AssertEqual_Vector(ClientOneSpawnedPawn->GetActorLocation(), CharacterSpawnLocation,
									  TEXT("Server 1 should see the pawn close to the initial spawn location"), 1.0f))
			{
				FinishStep();
			}
		});

		// Step 8 - Client 1 checks that the movement was replicated correctly.
		AddStep(
			TEXT("DynamicSubobjectsTestClientCheckSecondMovement"), FWorkerDefinition::Client(1), nullptr, nullptr,
			[this](float DeltaTime) {
				APawn* PlayerCharacter = GetFlowPawn();

				if (AssertIsValid(PlayerCharacter, TEXT("PlayerCharacter should be valid"))
					&& AssertEqual_Vector(PlayerCharacter->GetActorLocation(), CharacterSpawnLocation,
										  TEXT("Client 1 should see themself close to the initial spawn location"), 1.0f))
				{
					FinishStep();
				}
			},
			StepTimeLimit);

		// Step 9 - Client 1 checks it can see the AReplicatedGASTestActor
		AddStep(
			TEXT("DynamicSubobjectsTestClientCheckIntValueIncreased2"), FWorkerDefinition::Client(1), nullptr, nullptr,
			[this, i](float DeltaTime) {
				if (AssertIsValid(TestActor, TEXT("Test actor should be valid"))
					&& AssertEqual_Int(TestActor->TestIntProperty, i + 1, TEXT("Client 1 should see the updated TestIntProperty value")))
				{
					FinishStep();
				}
			},
			StepTimeLimit);

		if (LastStepLoop)
		{
			// Step 9.1 - Client 1 checks all components on ReplicatedGASTestActor have been removed
			AddStep(
				TEXT("DynamicSubobjectsTestClientCheckIntValueIncreased2"), FWorkerDefinition::Client(1), nullptr, nullptr,
				[this](float DeltaTime) {
					if (AssertIsValid(TestActor, TEXT("Test actor should be valid")))
					{
						TArray<UActorComponent*> AllActorComp;
						TestActor->GetComponents<UActorComponent>(AllActorComp);

						RequireEqual_Int(AllActorComp.Num(), 0,
										 TEXT("All components on DynamicSubobjectTestActor should have been destroyed."));
						FinishStep();
					}
				},
				StepTimeLimit);
		}
	}

	// Step 10 - Server Cleanup.
	AddStep(TEXT("DynamicSubobjectsTestServerCleanup"), FWorkerDefinition::Server(1), nullptr, [this]() {
		// Possess the original pawn, so that the spawned character can get destroyed correctly
		ASpatialFunctionalTestFlowController* ClientOneFlowController = GetFlowController(ESpatialFunctionalTestWorkerType::Client, 1);
		APlayerController* PlayerController = Cast<APlayerController>(ClientOneFlowController->GetOwner());

		if (AssertIsValid(PlayerController, TEXT("PlayerController should be valid")))
		{
			PlayerController->Possess(ClientOneDefaultPawn);
			FinishStep();
		}
	});
}

AReplicatedGASTestActor* ADynamicSubobjectsTest::GetReplicatedTestActor()
{
	TArray<AActor*> FoundActors;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), AReplicatedGASTestActor::StaticClass(), FoundActors);
	if (AssertEqual_Int(FoundActors.Num(), 1, TEXT("There should only be one actor of type AReplicatedGASTestActor in the world")))
	{
		TestActor = Cast<AReplicatedGASTestActor>(FoundActors[0]);
		if (AssertIsValid(TestActor, TEXT("TestActor must be valid")))
		{
			return TestActor;
		}
	}
	return nullptr;
}
