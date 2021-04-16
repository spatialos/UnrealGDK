// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "CrossServerPossessionLockTest.h"
#include "EngineClasses/SpatialNetDriver.h"
#include "GameFramework/PlayerController.h"
#include "Net/UnrealNetwork.h"
#include "SpatialFunctionalTestFlowController.h"
#include "SpatialGDKFunctionalTests/SpatialGDK/TestActors/TestPossessionPawn.h"
#include "TestPossessionPlayerController.h"

/**
 * This test tests 1 locked Controller remote possess over 1 pawn.
 *
 * This test expects a load balancing grid and ACrossServerPossessionGameMode
 * Recommend to use 2*2 load balancing grid because the position of Pawn was written in the code
 * The client workers begin with a player controller and their default pawns, which they initially possess.
 * The flow is as follows:
 *  Recommend to use LockedControllerPossessPawnGym.umap in UnrealGDKTestGyms project which ready for tests.
 *  - Setup:
 *    - Specify `GameMode Override` as ACrossServerPossessionGameMode
 *    - Specify `Multi Worker Settings Class` as Zoning 2x2(e.g. BP_Possession_Settings_Zoning2_2 of UnrealGDKTestGyms)
 *	  - Set `Num Required Clients` as 1
 *  - Test:
 *	  - Create a Pawn in first quadrant
 *	  - Create Controller in other quadrant
 *	  - Wait for Pawn in right worker.
 *	  -	Lock the Controller and possess the Pawn
 *	- Result Check:
 *    - Pawn didn't have a Controller
 */

ACrossServerPossessionLockTest::ACrossServerPossessionLockTest()
	: Super()
{
	Author = "Jay";
	Description = TEXT("Test Locked Actor Cross-Server Possession");
}

void ACrossServerPossessionLockTest::PrepareTest()
{
	Super::PrepareTest();

	AddStep(TEXT("Controller remote possess"), FWorkerDefinition::AllClients, nullptr, nullptr, [this](float) {
		ATestPossessionPawn* Pawn = GetPawn();
		AssertIsValid(Pawn, TEXT("Test requires a Pawn"));
		for (ASpatialFunctionalTestFlowController* FlowController : GetFlowControllers())
		{
			if (FlowController->WorkerDefinition.Type == ESpatialFunctionalTestWorkerType::Client)
			{
				ATestPossessionPlayerController* PlayerController = Cast<ATestPossessionPlayerController>(FlowController->GetOwner());
				if (PlayerController != nullptr)
				{
					PlayerController->RemotePossessOnClient(Pawn, true);
				}
			}
		}
		FinishStep();
	});

	AddWaitStep(FWorkerDefinition::AllServers);

	AddStep(TEXT("Check test result"), FWorkerDefinition::AllServers, nullptr, nullptr, [this](float) {
		ATestPossessionPawn* Pawn = GetPawn();
		AssertIsValid(Pawn, TEXT("Test requires a Pawn"));
		AssertTrue(Pawn->GetController() == nullptr, TEXT("Pawn shouldn't have a controller"), Pawn);
		FinishStep();
	});

	AddStep(TEXT("Release locks on the pawns and player controllers"), FWorkerDefinition::AllServers, nullptr, nullptr, [this](float) {
		for (ASpatialFunctionalTestFlowController* FlowController : GetFlowControllers())
		{
			ATestPossessionPlayerController* PlayerController = Cast<ATestPossessionPlayerController>(FlowController->GetOwner());
			if (PlayerController != nullptr)
			{
				PlayerController->RemovePossessionComponent();
				PlayerController->UnlockAllTokens();
			}
		}
		FinishStep();
	});

	//Wait for playercontroller to get unlocked before trying to migrate it back in the cleanup steps
	AddWaitStep(FWorkerDefinition::AllServers);

	AddCleanupSteps();

}
