// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "SpatialTestNetReference.h"
#include "GameFramework/PlayerController.h"
#include "SpatialFunctionalTestFlowController.h"
#include "SpatialGDKFunctionalTests/SpatialGDK/SpatialTestCharacterMovement/TestMovementCharacter.h"
#include "Kismet/GameplayStatics.h"
#include "CubeWithReferences.h"

/**
 * This test automates the Net Reference Test gym, which tested that references to replicated actors are stable when actors go in and out of relevance.
 * This test also adds an interest check on top of the previously mentioned Gym.
 * NOTE: This test requires a specific GameMode, PlayerController and Map, trying to run this test on a different Map than SpatialTestNetReferenceMap will result in the test failing.
 * 
 * The test includes a single server and two client workers. For performance considerations, the only client that is executing the test is Client 1.
 * The flow is as follows:
 * - Setup:
 *   - The majority of the setup is done in the map itself, that is placing 4 CubeWithReferences objects and correctly setting their locations and references.
 * - Test:
 *	 - The test contains 2 runs of the same flow: Client 1 moves its character to 6 specific locations and, after arriving at each location, it checks that:
 *			1) The correct amount of cubes are present in the world, based on the default NetCullDistanceSquared of the PlayerController.
 *			2) The references to the replicated actors are correct.
 * - Clean-up:
 *  - The test variables are reset before the test finishes so that it can be ran multiple times.
 */

ASpatialTestNetReference::ASpatialTestNetReference()
	: Super()
{
	Author = "Andrei";
	Description = TEXT("Test Net Reference");

	LocationIndex = 0;
	TimerHelper = 0.0f;

	TestLocations.Add(TPair<FVector, int> (FVector(-15000.0f, -15000.0f, 40.0f), 1));
	TestLocations.Add(TPair<FVector, int> (FVector(0.0f, -15000.0f, 40.0f), 2));
	TestLocations.Add(TPair<FVector, int> (FVector(0.0f, 0.0f, 40.0f), 4));
	TestLocations.Add(TPair<FVector, int> (FVector(-15000.0f, 0.0f, 40.0f), 2));
	TestLocations.Add(TPair<FVector, int> (FVector(-15000.0f, 15000.0f, 40.0f), 1));
	TestLocations.Add(TPair<FVector, int> (FVector(0.0f, 15000.0f, 40.0f), 2));
}

void ASpatialTestNetReference::BeginPlay()
{
	Super::BeginPlay();

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
		if (PlayerCharacter->GetActorLocation().Equals(Test->TestLocations[CurrentMoveIndex].Key, 0.25f))
		{
			// Wait for a second as it may take some time for the references to be updated correctly
			if (Test->TimerHelper < 1.0f)
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
}
