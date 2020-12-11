// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "CrossServerPossessionTest.h"

#include "Containers/Array.h"
#include "EngineClasses/SpatialNetDriver.h"
#include "EngineClasses/SpatialPossession.h"
#include "GameFramework/GameModeBase.h"
#include "GameFramework/PlayerController.h"
#include "GameMapsSettings.h"
#include "Kismet/GameplayStatics.h"
#include "LoadBalancing/LayeredLBStrategy.h"
#include "SpatialFunctionalTestFlowController.h"
#include "SpatialGDKFunctionalTests/SpatialGDK/TestActors/TestMovementCharacter.h"
#include "TestPossessionPawn.h"
#include "TestPossessionPlayerController.h"
#include "Utils/SpatialStatics.h"

const float ACrossServerPossessionTest::MaxWaitTime = 1.0f;

ACrossServerPossessionTest::ACrossServerPossessionTest()
	: WaitTime(0.0f)
{
	Author = "Ken.Yu";
	Description = TEXT("Test Cross-Server Possession");
}

void ACrossServerPossessionTest::PrepareTest()
{
	Super::PrepareTest();

	AddStep(TEXT("EnsureSpatialOS"), FWorkerDefinition::AllServers, nullptr, [this]() {
		ULayeredLBStrategy* LoadBalanceStrategy = GetLoadBalancingStrategy();
		if (LoadBalanceStrategy == nullptr)
		{
			FinishTest(EFunctionalTestResult::Error, TEXT("Test requires SpatialOS enabled with Load-Balancing Strategy"));
		}
		else
		{
			if (AGameModeBase* GameMode = Cast<AGameModeBase>(AGameModeBase::StaticClass()->GetDefaultObject()))
			{
				LogStep(ELogVerbosity::Log, FString::Printf(TEXT("GameMode:%s"), *GameMode->GetFullName()));
			}
			FinishStep();
		}
	});

	AddStep(TEXT("Cross-Server Possession check client 03 authority"), FWorkerDefinition::Server(3), nullptr, nullptr, [this](float) {
		ULayeredLBStrategy* LoadBalanceStrategy = GetLoadBalancingStrategy();
		ATestPossessionPawn* Pawn =
			GetWorld()->SpawnActor<ATestPossessionPawn>(FVector(-100.0f, 100.0f, 50.0f), FRotator::ZeroRotator, FActorSpawnParameters());
		uint32 WorkerId03 = LoadBalanceStrategy->WhoShouldHaveAuthority(*Pawn);
		LogStep(ELogVerbosity::Log, FString::Printf(TEXT("Worker %d has the authority of the Pawn"), WorkerId03));

		FinishStep();
	});

	AddStep(TEXT("Cross-Server Possession"), FWorkerDefinition::AllServers, nullptr, nullptr, [this](float) {
		for (ASpatialFunctionalTestFlowController* FlowController : GetFlowControllers())
		{
			if (FlowController->WorkerDefinition.Type == ESpatialFunctionalTestWorkerType::Client)
			{
				APlayerController* PlayerController = Cast<APlayerController>(FlowController->GetOwner());
				if (PlayerController && PlayerController->HasAuthority())
				{
					ATestPossessionPawn* Pawn =
						Cast<ATestPossessionPawn>(UGameplayStatics::GetActorOfClass(GetWorld(), ATestPossessionPawn::StaticClass()));

					AssertTrue(PlayerController->HasAuthority(), TEXT("PlayerController should HasAuthority"), PlayerController);
					AssertFalse(Pawn->HasAuthority(), TEXT("Pawn shouldn't HasAuthority"), Pawn);
					USpatialPossession::RemotePossess(PlayerController, Pawn);
				}
			}
		}
		FinishStep();
	});

	AddStep(TEXT("Cross-Server Possession: Wait"), FWorkerDefinition::Server(1), nullptr, nullptr, [this](float DeltaTime) {
		if (WaitTime > MaxWaitTime)
		{
			FinishStep();
		}
		WaitTime += DeltaTime;
	});

	AddStep(TEXT("Cross-Server Possession: Check test result"), FWorkerDefinition::Server(1), nullptr, nullptr, [this](float) {
		ASpatialFunctionalTestFlowController* FlowController = GetFlowController(ESpatialFunctionalTestWorkerType::Client, 1);
		APlayerController* PlayerController = Cast<APlayerController>(FlowController->GetOwner());
		ATestPossessionPawn* Pawn =
			Cast<ATestPossessionPawn>(UGameplayStatics::GetActorOfClass(GetWorld(), ATestPossessionPawn::StaticClass()));

		AssertTrue(Pawn->Controller == PlayerController, TEXT("PlayerController has possessed the pawn"), PlayerController);

		FinishStep();
	});
}
