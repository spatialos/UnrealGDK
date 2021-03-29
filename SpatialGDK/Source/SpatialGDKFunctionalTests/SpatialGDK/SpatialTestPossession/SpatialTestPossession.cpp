// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "SpatialTestPossession.h"
#include "Containers/Array.h"
#include "GameFramework/PlayerController.h"
#include "SpatialFunctionalTestFlowController.h"
#include "SpatialGDKFunctionalTests/SpatialGDK/TestActors/TestPossessionPawn.h"

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

void ASpatialTestPossession::PrepareTest()
{
	Super::PrepareTest();

	AddStep(TEXT("SpatialTestPossessionServerSetupStep"), FWorkerDefinition::Server(1), nullptr, nullptr, [this](float DeltaTime) {
		float YToSpawnAt = -60.0f;
		float YSpawnIncrement = 120.0f;

		for (ASpatialFunctionalTestFlowController* FlowController : GetFlowControllers())
		{
			if (FlowController->WorkerDefinition.Type == ESpatialFunctionalTestWorkerType::Server)
			{
				continue;
			}
			// Spawn the actor for the client to possess, and increment the y variable for the next spawn
			ATestPossessionPawn* TestPawn = GetWorld()->SpawnActor<ATestPossessionPawn>(FVector(0.0f, YToSpawnAt, 50.0f),
																						FRotator::ZeroRotator, FActorSpawnParameters());
			YToSpawnAt += YSpawnIncrement;

			RegisterAutoDestroyActor(TestPawn);

			AController* PlayerController = Cast<AController>(FlowController->GetOwner());

			// This is a reversible version of the normal possess that will automatically revert the possession back to the original pawns
			// at the end of the test.
			CleanablePossess(PlayerController, TestPawn);
		}

		FinishStep();
	});

	AddStep(
		TEXT("SpatialTestPossessionClientCheckStep"), FWorkerDefinition::AllClients,
		[this]() -> bool {
			AController* PlayerController = Cast<AController>(GetLocalFlowController()->GetOwner());
			return IsValid(PlayerController->GetPawn());
		},
		[this]() {
			ASpatialFunctionalTestFlowController* FlowController = GetLocalFlowController();

			AController* PlayerController = Cast<AController>(FlowController->GetOwner());

			// Run the assertion. This checks the currently possessed pawn is a TestPossessionPawn, and fails if not
			AssertTrue(PlayerController->GetPawn()->GetClass() == ATestPossessionPawn::StaticClass(),
					   TEXT("Player has possessed test pawn"), PlayerController);

			FinishStep();
		});
}
