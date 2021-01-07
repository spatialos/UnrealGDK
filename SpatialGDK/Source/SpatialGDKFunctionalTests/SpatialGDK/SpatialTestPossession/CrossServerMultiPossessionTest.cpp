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
 * This test tests multi Controllers possess over 1 Pawn.
 *
 * This test expects a 2x2 load balancing grid and ACrossServerPossessionGameMode
 * The client workers begin with a player controller and their default pawns, which they initially possess.
 * The flow is as follows:
 *	Recommend to use PossessionGym.umap in UnrealGDKTestGyms project which ready for tests.
 *  - Setup:
 *    - Specify `GameMode Override` as ACrossServerPossessionGameMode
 *    - Specify `Multi Worker Settings Class` as Zoning 2x2(e.g. BP_Possession_Settings_Zoning2_2 of UnrealGDKTestGyms)
 *	  - Set `Num Required Clients` as 2 or more
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

	AddStep(TEXT("Controller remote possess"), FWorkerDefinition::AllClients, nullptr,nullptr,
			[this](float DeltaTime) {
		ATestPossessionPawn* Pawn = GetPawn();
		AssertIsValid(Pawn, TEXT("Test requires a Pawn"));
		for (ASpatialFunctionalTestFlowController* FlowController : GetFlowControllers())
		{
			if (FlowController->WorkerDefinition.Type == ESpatialFunctionalTestWorkerType::Client)
			{
				ATestPossessionPlayerController* Controller = Cast<ATestPossessionPlayerController>(FlowController->GetOwner());
				if (Controller != nullptr)
				{
					Controller->RemotePossessOnClient(Pawn, false);
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
		FinishStep();
	});
}
