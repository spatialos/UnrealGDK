// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "CrossServerPossessionLockTest.h"
#include "EngineClasses/SpatialNetDriver.h"
#include "GameFramework/PlayerController.h"
#include "Net/UnrealNetwork.h"
#include "SpatialFunctionalTestFlowController.h"
#include "TestPossessionPawn.h"
#include "TestPossessionPlayerController.h"

/**
 * This test tests client possession over pawns in other zones.
 *
 * The test includes two servers and two client workers. The client workers begin with a player controller and their default pawns,
 * which they initially possess. The flow is as follows:
 *  - Setup:
 *    - Two test pawn actors are spawned, one for each client, with an offset in the y direction for easy visualisation
 *    - The controllers for each client possess the spawned test pawn actors
 *  - Test:
 *    - The clients assert that the pawn they currently possess are test pawns
 *  - Cleanup:
 *    - The clients repossess their default pawns
 *    - The test pawns are destroyed
 */

ACrossServerPossessionLockTest::ACrossServerPossessionLockTest()
	: Super()
{
	Author = "Jay";
	Description = TEXT("Test Actor Remote Possession");
}

void ACrossServerPossessionLockTest::PrepareTest()
{
	Super::PrepareTest();

	AddStep(TEXT("Controller remote possess"), FWorkerDefinition::AllClients, nullptr, nullptr, [this](float DeltaTime) {
		ATestPossessionPawn* Pawn = GetPawn();
		AssertIsValid(Pawn, TEXT("Test requires a Pawn"));
		for (ASpatialFunctionalTestFlowController* FlowController : GetFlowControllers())
		{
			if (FlowController->WorkerDefinition.Type == ESpatialFunctionalTestWorkerType::Client)
			{
				ATestPossessionPlayerController* Controller = Cast<ATestPossessionPlayerController>(FlowController->GetOwner());
				AssertIsValid(Controller, TEXT("PlayerController not possessed the pawn"));
				Controller->RemotePossessOnClient(Pawn);
			}
		}
		FinishStep();
	});

	AddWaitStep(FWorkerDefinition::AllServers);

	AddStep(TEXT("Check test result"), FWorkerDefinition::Server(1), nullptr, nullptr, [this](float) {
		ATestPossessionPawn* Pawn = GetPawn();
		AssertIsValid(Pawn, TEXT("Test requires a Pawn"));
		for (ASpatialFunctionalTestFlowController* FlowController : GetFlowControllers())
		{
			if (FlowController->WorkerDefinition.Type == ESpatialFunctionalTestWorkerType::Server)
			{
				AssertTrue(Pawn->Controller == nullptr, TEXT("PlayerController not possessed the pawn"), Pawn);
			}
		}
		FinishStep();
	});
}
