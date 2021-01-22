// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "CrossServerPossessionLockTest.h"
#include "EngineClasses/SpatialNetDriver.h"
#include "GameFramework/PlayerController.h"
#include "Net/UnrealNetwork.h"
#include "SpatialFunctionalTestFlowController.h"
#include "TestPossessionController.h"
#include "TestPossessionPawn.h"

/**
 * This test tests 1 locked Controller remote possess over 1 pawn.
 *
 * This test expects a load balancing grid and ACrossServerPossessionGameMode
 * Recommend to use 2*2 load balancing grid because the position of Pawn was written in the code
 * The client workers begin with a player controller and their default pawns, which they initially possess.
 * The flow is as follows:
 *  - Setup:
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

	AddStep(TEXT("Locked Controller remote possess"), FWorkerDefinition::AllServers, nullptr, nullptr, [this](float DeltaTime) {
		ATestPossessionPawn* Pawn = GetPawn();
		AssertIsValid(Pawn, TEXT("Test requires a Pawn"));
		if (ASpatialFunctionalTestFlowController* FlowController = GetLocalFlowController())
		{
			ATestPossessionController* Controller = GetController();
			if (Controller && Controller->HasAuthority())
			{
				Controller->RemotePossessOnServer(Pawn, true);
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

	AddCleanStep();
}

void ACrossServerPossessionLockTest::CreateControllerAndPawn()
{
	CreateController(FVector(-500.0f, -500.0f, 50.0f));
	CreatePawn(FVector(500.0f, 500.0f, 50.0f));
}
