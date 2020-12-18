// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "CrossServerMultiPossessionTest.h"

#include "Containers/Array.h"
#include "EngineClasses/SpatialNetDriver.h"
#include "GameFramework/GameModeBase.h"
#include "GameFramework/PlayerController.h"
#include "GameMapsSettings.h"
#include "LoadBalancing/LayeredLBStrategy.h"
#include "SpatialFunctionalTestFlowController.h"
#include "SpatialGDKFunctionalTests/SpatialGDK/TestActors/TestMovementCharacter.h"
#include "TestPossessionPawn.h"
#include "TestPossessionPlayerController.h"
#include "Utils/SpatialStatics.h"

/**
 * This test tests multi clients possession over 1 pawns.
 *
 * The test includes 2x2 gridbase servers and at least two client workers.
 * The client workers begin with a player controller and their default pawns, which they initially possess.
 * The flow is as follows:
 *  - Setup:
 *    - Specify `GameMode Override` as ACrossServerPossessionGameMode
 *    - Specify `Multi Worker Settings Class` as Zoning 2x2(e.g. BP_Possession_Settings_Zoning2_2 of UnrealGDKTestGyms)
 *	  - Set `Num Required Clients` as 3
 *  - Test:
 *    - One of Controller possessed the Pawn and others failed
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
				ATestPossessionPlayerController* Controller = Cast<ATestPossessionPlayerController>(FlowController->GetOwner());
				AssertIsValid(Controller, TEXT("Test requires an ATestPossessionPlayerController"));
				Controller->RemotePossessOnClient(Pawn);
			}
		}
		FinishStep();
	});

	// Make sure all the workers can check the results
	AddWaitStep(FWorkerDefinition::AllServers);

	AddStep(TEXT("Check results on all servers"), FWorkerDefinition::AllServers, nullptr, nullptr, [this](float DeltaTime) {
		ATestPossessionPawn* Pawn = GetPawn();
		AssertIsValid(Pawn, TEXT("Test requires a Pawn"));

		AssertTrue(Pawn->GetController() != nullptr, TEXT("GetController of Pawn to check if possessed on server"), Pawn);

		AssertValue_Int(ATestPossessionPlayerController::OnPossessCalled, EComparisonMethod::Equal_To, 1,
						TEXT("OnPossess should be called 1 time"));
		FinishStep();
	});
}
