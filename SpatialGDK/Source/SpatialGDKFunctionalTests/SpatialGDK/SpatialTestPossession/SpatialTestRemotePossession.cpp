// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "SpatialTestRemotePossession.h"

#include "Kismet/GameplayStatics.h"
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

	AddStep(TEXT("EnsureSpatialOS"), FWorkerDefinition::AllServers, nullptr, [this]() {
		ULayeredLBStrategy* LoadBalanceStrategy = GetLoadBalancingStrategy();
		AssertTrue(LoadBalanceStrategy != nullptr, TEXT("Test requires SpatialOS enabled with Load-Balancing Strategy"));
		FinishStep();
	});

	AddStep(TEXT("Create Pawn"), FWorkerDefinition::Server(1), nullptr, nullptr, [this](float DeltaTime) {
		ATestPossessionPawn* Pawn =
			GetWorld()->SpawnActor<ATestPossessionPawn>(LocationOfPawn, FRotator::ZeroRotator, FActorSpawnParameters());
		RegisterAutoDestroyActor(Pawn);
		FinishStep();
	});

	AddStep(TEXT("Clear Original Pawn Array"), FWorkerDefinition::AllServers, nullptr, [this]() {
		OriginalPawns.Empty();
		FinishStep();
	});

	// Ensure that all Controllers are located on the right Worker
	AddWaitStep(FWorkerDefinition::AllServers);

	AddStep(TEXT("Save Original Pawns"), FWorkerDefinition::AllServers, nullptr, [this]() {
		for (ASpatialFunctionalTestFlowController* FlowController : GetFlowControllers())
		{
			ATestPossessionPlayerController* PlayerController = Cast<ATestPossessionPlayerController>(FlowController->GetOwner());
			if (PlayerController != nullptr && PlayerController->HasAuthority() && PlayerController->GetPawn()->HasAuthority())
			{
				AddToOriginalPawns(PlayerController, PlayerController->GetPawn());
				UE_LOG(LogTemp, Log, TEXT("Adding original pawn %s to array"), *PlayerController->GetPawn()->GetName());
			}
		}
		FinishStep();
	});

	ATestPossessionPlayerController::ResetCalledCounter();
}

void ASpatialTestRemotePossession::AddCleanupSteps() {
	AddStep(TEXT("Clean up the test"), FWorkerDefinition::AllServers, nullptr, nullptr, [this](float) {
		for (const auto& OriginalPawnPair : OriginalPawns)
		{
			if (OriginalPawnPair.Controller != nullptr && OriginalPawnPair.Controller->HasAuthority())
			{
				OriginalPawnPair.Controller->UnPossess();
				OriginalPawnPair.Controller->RemotePossessOnServer(OriginalPawnPair.Pawn);
				AssertIsValid(OriginalPawnPair.Pawn, TEXT("Original pawn shouldn't be null"));
			}
		}
		FinishStep();
	});

	AddStep(TEXT("Wait for all controllers to migrate back"), FWorkerDefinition::AllServers, nullptr, nullptr, [this](float) {
		for (const auto& OriginalPawnPair : OriginalPawns)
		{
			//if(AssertIsValid(OriginalPawnPair.Controller,TEXT("We should be able to see all player controllers from any server")))
			if (OriginalPawnPair.Controller)
			{
				RequireTrue(OriginalPawnPair.Controller->GetPawn() == OriginalPawnPair.Pawn,
							FString::Printf(TEXT("The player controller should have possession over its original pawn %s"),
											*OriginalPawnPair.Pawn->GetName()));
				if (OriginalPawnPair.Controller->GetPawn() == OriginalPawnPair.Pawn && OriginalPawnPair.Controller->HasAuthority())
					UE_LOG(LogTemp, Log, TEXT("Controller %s is back on its original server %s"), *OriginalPawnPair.Controller->GetName(), *GetLocalFlowController()->GetDisplayName());
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

void ASpatialTestRemotePossession::AddToOriginalPawns_Implementation(ATestPossessionPlayerController* Controller, APawn* Pawn)
{
	FControllerPawnPair OriginalPair;
	OriginalPair.Controller = Controller;
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

