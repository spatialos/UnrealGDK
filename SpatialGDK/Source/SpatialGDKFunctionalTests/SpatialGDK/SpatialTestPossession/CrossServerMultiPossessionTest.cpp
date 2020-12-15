// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "CrossServerMultiPossessionTest.h"

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

const float ACrossServerMultiPossessionTest::MaxWaitTime = 2.0f;

ACrossServerMultiPossessionTest::ACrossServerMultiPossessionTest()
	: Super()
	, WaitTime(0.0f)
{
	Author = "Ken.Yu";
	Description = TEXT("Test Cross-Server 3 Controllers Possess 1 Pawn");
}

void ACrossServerMultiPossessionTest::PrepareTest()
{
	Super::PrepareTest();

	ATestPossessionPlayerController::ResetCalledCounter();

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

	AddStep(TEXT("Create Pawn"), FWorkerDefinition::Server(1), nullptr, nullptr, [this](float DeltaTime) {
		ATestPossessionPawn* Pawn =
			GetWorld()->SpawnActor<ATestPossessionPawn>(FVector(500.0f, 500.0f, 50.0f), FRotator::ZeroRotator, FActorSpawnParameters());
		RegisterAutoDestroyActor(Pawn);
		FinishStep();
	});

	// Make sure all the Controller and Pawn authoritative in right workers
	AddWaitStep(FWorkerDefinition::AllClients);

	AddStep(TEXT("Shown who should have authority"), FWorkerDefinition::Server(1), nullptr, nullptr, [this](float DeltaTime) {
		ULayeredLBStrategy* LoadBalanceStrategy = GetLoadBalancingStrategy();
		if (LoadBalanceStrategy != nullptr)
		{
			if (ATestPossessionPawn* Pawn = GetPawn())
			{
				for (ASpatialFunctionalTestFlowController* FlowController : GetFlowControllers())
				{
					AController* Controller = Cast<AController>(FlowController->GetOwner());
					if (Controller != nullptr)
					{
						uint32 WorkerId = LoadBalanceStrategy->WhoShouldHaveAuthority(*Controller);
						LogStep(ELogVerbosity::Log,
								FString::Printf(TEXT("Controller:%s authoritatived in worker: %d"), *Controller->GetFullName(), WorkerId));
					}
				}

				uint32 WorkerId = LoadBalanceStrategy->WhoShouldHaveAuthority(*Pawn);
				LogStep(ELogVerbosity::Log,
						FString::Printf(TEXT("Pawn:%s authoritatived in worker: %d"), *GetPawn()->GetFullName(), WorkerId));
			}
			else
			{
				FinishTest(EFunctionalTestResult::Error, TEXT("Couldn't found pawn for possession"));
			}
		}
		else
		{
			FinishTest(EFunctionalTestResult::Error, TEXT("Test requires SpatialOS enabled with Load-Balancing Strategy"));
		}
		FinishStep();
	});

	AddStep(TEXT("Controller remote possess"), FWorkerDefinition::AllClients, nullptr, nullptr, [this](float DeltaTime) {
		if (ATestPossessionPawn* Pawn = GetPawn())
		{
			for (ASpatialFunctionalTestFlowController* FlowController : GetFlowControllers())
			{
				if (FlowController->WorkerDefinition.Type == ESpatialFunctionalTestWorkerType::Client)
				{
					ATestPossessionPlayerController* Controller = Cast<ATestPossessionPlayerController>(FlowController->GetOwner());
					if (Controller != nullptr)
					{
						Controller->RemotePossess(Pawn);
					}
				}
			}
		}
		FinishStep();
	});

	// Make sure all the workers can check the results
	AddWaitStep(FWorkerDefinition::AllServers);

	AddStep(TEXT("Check results on all servers"), FWorkerDefinition::AllServers, nullptr, nullptr, [this](float DeltaTime) {
		if (ATestPossessionPawn* Pawn = GetPawn())
		{
			AssertTrue(Pawn->GetController() != nullptr, TEXT("GetController of Pawn to check if possessed on server"), Pawn);

			AssertTrue(ATestPossessionPlayerController::OnPossessCalled == 1, TEXT("OnPossess should be called 1 time"));
			AssertTrue(ATestPossessionPlayerController::OnPossessFailedCalled == 2, TEXT("OnPossessFailed should be called 2 times"));
		}
		else
		{
			LogStep(ELogVerbosity::Log, TEXT("Couldn't found pawn for possession"));
		}
		FinishStep();
	});
}

ATestPossessionPawn* ACrossServerMultiPossessionTest::GetPawn()
{
	return Cast<ATestPossessionPawn>(UGameplayStatics::GetActorOfClass(GetWorld(), ATestPossessionPawn::StaticClass()));
}

void ACrossServerMultiPossessionTest::CreateController(int Index, FVector Position)
{
	ASpatialFunctionalTestFlowController* FlowController = GetFlowController(ESpatialFunctionalTestWorkerType::Client, Index);
	ATestPossessionPlayerController* PlayerController = Cast<ATestPossessionPlayerController>(FlowController->GetOwner());
	if (IsValid(PlayerController))
	{
		ATestMovementCharacter* Character =
			GetWorld()->SpawnActor<ATestMovementCharacter>(Position, FRotator::ZeroRotator, FActorSpawnParameters());
		RegisterAutoDestroyActor(Character);
		USpatialPossession::RemotePossess(PlayerController, Character);
	}
	else
	{
		FinishTest(EFunctionalTestResult::Error, TEXT("Failed to get PlayerController"));
	}
}

void ACrossServerMultiPossessionTest::CheckControllerHasAuthority(int Index)
{
	ASpatialFunctionalTestFlowController* FlowController = GetFlowController(ESpatialFunctionalTestWorkerType::Client, Index);
	ATestPossessionPlayerController* PlayerController = Cast<ATestPossessionPlayerController>(FlowController->GetOwner());
	if (IsValid(PlayerController))
	{
		AssertTrue(PlayerController->HasAuthority(), TEXT("PlayerController HasAuthority"), PlayerController);
	}
	else
	{
		FinishTest(EFunctionalTestResult::Error, TEXT("Failed to get PlayerController"));
	}
}

void ACrossServerMultiPossessionTest::AddWaitStep(const FWorkerDefinition& Worker)
{
	AddStep(TEXT("Wait"), Worker, nullptr, nullptr, [this](float DeltaTime) {
		if (WaitTime > MaxWaitTime)
		{
			FinishStep();
		}
		WaitTime += DeltaTime;
	});
}
