// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "CrossServerPossessionTest.h"

#include "Containers/Array.h"
#include "EngineClasses/SpatialNetDriver.h"
#include "GameFramework/PlayerController.h"
#include "SpatialFunctionalTestFlowController.h"
#include "SpatialGDKFunctionalTests/SpatialGDK/TestActors/TestMovementCharacter.h"
#include "SpatialGDKFunctionalTests/SpatialGDK/TestActors/TestPossessionPawn.h"
#include "TestPossessionPlayerController.h"

/**
 * This test tests 1 Controller remote possess over 1 Pawn.
 *
 * This test expects a load balancing grid and ACrossServerPossessionGameMode
 * Recommend to use 2*2 load balancing grid because the position is written in the code
 * The client workers begin with a player controller and their default pawns, which they initially possess.
 * The flow is as follows:
 *  - Setup:
 *    - Specify `GameMode Override` as ACrossServerPossessionGameMode
 *    - Specify `Multi Worker Settings Class` as Zoning 2x2(e.g. BP_Possession_Settings_Zoning2_2 of UnrealGDKTestGyms)
 *	  - Set `Num Required Clients` as 1
 *  - Test:
 *	  - Create a Pawn in first quadrant
 *	  - Create Controller in other quadrant, the position is determined by ACrossServerPossessionGameMode
 *	  - Wait for Pawn in right worker.
 *	  -	The Controller possess the Pawn in server-side
 *	- Result Check:
 *    - ATestPossessionPlayerController::OnPossess should be called == 1 times
 */

ACrossServerPossessionTest::ACrossServerPossessionTest()
{
	Author = "Ken.Yu";
	Description = TEXT("Test Cross-Server Possession");
}

void ACrossServerPossessionTest::PrepareTest()
{
	Super::PrepareTest();

	AddStep(
		TEXT("Cross-Server Possession"), FWorkerDefinition::AllServers, nullptr, /*StartEvent*/ [this](float DeltaTime) {
		ATestPossessionPawn* Pawn = GetPawn();
		AssertIsValid(Pawn, TEXT("Test requires a Pawn"));
		for (ASpatialFunctionalTestFlowController* FlowController : GetFlowControllers())
		{
			if (FlowController->WorkerDefinition.Type == ESpatialFunctionalTestWorkerType::Client)
			{
				ATestPossessionPlayerController* PlayerController = Cast<ATestPossessionPlayerController>(FlowController->GetOwner());
				if (PlayerController && PlayerController->HasAuthority())
				{
					AssertFalse(Pawn->HasAuthority(), TEXT("Pawn shouldn't HasAuthority"), Pawn);
					if (!Pawn->HasAuthority())
					{
						PlayerController->UnPossess();
						PlayerController->RemotePossessOnServer(Pawn);
					}
				}
			}
		}
		FinishStep();
	});


	//The pawn is expected to be spawned on server 4 and thus the player controller is expected to migrate to server 4 to possess it
	AddStep(
		TEXT("Check test result"), FWorkerDefinition::Server(4),
		[this]() -> bool {
			return ATestPossessionPlayerController::OnPossessCalled == 1;
		},
		/*StartEvent*/
		[this]() {
			for (ASpatialFunctionalTestFlowController* FlowController : GetFlowControllers())
			{
				if (FlowController->WorkerDefinition.Type == ESpatialFunctionalTestWorkerType::Client)
				{
					if (ATestPossessionPlayerController* PlayerController =
							Cast<ATestPossessionPlayerController>(FlowController->GetOwner()))
					{
						AssertTrue(PlayerController->HasAuthority(), TEXT("The PlayerController should be on server 4"));
						AssertTrue(PlayerController->HasMigrated(), TEXT("PlayerController should have migrated"), PlayerController);
					}
				}
			}
			FinishStep();
		});

	AddCleanupSteps();
}
