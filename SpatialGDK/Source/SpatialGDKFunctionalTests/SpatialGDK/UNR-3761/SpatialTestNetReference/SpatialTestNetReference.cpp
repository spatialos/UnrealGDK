// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "SpatialTestNetReference.h"
#include "GameFramework/PlayerController.h"
#include "SpatialFunctionalTestFlowController.h"
#include "SpatialGDKFunctionalTests/SpatialGDK/SpatialTestCharacterMovement/TestMovementCharacter.h"
#include "Kismet/GameplayStatics.h"
#include "CubeWithReferences.h"
#include "SpatialGDKSettings.h"


/**
 * This test automates the Net Reference Test gym, which tested that references to replicated actors are stable when actors go in and out of relevance.
 * This test also adds an interest check on top of the previously mentioned Gym.
 * 
 * The test includes a single server and two client workers. For performance considerations, the only client that is executing the test is Client 1.
 * The flow is as follows:
 * - Setup:
 *   - The Server spawns 4 CubeWithReferences objects and sets up their references.
 * - Test:
 *	 - The test contains 2 runs of the same flow: Client 1 moves its character to 4 specific locations and, after arriving at each location, it checks that:
 *			1) The correct amount of cubes are present in the world, based on the default NetCullDistanceSquared of the PlayerController.
 *			2) The references to the replicated actors are correct.
 * - Clean-up:
 *	- The Server destroys the previously spawned CubeWithReferences
 *  - The test variables are reset before the test finishes so that it can be ran multiple times.
 */

ASpatialTestNetReference::ASpatialTestNetReference()
	: Super()
{
	Author = "Andrei";
	Description = TEXT("Test Net Reference");

	LocationIndex = 0;
	TimerHelper = 0.0f;

	TestLocations.Add(TPair<FVector, int> (FVector(0.0f, -15000.0f, 40.0f), 1));
	TestLocations.Add(TPair<FVector, int>(FVector(5000.0f, -5000.0f, 40.0f), 2));
	TestLocations.Add(TPair<FVector, int>(FVector(5000.0f, 1000.0f, 40.0f), 3));
	TestLocations.Add(TPair<FVector, int> (FVector(100.0f, 100.0f, 40.0f), 4));
}

