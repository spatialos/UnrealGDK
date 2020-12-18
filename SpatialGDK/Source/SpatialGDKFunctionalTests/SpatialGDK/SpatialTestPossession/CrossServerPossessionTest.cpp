// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "CrossServerPossessionTest.h"

#include "Containers/Array.h"
#include "EngineClasses/SpatialNetDriver.h"
#include "GameFramework/PlayerController.h"
#include "SpatialFunctionalTestFlowController.h"
#include "SpatialGDKFunctionalTests/SpatialGDK/TestActors/TestMovementCharacter.h"
#include "TestPossessionPawn.h"
#include "TestPossessionPlayerController.h"

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
		for (ASpatialFunctionalTestFlowController* FlowController : GetFlowControllers())
		{
			if (FlowController->WorkerDefinition.Type == ESpatialFunctionalTestWorkerType::Client)
			{
				ATestPossessionPlayerController* PlayerController = Cast<ATestPossessionPlayerController>(FlowController->GetOwner());
				if (PlayerController && PlayerController->HasAuthority())
				{
					AssertTrue(PlayerController->HasAuthority(), TEXT("PlayerController should HasAuthority"), PlayerController);
					AssertFalse(Pawn->HasAuthority(), TEXT("Pawn shouldn't HasAuthority"), Pawn);
					PlayerController->RemotePossessOnServer(Pawn);
				}
			}
		}
		FinishStep();
	});

	// Make sure all the workers can check the results
	AddWaitStep(FWorkerDefinition::AllServers);

	AddStep(TEXT("Check test result"), FWorkerDefinition::Server(1), nullptr, nullptr, [this](float) {
		ASpatialFunctionalTestFlowController* FlowController = GetFlowController(ESpatialFunctionalTestWorkerType::Client, 1);
		AssertIsValid(FlowController, TEXT("Test requires Client FlowController"));
		ATestPossessionPlayerController* PlayerController = Cast<ATestPossessionPlayerController>(FlowController->GetOwner());
		AssertIsValid(PlayerController, TEXT("Test requires an ATestPossessionPlayerController"));
		ATestPossessionPawn* Pawn = GetPawn();
		AssertIsValid(Pawn, TEXT("Test requires a Pawn"));
		AssertTrue(Pawn->Controller == PlayerController, TEXT("PlayerController has possessed the pawn"), PlayerController);
		FinishStep();
	});
}
