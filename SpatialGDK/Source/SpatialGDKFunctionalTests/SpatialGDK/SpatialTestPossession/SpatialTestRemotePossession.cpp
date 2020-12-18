// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "SpatialTestRemotePossession.h"

#include "Kismet/GameplayStatics.h"
#include "TestPossessionPawn.h"
#include "TestPossessionPlayerController.h"

const float ASpatialTestRemotePossession::MaxWaitTime = 2.0f;

ASpatialTestRemotePossession::ASpatialTestRemotePossession()
	: Super()
	, WaitTime(0.0f)
{
	Author = "Jay";
	Description = TEXT("Test Actor Remote Possession");
}

ATestPossessionPawn* ASpatialTestRemotePossession::GetPawn()
{
	return Cast<ATestPossessionPawn>(UGameplayStatics::GetActorOfClass(GetWorld(), ATestPossessionPawn::StaticClass()));
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

void ASpatialTestRemotePossession::PrepareTest()
{
	Super::PrepareTest();

	ATestPossessionPlayerController::ResetCalledCounter();

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

	// Make sure all the Controller and Pawn authoritative in right workers
	AddWaitStep(FWorkerDefinition::AllServers);
}
