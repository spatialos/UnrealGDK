// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "CrossServerMultiPossessionTest.h"

#include "Kismet/GameplayStatics.h"
#include "SpatialFunctionalTestFlowController.h"
#include "TestPossessionController.h"
#include "TestPossessionPawn.h"
#include "Utils/SpatialStatics.h"

/**
 * This test tests multi Controllers remote possess over 1 Pawn.
 *
 * The flow is as follows:
 *	Recommend to use PossessionGym.umap in UnrealGDKTestGyms project which ready for tests.
 *  - Setup:
 *    - Specify `Multi Worker Settings Class` as Zoning 2x2(e.g. BP_Possession_Settings_Zoning2_2 of UnrealGDKTestGyms)
 *	  - Set `Num Required Clients` as 1
 *  - Test:
 *	  - Create a Pawn in first quadrant
 *	  - Create 3 Controllers in other quadrant
 *	  - Wait for Pawn and Controllers in right worker.
 *	  -	The Controller possess the Pawn
 *	- Result Check:
 *    - ATestPossessionController::OnPossess should be called 3 times
 *	  - Controllers should have migrated
 */

ACrossServerMultiPossessionTest::ACrossServerMultiPossessionTest()
	: Super()
{
	Author = "Ken.Yu";
	Description = TEXT("Test Cross-Server 3 Controllers Possess 1 Pawn");
}

void ACrossServerMultiPossessionTest::PrepareTest()
{
	ASpatialTestRemotePossession::PrepareTest();

	AddStep(TEXT("Controller remote possess"), FWorkerDefinition::Server(1), nullptr, nullptr, [this](float DeltaTime) {
		ATestPossessionPawn* Pawn = GetPawn();
		AssertIsValid(Pawn, TEXT("Test requires a Pawn"));

		if (ASpatialFunctionalTestFlowController* FlowController = GetLocalFlowController())
		{
			TArray<AActor*> OutActors;
			UGameplayStatics::GetAllActorsOfClass(GetWorld(), ATestPossessionController::StaticClass(), OutActors);
			for (AActor* Actor : OutActors)
			{
				ATestPossessionController* Controller = Cast<ATestPossessionController>(Actor);
				if (Controller != nullptr)
				{
					Controller->RemotePossess(Pawn);
				}
			}
		}
		FinishStep();
	});

	AddStep(
		TEXT("Check results on all servers"), FWorkerDefinition::AllServers,
		[this]() -> bool {
			LogStep(ELogVerbosity::Log, FString::Printf(TEXT("OnPossessCalled:%d"), ATestPossessionController::OnPossessCalled));
			return ATestPossessionController::OnPossessCalled == 3;
		},
		nullptr,
		[this](float DeltaTime) {
			if (ASpatialFunctionalTestFlowController* FlowController = GetLocalFlowController())
			{
				ATestPossessionController* Controller = GetController();
				if (Controller && Controller->HasAuthority())
				{
					AssertTrue(Controller->HasMigrated(), TEXT("Controller should have migrated"), Controller);
				}
				FinishStep();
			}
		});

	AddCleanStep();
}

void ACrossServerMultiPossessionTest::CreateControllerAndPawn()
{
	CreateController(FVector(-500.0f, -500.0f, 50.0f));
	CreateController(FVector(500.0f, -500.0f, 50.0f));
	CreateController(FVector(-500.0f, 500.0f, 50.0f));

	CreatePawn(FVector(500.0f, 500.0f, 50.0f));
}
