// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "NoneCrossServerPossessionTest.h"

#include "Containers/Array.h"
#include "EngineClasses/SpatialNetDriver.h"
#include "GameFramework/PlayerController.h"
#include "SpatialFunctionalTestFlowController.h"
#include "TestPossessionPawn.h"
#include "TestPossessionPlayerController.h"

/**
 * This test tests 1 Controller possess over 1 Pawn.
 *
 * This test expects a load balancing grid and ACrossServerPossessionGameMode
 * Recommend to use 2*2 load balancing grid because the position is written in the code
 * The client workers begin with a player controller and their default pawns, which they initially possess.
 * The flow is as follows:
 *	Recommend to use LocalControllerPossessPawnGym.umap in UnrealGDKTestGyms project which ready for tests.
 *  - Setup:
 *    - Specify `GameMode Override` as ACrossServerPossessionGameMode
 *    - Specify `Multi Worker Settings Class` as Zoning 2x2(e.g. BP_Possession_Settings_Zoning2_2 of UnrealGDKTestGyms)
 *	  - Set `Num Required Clients` as 1
 *  - Test:
 *	  - Create a Pawn in 4th quadrant
 *	  - Create 1 Controller in 4th quadrant, the position is determined by ACrossServerPossessionGameMode
 *	  - Wait for Pawn in right worker.
 *	  -	The Controller possess the Pawn in server-side
 *	- Result Check:
 *    - ATestPossessionPlayerController::OnPossess should be called == 1 times
 */

ANoneCrossServerPossessionTest::ANoneCrossServerPossessionTest()
{
	Author = "Ken.Yu";
	Description = TEXT("Test Local Possession via RemotePossessionComponent");
	LocationOfPawn = FVector(-500.0f, -500.0f, 50.0f);

	SetNumRequiredClients(1);
}

void ANoneCrossServerPossessionTest::PrepareTest()
{
	Super::PrepareTest();

	AddStep(TEXT("Possession"), FWorkerDefinition::AllServers, nullptr, nullptr, [this](float) {
		ATestPossessionPawn* Pawn = GetPawn();
		AssertIsValid(Pawn, TEXT("Test requires a Pawn"));
		for (ASpatialFunctionalTestFlowController* FlowController : GetFlowControllers())
		{
			if (FlowController->WorkerDefinition.Type == ESpatialFunctionalTestWorkerType::Client)
			{
				ATestPossessionPlayerController* PlayerController = Cast<ATestPossessionPlayerController>(FlowController->GetOwner());
				if (PlayerController && PlayerController->HasAuthority())
				{
					AssertTrue(PlayerController->HasAuthority(), TEXT("PlayerController should HasAuthority"), PlayerController);
					AssertTrue(Pawn->HasAuthority(), TEXT("Pawn should HasAuthority"), Pawn);
					PlayerController->UnPossess();
					PlayerController->RemotePossessOnServer(Pawn);
				}
			}
		}
		FinishStep();
	});

	AddStep(
		TEXT("Check test result"), FWorkerDefinition::Server(1),
		[this]() -> bool {
			return ATestPossessionPlayerController::OnPossessCalled == 1;
		},
		nullptr,
		[this](float) {
			for (ASpatialFunctionalTestFlowController* FlowController : GetFlowControllers())
			{
				if (FlowController->WorkerDefinition.Type == ESpatialFunctionalTestWorkerType::Client)
				{
					ATestPossessionPlayerController* PlayerController = Cast<ATestPossessionPlayerController>(FlowController->GetOwner());
					if (PlayerController && PlayerController->HasAuthority())
					{
						AssertFalse(PlayerController->HasMigrated(), TEXT("PlayerController shouldn't have migrated"), PlayerController);
					}
				}
			}
			FinishStep();
		});
}
