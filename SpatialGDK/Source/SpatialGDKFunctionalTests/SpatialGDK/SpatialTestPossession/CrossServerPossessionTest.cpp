// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "CrossServerPossessionTest.h"

#include "SpatialFunctionalTestFlowController.h"
#include "TestPossessionController.h"
#include "TestPossessionPawn.h"

/**
 * This test tests 1 Controller remote possess over 1 Pawn.
 *
 * Recommand to use 2*2 load balancing grid because the position is written in the code
 * The flow is as follows:
 *	Recommend to use PossessionGym.umap in UnrealGDKTestGyms project which ready for tests.
 *  - Setup:
 *    - Specify `Multi Worker Settings Class` as Zoning 2x2(e.g. BP_Possession_Settings_Zoning2_2 of UnrealGDKTestGyms)
 *	  - Set `Num Required Clients` as 1
 *  - Test:
 *	  - Create Controller in 3rd quadrant
 *	  - Create a Pawn in first quadrant
 *	  - Wait for Controller and Pawn in right worker.
 *	  -	The Controller possess the Pawn in server-side
 *	- Result Check:
 *    - ATestPossessionController::OnPossess should be called 1 time
 *	  - Controller should have migrated
 */

ACrossServerPossessionTest::ACrossServerPossessionTest()
{
	Author = "Ken.Yu";
	Description = TEXT("Test Cross-Server Possession");
}

void ACrossServerPossessionTest::PrepareTest()
{
	Super::PrepareTest();

	AddStep(TEXT("Cross-Server Possession"), FWorkerDefinition::AllServers, nullptr, nullptr, [this](float) {
		ATestPossessionPawn* Pawn = GetPawn();
		AssertIsValid(Pawn, TEXT("Test requires a Pawn"));
		if (ASpatialFunctionalTestFlowController* FlowController = GetLocalFlowController())
		{
			ATestPossessionController* Controller = GetController();
			if (Controller && Controller->HasAuthority())
			{
				AssertFalse(Pawn->HasAuthority(), TEXT("Pawn shouldn't have authority"), Pawn);
				Controller->RemotePossessOnServer(Pawn);
			}
		}
		FinishStep();
	});

	AddStep(
		TEXT("Check test result"), FWorkerDefinition::AllServers,
		[this]() -> bool {
			LogStep(ELogVerbosity::Log, FString::Printf(TEXT("OnPossessCalled:%d"), ATestPossessionController::OnPossessCalled));
			return ATestPossessionController::OnPossessCalled == 1;
		},
		nullptr,
		[this](float) {
			if (ASpatialFunctionalTestFlowController* FlowController = GetLocalFlowController())
			{
				ATestPossessionController* Controller = GetController();
				if (Controller && Controller->HasAuthority())
				{
					AssertTrue(Controller->HasMigrated(), TEXT("Controller should have migrated"), Controller);
				}
			}
			FinishStep();
		});

	AddCleanStep();
}

void ACrossServerPossessionTest::CreateControllerAndPawn()
{
	CreateController(FVector(-500.0f, -500.0f, 50.0f));
	CreatePawn(FVector(500.0f, 500.0f, 50.0f));
}
