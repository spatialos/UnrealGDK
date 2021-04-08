// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "SpatialTestRemotePossession.h"

#include "Kismet/GameplayStatics.h"
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

	// Ensure that all Controllers are located on the right Worker
	AddWaitStep(FWorkerDefinition::AllServers);

	ATestPossessionPlayerController::ResetCalledCounter();
}

bool ASpatialTestRemotePossession::IsReadyForPossess()
{
	ATestPossessionPawn* Pawn = GetPawn();
	return Pawn->Controller != nullptr;
}

void ASpatialTestRemotePossession::AddToOriginalPawns_Implementation(ATestPossessionPlayerController* Controller, APawn* Pawn)
{
	FControllerPawnPair OriginalPair;
	OriginalPair.Controller = MakeWeakObjectPtr(Controller);
	OriginalPair.Pawn = MakeWeakObjectPtr(Pawn);
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
