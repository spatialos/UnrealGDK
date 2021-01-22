// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "SpatialTestRemotePossession.h"

#include "Kismet/GameplayStatics.h"
#include "TestPossessionController.h"
#include "TestPossessionPawn.h"

const float ASpatialTestRemotePossession::MaxWaitTime = 2.0f;

ASpatialTestRemotePossession::ASpatialTestRemotePossession()
	: Super()
{
	Author = "Jay";
	Description = TEXT("Test Actor Remote Possession");
}

ATestPossessionPawn* ASpatialTestRemotePossession::GetPawn()
{
	return Cast<ATestPossessionPawn>(UGameplayStatics::GetActorOfClass(GetWorld(), ATestPossessionPawn::StaticClass()));
}

ATestPossessionController* ASpatialTestRemotePossession::GetController()
{
	return Cast<ATestPossessionController>(UGameplayStatics::GetActorOfClass(GetWorld(), ATestPossessionController::StaticClass()));
}

void ASpatialTestRemotePossession::PrepareTest()
{
	Super::PrepareTest();

	AddStep(TEXT("EnsureSpatialOS"), FWorkerDefinition::AllServers, nullptr, [this]() {
		ULayeredLBStrategy* LoadBalanceStrategy = GetLoadBalancingStrategy();
		AssertTrue(LoadBalanceStrategy != nullptr, TEXT("Test requires SpatialOS enabled with Load-Balancing Strategy"));
		FinishStep();
	});

	AddStep(TEXT("Create Controller(s) and Pawn"), FWorkerDefinition::Server(1), nullptr, nullptr, [this](float DeltaTime) {
		CreateControllerAndPawn();
		FinishStep();
	});

	// Ensure that all Controllers are located on the right Worker
	AddWaitStep(FWorkerDefinition::AllServers);

	ATestPossessionController::ResetCalledCounter();
}

void ASpatialTestRemotePossession::CreateController(FVector Location)
{
	ATestPossessionController* Controller =
		GetWorld()->SpawnActor<ATestPossessionController>(Location, FRotator::ZeroRotator, FActorSpawnParameters());
	RegisterAutoDestroyActor(Controller);
}

void ASpatialTestRemotePossession::CreatePawn(FVector Location)
{
	ATestPossessionPawn* Pawn = GetWorld()->SpawnActor<ATestPossessionPawn>(Location, FRotator::ZeroRotator, FActorSpawnParameters());
	RegisterAutoDestroyActor(Pawn);
}

bool ASpatialTestRemotePossession::IsReadyForPossess()
{
	ATestPossessionPawn* Pawn = GetPawn();
	return Pawn->Controller != nullptr;
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

void ASpatialTestRemotePossession::AddCleanStep()
{
	AddStep(TEXT("Clean"), FWorkerDefinition::AllServers, nullptr, nullptr, [this](float) {
		if (ASpatialFunctionalTestFlowController* FlowController = GetLocalFlowController())
		{
			ATestPossessionPawn* Pawn = GetPawn();
			if (Pawn != nullptr && Pawn->HasAuthority())
			{
				GetWorld()->DestroyActor(Pawn);
			}

			TArray<AActor*> OutActors;
			UGameplayStatics::GetAllActorsOfClass(GetWorld(), ATestPossessionController::StaticClass(), OutActors);
			for (AActor* Actor : OutActors)
			{
				if (Actor != nullptr && Actor->HasAuthority())
				{
					GetWorld()->DestroyActor(Actor);
				}
			}
		}
		FinishStep();
	});
}
