// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "SpatialTestRemotePossession.h"

#include "Kismet/GameplayStatics.h"
#include "Net/UnrealNetwork.h"
#include "SpatialFunctionalTestFlowController.h"
#include "SpatialGDKFunctionalTests/SpatialGDK/TestActors/TestPossessionPawn.h"
#include "TestPossessionPlayerController.h"

const float ASpatialTestRemotePossession::MaxWaitTime = 2.0f;

ASpatialTestRemotePossession::ASpatialTestRemotePossession()
	: Super()
	, LocationOfPawn(500.0f, 500.0f, 50.0f)
{
	Author = "Jay";
	Description = TEXT("Test Actor Remote Possession");
}

ATestPossessionPawn* ASpatialTestRemotePossession::GetPawn()
{
	return Cast<ATestPossessionPawn>(UGameplayStatics::GetActorOfClass(GetWorld(), ATestPossessionPawn::StaticClass()));
}

void ASpatialTestRemotePossession::PrepareTest()
{
	Super::PrepareTest();

	AddStep(TEXT("EnsureSpatialOS"), FWorkerDefinition::AllServers, nullptr, /*StartEvent*/ [this]() {
		ULayeredLBStrategy* LoadBalanceStrategy = GetLoadBalancingStrategy();
		AssertTrue(LoadBalanceStrategy != nullptr, TEXT("Test requires SpatialOS enabled with Load-Balancing Strategy"));
		FinishStep();
	});

	AddStep(TEXT("Create Pawn"), FWorkerDefinition::Server(1), nullptr, /*StartEvent*/ [this]() {
		ATestPossessionPawn* Pawn =
			GetWorld()->SpawnActor<ATestPossessionPawn>(LocationOfPawn, FRotator::ZeroRotator, FActorSpawnParameters());
		RegisterAutoDestroyActor(Pawn);
		FinishStep();
	});

	AddStep(TEXT("Clear Original Pawn Array"), FWorkerDefinition::AllServers, nullptr, /*StartEvent*/ [this]() {
		OriginalPawns.Empty();
		FinishStep();
	});

	// Ensure that all Controllers are located on the right Worker
	AddWaitStep(FWorkerDefinition::AllServers);

	AddStep(TEXT("Save Original Pawns"), FWorkerDefinition::AllServers, nullptr, /*StartEvent*/ [this]() {
		for (ASpatialFunctionalTestFlowController* FlowController : GetFlowControllers())
		{
			ATestPossessionPlayerController* PlayerController = Cast<ATestPossessionPlayerController>(FlowController->GetOwner());
			if (PlayerController != nullptr && PlayerController->HasAuthority())
			{
				if (PlayerController->GetPawn() != nullptr && PlayerController->GetPawn()->HasAuthority())
				{
					AddToOriginalPawns(PlayerController, PlayerController->GetPawn());
				}
			}
		}
		FinishStep();
	});

	ATestPossessionPlayerController::ResetCalledCounter();
}

void ASpatialTestRemotePossession::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION(ThisClass, OriginalPawns, COND_ServerOnly);
}

void ASpatialTestRemotePossession::AddCleanupSteps()
{
	AddStep(TEXT("Clean up the test"), FWorkerDefinition::AllServers, nullptr, /*StartEvent*/ [this]() {
		for (const auto& OriginalPawnPair : OriginalPawns)
		{
			if (OriginalPawnPair.PlayerController != nullptr && OriginalPawnPair.PlayerController->HasAuthority())
			{
				OriginalPawnPair.PlayerController->UnPossess();
				OriginalPawnPair.PlayerController->RemotePossessOnServer(OriginalPawnPair.Pawn);
			}
		}
		FinishStep();
	});

	AddStep(TEXT("Wait for all controllers to migrate back"), FWorkerDefinition::AllServers, nullptr, nullptr,
			/*TickEvent*/ [this](float DeltaTime) {
				for (const auto& OriginalPawnPair : OriginalPawns)
				{
					if (AssertIsValid(OriginalPawnPair.PlayerController,
									  TEXT("We should be able to see all player controllers from any server")))
					{
						RequireTrue(OriginalPawnPair.PlayerController->GetPawn() == OriginalPawnPair.Pawn,
									FString::Printf(TEXT("The player controller should have possession over its original pawn %s"),
													*OriginalPawnPair.Pawn->GetName()));
					}
				}
				FinishStep();
			});
}

bool ASpatialTestRemotePossession::IsReadyForPossess()
{
	ATestPossessionPawn* Pawn = GetPawn();
	return Pawn->Controller != nullptr;
}

void ASpatialTestRemotePossession::AddToOriginalPawns_Implementation(ATestPossessionPlayerController* PlayerController, APawn* Pawn)
{
	FControllerPawnPair OriginalPair;
	OriginalPair.PlayerController = PlayerController;
	OriginalPair.Pawn = Pawn;
	OriginalPawns.Add(OriginalPair);
}

void ASpatialTestRemotePossession::AddWaitStep(const FWorkerDefinition& Worker)
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
