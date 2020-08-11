// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "SpatialTestPossession.h"
#include "Containers/Array.h"
#include "GameFramework/PlayerController.h"
#include "SpatialFunctionalTestFlowController.h"
#include "TestPossessionPawn.h"

/**
 * This test tests client possession over pawns.
 *
 * The test includes a single server and two client workers. The client workers begin with a player controller and their default pawns,
 * which they initially possess. The flow is as follows:
 *  - Setup:
 *    - Two test pawn actors are spawned, one for each client, with an offset in the y direction for easy visualisation
 *    - The controllers for each client  possess the spawned test pawn actors
 *  - Test:
 *    - The clients  assert that the pawn they currently possess are test pawns
 *  - Cleanup:
 *    - The clients repossess their default pawns
 *    - The test pawns are destroyed
 */

ASpatialTestPossession::ASpatialTestPossession()
	: Super()
{
	Author = "Miron";
	Description = TEXT("Test Actor Possession");
}

void ASpatialTestPossession::BeginPlay()
{
	Super::BeginPlay();

	AddStep(TEXT("SpatialTestPossessionServerSetupStep"), FWorkerDefinition::Server(1), nullptr, nullptr,
			[](ASpatialFunctionalTest* NetTest, float DeltaTime) {
				ASpatialTestPossession* Test = Cast<ASpatialTestPossession>(NetTest);

				float YToSpawnAt = -60.0f;
				float YSpawnIncrement = 120.0f;

				for (ASpatialFunctionalTestFlowController* FlowController : Test->GetFlowControllers())
				{
					if (FlowController->WorkerDefinition.Type == ESpatialFunctionalTestWorkerType::Server)
					{
						continue;
					}
					// Spawn the actor for the client to possess, and increment the y variable for the next spawn
					ATestPossessionPawn* TestPawn = Test->GetWorld()->SpawnActor<ATestPossessionPawn>(
						FVector(0.0f, YToSpawnAt, 50.0f), FRotator::ZeroRotator, FActorSpawnParameters());
					YToSpawnAt += YSpawnIncrement;

					Test->RegisterAutoDestroyActor(TestPawn);

					AController* PlayerController = Cast<AController>(FlowController->GetOwner());

					// Save old one to put it back in the final step
					Test->OriginalPawns.Add(TPair<AController*, APawn*>(PlayerController, PlayerController->GetPawn()));

					// Actually do the possession of the test pawn
					PlayerController->Possess(TestPawn);
				}

				Test->FinishStep();
			});

	AddStep(
		TEXT("SpatialTestPossessionClientCheckStep"), FWorkerDefinition::AllClients,
		[](ASpatialFunctionalTest* NetTest) -> bool {
			AController* PlayerController = Cast<AController>(NetTest->GetLocalFlowController()->GetOwner());
			return IsValid(PlayerController->GetPawn());
		},
		[](ASpatialFunctionalTest* NetTest) {
			ASpatialTestPossession* Test = Cast<ASpatialTestPossession>(NetTest);
			ASpatialFunctionalTestFlowController* FlowController = Test->GetLocalFlowController();

			AController* PlayerController = Cast<AController>(FlowController->GetOwner());

			// Run the assertion. This checks the currently possessed pawn is a TestPossessionPawn, and fails if not
			Test->AssertTrue(PlayerController->GetPawn()->GetClass() == ATestPossessionPawn::StaticClass(),
							 TEXT("Player has possessed test pawn"), PlayerController);

			Test->FinishStep();
		});

	AddStep(TEXT("SpatialTestPossessionServerPossessOldPawns"), FWorkerDefinition::Server(1), nullptr, nullptr,
			[](ASpatialFunctionalTest* NetTest, float DeltaTime) {
				ASpatialTestPossession* Test = Cast<ASpatialTestPossession>(NetTest);
				for (const auto& OriginalPawnPair : Test->OriginalPawns)
				{
					OriginalPawnPair.Key->Possess(OriginalPawnPair.Value);
				}
				Test->FinishStep();
			});
}
