// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "CrossServerPlayerControllerPossessPawn.h"

#include "Kismet/GameplayStatics.h"
#include "SpatialFunctionalTestFlowController.h"
#include "TestPossessionPlayerController.h"
#include "TestPossessionPawn.h"

/**
 * This test tests 1 Controller remote possess over 1 Pawn.
 *
 * The client workers begin with a player controller and their default pawns, which they initially possess.
 * The flow is as follows:
 *	Recommend to use PossessionGym.umap in UnrealGDKTestGyms project which ready for tests.
 *  - Setup:
 *    - Specify `Multi Worker Settings Class` as Zoning 2x2(e.g. BP_Possession_Settings_Zoning2_2 of UnrealGDKTestGyms)
 *	  - Set `Num Required Clients` as 1
 *  - Test:
 *	  - Create a Pawn in first quadrant
 *	  - Start a PlayerController in 3rd quadrant
 *	  - Wait for PlayerController and Pawn in right worker.
 *	  -	The PlayerController possess the Pawn in server-side
 *	- Result Check:
 *    - ATestPossessionPlayerController::OnPossess should be called 1 time
 *	  - PlayerController should have migrated
 */

const float ACrossServerPlayerControllerPossessPawn::MaxWaitTime = 2.0f;

ACrossServerPlayerControllerPossessPawn::ACrossServerPlayerControllerPossessPawn()
{
	Author = "Ken.Yu";
	Description = TEXT("Test PlayerController Cross-Server Possess the Pawn");
}

void ACrossServerPlayerControllerPossessPawn::PrepareTest()
{
	Super::PrepareTest();

	AddStep(TEXT("EnsureSpatialOS"), FWorkerDefinition::AllServers, nullptr, [this]() {
		ULayeredLBStrategy* LoadBalanceStrategy = GetLoadBalancingStrategy();
		AssertTrue(LoadBalanceStrategy != nullptr, TEXT("Test requires SpatialOS enabled with Load-Balancing Strategy"));
		FinishStep();
	});

	AddStep(TEXT("Create Pawn"), FWorkerDefinition::Server(1), nullptr, nullptr, [this](float DeltaTime) {
		ATestPossessionPawn* Pawn =
			GetWorld()->SpawnActor<ATestPossessionPawn>(FVector(500.0f, 500.0f, 50.0f), FRotator::ZeroRotator, FActorSpawnParameters());
		RegisterAutoDestroyActor(Pawn);
		FinishStep();
	});

	// Ensure that the Controller is located on the right Worker
	AddWaitStep(FWorkerDefinition::AllServers);

	ATestPossessionPlayerController::OnPossessCalled = 0;

	AddStep(TEXT("Cross-Server Possession"), FWorkerDefinition::AllServers, nullptr, nullptr, [this](float) {
		ATestPossessionPawn* Pawn = GetPawn();
		AssertIsValid(Pawn, TEXT("Pawn shouldn't have authority"));
		for (ASpatialFunctionalTestFlowController* FlowController : GetFlowControllers())
		{
			if (FlowController->WorkerDefinition.Type == ESpatialFunctionalTestWorkerType::Client)
			{
				ATestPossessionPlayerController* PlayerController = Cast<ATestPossessionPlayerController>(FlowController->GetOwner());
				if (PlayerController && PlayerController->HasAuthority())
				{
					AssertFalse(Pawn->HasAuthority(), TEXT("Pawn shouldn't have authority"), Pawn);

					// Clean the Default Pawn. It wasn't set as replicate before.
					// Will hit a warning in GetPawn()->SetReplicates(true) of APlayerController::OnUnPossess() if without UnPossess here.
					PlayerController->UnPossess();

					PlayerController->RemotePossessOnServer(Pawn);
				}
			}
		}
		FinishStep();
	});

	AddStep(
		TEXT("Check test result"), FWorkerDefinition::AllServers,
		[this]() -> bool {
			LogStep(ELogVerbosity::Log, FString::Printf(TEXT("OnPossessCalled:%d"), ATestPossessionPlayerController::OnPossessCalled));
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
						AssertTrue(PlayerController->HasMigrated(), TEXT("PlayerController should have migrated"), PlayerController);

						// Cleanup
						PlayerController->UnPossess();
					}
				}
			}
			FinishStep();
		});

	AddStep(
		TEXT("Clean"), FWorkerDefinition::AllServers, nullptr, nullptr,
		[this](float) {
			if (ASpatialFunctionalTestFlowController* FlowController = GetLocalFlowController())
			{
				ATestPossessionPawn* Pawn = GetPawn();
				if (Pawn != nullptr && Pawn->HasAuthority())
				{
					GetWorld()->DestroyActor(Pawn);
				}
			}
			FinishStep();
		});
}

ATestPossessionPawn* ACrossServerPlayerControllerPossessPawn::GetPawn()
{
	return Cast<ATestPossessionPawn>(UGameplayStatics::GetActorOfClass(GetWorld(), ATestPossessionPawn::StaticClass()));
}

void ACrossServerPlayerControllerPossessPawn::AddWaitStep(const FWorkerDefinition& Worker)
{
	AddStep(TEXT("Wait"), Worker, nullptr, nullptr, [this](float DeltaTime) {
		if (WaitTime > MaxWaitTime)
		{
			WaitTime = 0;
			FinishStep();
		}
		WaitTime += DeltaTime;
	});
}