void ASpatialTestNetReference::BeginPlay()
{
	Super::BeginPlay();

	AddServerStep(TEXT("SpatialTestNetReferenceServerSetup"), 1, nullptr, [](ASpatialFunctionalTest* NetTest) {
		ASpatialTestNetReference* Test = Cast<ASpatialTestNetReference>(NetTest);

		// Set up the cubes' spawn locations
		TArray<FVector> CubeLocations;
		CubeLocations.Add(FVector(0.0f, -10000.0f, 40.0f));
		CubeLocations.Add(FVector(10000.0f, 0.0f, 40.0f));
		CubeLocations.Add(FVector(0.0f, 10000.0f, 40.0f));
		CubeLocations.Add(FVector(-10000.0f, 0.0f, 40.0f));

		// Spawn the cubes
		TArray<ACubeWithReferences*> TestCubes;
		int NumberOfCubes = CubeLocations.Num();

		for (int i = 0; i < NumberOfCubes; ++i)
		{
			ACubeWithReferences* CubeWithReferences = NetTest->GetWorld()->SpawnActor<ACubeWithReferences>(CubeLocations[i], FRotator::ZeroRotator, FActorSpawnParameters());
			TestCubes.Add(CubeWithReferences);
		}

		// Set the cubes' references
		for (int i = 0; i < NumberOfCubes; ++i)
		{
			TestCubes[i]->Neighbour1 = TestCubes[(i + 1) % NumberOfCubes];
			TestCubes[i]->Neighbour2 = TestCubes[(i + NumberOfCubes -1) % NumberOfCubes];
		}

		// Set the PositionUpdateFrequency to a higher value so that the amount of waiting time before checking the references can be smaller, decreasing the overall duration of the test
		Test->PreviousPositionUpdateFrequency = GetMutableDefault<USpatialGDKSettings>()->PositionUpdateFrequency;
		GetMutableDefault<USpatialGDKSettings>()->PositionUpdateFrequency = 10000.0f;

		// Spawn the TestMovementCharacter actor for client 1 to possess.
		for (ASpatialFunctionalTestFlowController* FlowController : Test->GetFlowControllers())
		{
			if (FlowController->ControllerType == ESpatialFunctionalTestFlowControllerType::Client && FlowController->ControllerInstanceId == 1)
			{
				ATestMovementCharacter* TestCharacter = Test->GetWorld()->SpawnActor<ATestMovementCharacter>(FVector::ZeroVector, FRotator::ZeroRotator, FActorSpawnParameters());
				AController* PlayerController = Cast<AController>(FlowController->GetOwner());

				Test->OriginalPawn = TPair<AController*, APawn*>(PlayerController, PlayerController->GetPawn());
				Test->RegisterAutoDestroyActor(TestCharacter);

				PlayerController->Possess(TestCharacter);
			}
		}

		Test->FinishStep();
		});

	AddClientStep(TEXT("SpatialTestNetReferenceClientExecuteTest"), 1, nullptr, nullptr, [](ASpatialFunctionalTest* NetTest, float DeltaTime) {
		ASpatialTestNetReference* Test = Cast<ASpatialTestNetReference>(NetTest);
		AController* PlayerController = Cast<AController>(NetTest->GetLocalFlowController()->GetOwner());
		ATestMovementCharacter* PlayerCharacter = Cast<ATestMovementCharacter>(PlayerController->GetPawn());

		// The index of the current move, the % is required since the test goes through each test location twice.
		int CurrentMoveIndex = Test->LocationIndex % Test->TestLocations.Num();

		// Move the character to the correct location
		if(Test->TimerHelper == 0.0f)
		{
			PlayerCharacter->ServerMoveToLocation(Test->TestLocations[CurrentMoveIndex].Key);
		}

		// After arriving at the correct location, perform the required checks
		if (PlayerCharacter->GetActorLocation().Equals(Test->TestLocations[CurrentMoveIndex].Key, 1.0f))
		{
			// Wait for a second as it may take some time for the references to be updated correctly
			if (Test->TimerHelper < 0.1f)
			{
				Test->TimerHelper += DeltaTime;
				return;
			}


			// Reset the timer so that it can be used for the next move
			Test->TimerHelper = 0.0f;

			TArray<AActor*> CubesWithReferences;
			UGameplayStatics::GetAllActorsOfClass(Test->GetWorld(), ACubeWithReferences::StaticClass(), CubesWithReferences);

			// Check the number of relevant cubes
			Test->AssertEqual_Int(CubesWithReferences.Num(), Test->TestLocations[CurrentMoveIndex].Value, TEXT("Correct number of references are relevant to the player"));

			for (AActor* ArrayObject : CubesWithReferences)
			{
				ACubeWithReferences* CurrentCube = Cast<ACubeWithReferences>(ArrayObject);
				FVector CurrentCubeLocation = CurrentCube->GetActorLocation();

				bool bHasCorrectReferences = true;
				int ValidReferences = 0;

				for (AActor* OtherObject : CubesWithReferences)
				{
					ACubeWithReferences* OtherCube = Cast<ACubeWithReferences>(OtherObject);
					FVector OtherCubeLocation = OtherObject->GetActorLocation();

					// If the cube is the current one or the diagonally opposed one, then ignore it as it should never be a neigbhour of the current cube
					if(OtherCubeLocation.Equals(CurrentCubeLocation) || (OtherCubeLocation.X == -CurrentCubeLocation.X && OtherCubeLocation.Y == -CurrentCubeLocation.Y))
					{
						continue;
					}

					// Check that the current cube has a neighbour reference to this OtherCube
					bHasCorrectReferences &= (CurrentCube->Neighbour1 == OtherCube) || (CurrentCube->Neighbour2 == OtherCube);

					if (bHasCorrectReferences)
					{ 
						++ValidReferences;
					}
				}

				if (ValidReferences == 0)
				{
					// Check that the current cube has 0 valid references
					bHasCorrectReferences &= !IsValid(CurrentCube->Neighbour1) && !IsValid(CurrentCube->Neighbour2);
				}
				else if (ValidReferences == 1)
				{
					// We have previously checked that one neighbour reference is correctly pointing to the neighbour cube, also check that the other reference is null
					bHasCorrectReferences &=  !IsValid(CurrentCube->Neighbour1) || !IsValid(CurrentCube->Neighbour2);
				}

				Test->AssertEqual_Bool(bHasCorrectReferences, true, TEXT("References are correct"));
			}

			// Make the character move to the next step in the sequence
			Test->LocationIndex++;

			// After checking all locations twice, finish the test
			if (Test->LocationIndex == 2 * Test->TestLocations.Num())
			{
				// Reset this variable in order to allow for the test to be run multiple times
				Test->LocationIndex = 0;

				Test->FinishStep();
			}
		}
	});

	AddServerStep(TEXT("SpatialTestNetReferenceServerCleanup"), 1, nullptr, nullptr, [](ASpatialFunctionalTest* NetTest, float DeltaTime) {
		ASpatialTestNetReference* Test = Cast<ASpatialTestNetReference>(NetTest);

		// Destroy the previously spawned cubes
		TArray<AActor*> SpawnedObjects;
		UGameplayStatics::GetAllActorsOfClass(Test->GetWorld(), ACubeWithReferences::StaticClass(), SpawnedObjects);

		for (AActor* CubeToDestroy : SpawnedObjects)
		{
			CubeToDestroy->Destroy();
		}

		// Possess the original pawn, so that the spawned character can get destroyed correctly
		Test->OriginalPawn.Key->Possess(Test->OriginalPawn.Value);

		// Reset the PositionUpdateFrequency
		GetMutableDefault<USpatialGDKSettings>()->PositionUpdateFrequency = Test->PreviousPositionUpdateFrequency;

		Test->FinishStep();
	});
}
