// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "NoneCrossServerPossessionTest.h"

#include "SpatialFunctionalTestFlowController.h"
#include "TestPossessionController.h"
#include "TestPossessionPawn.h"

/**
 * This test tests 1 Controller possess over 1 Pawn.
 *
 * This test expects a load balancing grid and ACrossServerPossessionGameMode
 * Recommend to use 2*2 load balancing grid because the position is written in the code
 * The client workers begin with a player controller and their default pawns, which they initially possess.
 * The flow is as follows:
 *	Recommend to use PossessionGym.umap in UnrealGDKTestGyms project which ready for tests.
 *  - Setup:
 *    - Specify `Multi Worker Settings Class` as Zoning 2x2(e.g. BP_Possession_Settings_Zoning2_2 of UnrealGDKTestGyms)
 *	  - Set `Num Required Clients` as 1
 *  - Test:
 *	  - Create a Pawn in 4th quadrant
 *	  - Create 1 Controller in 4th quadrant
 *	  - Wait for Pawn in right worker.
 *	  -	The Controller possess the Pawn in server-side
 *	- Result Check:
 *    - ATestPossessionController::OnPossess should be called 1 time
 *	  - Controller should not migration
 */

ANoneCrossServerPossessionTest::ANoneCrossServerPossessionTest()
{
	Author = "Ken.Yu";
	Description = TEXT("Test Local Possession via RemotePossessionComponent");
}

void ANoneCrossServerPossessionTest::PrepareTest()
{
	Super::PrepareTest();

	AddStep(TEXT("Possession"), FWorkerDefinition::AllServers, nullptr, nullptr, [this](float) {
		ATestPossessionPawn* Pawn = GetPawn();
		AssertIsValid(Pawn, TEXT("Test requires a Pawn"));
		if (ASpatialFunctionalTestFlowController* FlowController = GetLocalFlowController())
		{
			ATestPossessionController* Controller = GetController();
			if (Controller && Controller->HasAuthority())
			{
				AssertTrue(Controller->HasAuthority(), TEXT("Controller should HasAuthority"), Controller);
				AssertTrue(Pawn->HasAuthority(), TEXT("Pawn should HasAuthority"), Pawn);
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
					AssertFalse(Controller->HasMigrated(), TEXT("Controller shouldn't have migrated"), Controller);
				}
				else
				{
					LogStep(ELogVerbosity::Log, FString::Printf(TEXT("Controller:%s has not authority"), *Controller->GetName()));
				}
			}
			FinishStep();
		});

	AddCleanStep();
}

void ANoneCrossServerPossessionTest::CreateControllerAndPawn()
{
	CreateController(FVector(-500.0f, -500.0f, 50.0f));
	CreatePawn(FVector(-500.0f, -500.0f, 50.0f));
}
