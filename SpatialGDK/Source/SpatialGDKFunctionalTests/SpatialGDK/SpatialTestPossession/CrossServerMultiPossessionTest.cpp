// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "CrossServerMultiPossessionTest.h"

#include "Containers/Array.h"
#include "EngineClasses/SpatialNetDriver.h"
#include "GameFramework/GameModeBase.h"
#include "GameFramework/PlayerController.h"
#include "GameMapsSettings.h"
#include "SpatialFunctionalTestFlowController.h"
#include "SpatialGDKFunctionalTests/SpatialGDK/TestActors/TestMovementCharacter.h"
#include "SpatialGDKFunctionalTests/SpatialGDK/TestActors/TestPossessionPawn.h"
#include "TestPossessionPlayerController.h"
#include "Utils/SpatialStatics.h"

/**
 * This test tests multi Controllers remote possess over 1 Pawn.
 *
 * This test expects a 2x2 load balancing grid and ACrossServerPossessionGameMode
 * The client workers begin with a player controller and their default pawns, which they initially possess.
 * The flow is as follows:
 *	Recommend to use MultiControllerPossessPawnGym.umap in UnrealGDKTestGyms project which ready for tests.
 *  - Setup:
 *    - Specify `GameMode Override` as ACrossServerPossessionGameMode
 *    - Specify `Multi Worker Settings Class` as Zoning 2x2(e.g. BP_Possession_Settings_Zoning2_2 of UnrealGDKTestGyms)
 *	  - Set `Num Required Clients` as 2 or more
 *  - Test:
 *	  - Create a Pawn in first quadrant
 *	  - Create Controllers in other quadrant, the position is determined by ACrossServerPossessionGameMode
 *	  - Wait for Pawn and Controllers in right worker.
 *	  -	The Controller possess the Pawn
 *	- Result Check:
 *    - ATestPossessionPlayerController::OnPossess should be called `Num Required Clients` times
 */

ACrossServerMultiPossessionTest::ACrossServerMultiPossessionTest()
	: Super()
{
	Author = "Ken.Yu";
	Description = TEXT("Test Cross-Server Multi Controllers Possess 1 Pawn");
}

void ACrossServerMultiPossessionTest::PrepareTest()
{
	ASpatialTestRemotePossession::PrepareTest();

	AddStep(TEXT("Controller remote possess"), FWorkerDefinition::AllClients, nullptr, nullptr, [this](float DeltaTime) {
		ATestPossessionPawn* Pawn = GetPawn();
		AssertIsValid(Pawn, TEXT("Test requires a Pawn"));
		for (ASpatialFunctionalTestFlowController* FlowController : GetFlowControllers())
		{
			if (FlowController->WorkerDefinition.Type == ESpatialFunctionalTestWorkerType::Client)
			{
				ATestPossessionPlayerController* PlayerController = Cast<ATestPossessionPlayerController>(FlowController->GetOwner());
				if (PlayerController != nullptr)
				{
					AddToOriginalPawns(PlayerController, PlayerController->GetPawn());
					PlayerController->RemotePossessOnClient(Pawn, false);
				}
			}
		}
		FinishStep();
	});

	AddStep(
		TEXT("Check results on all servers"), FWorkerDefinition::AllServers,
		[this]() -> bool {
			return ATestPossessionPlayerController::OnPossessCalled == GetNumRequiredClients();
		},
		nullptr,
		[this](float DeltaTime) {
			for (ASpatialFunctionalTestFlowController* FlowController : GetFlowControllers())
			{
				if (FlowController->WorkerDefinition.Type == ESpatialFunctionalTestWorkerType::Client)
				{
					ATestPossessionPlayerController* PlayerController = Cast<ATestPossessionPlayerController>(FlowController->GetOwner());
					if (PlayerController && PlayerController->HasAuthority())
					{
						AssertTrue(PlayerController->HasMigrated(), TEXT("PlayerController should have migrated"), PlayerController);
					}
				}
			}
			FinishStep();
		});

	AddStep(TEXT("Clean up the test"), FWorkerDefinition::AllServers, nullptr, nullptr, [this](float) {
		for (const auto& OriginalPawnPair : OriginalPawns)
		{
			if (OriginalPawnPair.Controller.Get() != nullptr && OriginalPawnPair.Controller.Get()->HasAuthority())
			{
				OriginalPawnPair.Controller.Get()->UnPossess();
				OriginalPawnPair.Controller.Get()->RemotePossessOnServer(OriginalPawnPair.Pawn.Get());
			}
		}
		FinishStep();
	});

	AddStep(TEXT("Wait for all controllers to migrate"), FWorkerDefinition::AllServers, nullptr, nullptr, [this](float) {
		for (const auto& OriginalPawnPair : OriginalPawns)
		{
			if (OriginalPawnPair.Controller.Get() != nullptr && OriginalPawnPair.Controller.Get()->HasAuthority())
			{
				RequireTrue(OriginalPawnPair.Pawn.Get()->HasAuthority(),
							TEXT("We should have authority over both original pawn and player controller on their initial server"));
				RequireTrue(OriginalPawnPair.Controller.Get()->GetPawn() == OriginalPawnPair.Pawn.Get(),
							TEXT("The player controller should have possession over its original pawn"));
			}
		}
		FinishStep();
	});
}
