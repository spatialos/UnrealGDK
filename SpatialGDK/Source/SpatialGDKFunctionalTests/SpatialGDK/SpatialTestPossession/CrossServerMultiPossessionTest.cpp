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

const float ACrossServerMultiPossessionTest::MaxWaitTime = 5.0f;

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

	AddStep(TEXT("Cross-Server Possession: Create Pawn"), FWorkerDefinition::Server(1), nullptr, [this]() {
		ATestPossessionPawn* Pawn =
			GetWorld()->SpawnActor<ATestPossessionPawn>(FVector(500.0f, 500.0f, 50.0f), FRotator::ZeroRotator, FActorSpawnParameters());
		RegisterAutoDestroyActor(Pawn);
		FinishStep();
	});

	AddStep(TEXT("Cross-Server Possession: Shown who should have authority"), FWorkerDefinition::Server(1), nullptr, nullptr,
			[this](float DeltaTime) {
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
								LogStep(ELogVerbosity::Log, FString::Printf(TEXT("Controller:%s authoritatived in worker: %d"),
																			*Controller->GetFullName(), WorkerId));
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

	AddStep(TEXT("Cross-Server Possession: controller remote possess"), FWorkerDefinition::AllServers, nullptr, [this]() {
		RemotePossess(1);
		RemotePossess(2);
		RemotePossess(3);
		FinishStep();
	});

	AddStep(TEXT("Cross-Server Possession: Wait"), FWorkerDefinition::Server(1), nullptr, nullptr, [this](float DeltaTime) {
		if (WaitTime > MaxWaitTime)
		{
			FinishStep();
		}
		WaitTime += DeltaTime;
	});

	AddStep(TEXT("Cross-Server Possession: Check results on all servers"), FWorkerDefinition::AllServers, nullptr, nullptr, [this](float DeltaTime) {
		if (ATestPossessionPawn* Pawn = GetPawn())
		{
			AssertTrue(Pawn->GetController() != nullptr, TEXT("GetController of Pawn to check if possessed on server"), Pawn);
		}
		else
		{
			LogStep(ELogVerbosity::Log, TEXT("Couldn't found pawn for possession"));
		}
		FinishStep();
	});

	AddStep(TEXT("Cross-Server Possession: Check results on all clients"), FWorkerDefinition::AllClients, nullptr, nullptr, [this](float DeltaTime) {
		if (ATestPossessionPawn* Pawn = GetPawn())
		{
			AssertTrue(Pawn->GetController() != nullptr, TEXT("GetController of Pawn to check if possessed on client"), Pawn);
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

void ACrossServerMultiPossessionTest::RemotePossess(int Index)
{
	if (ATestPossessionPawn* Pawn = GetPawn())
	{
		ASpatialFunctionalTestFlowController* FlowController = GetFlowController(ESpatialFunctionalTestWorkerType::Client, Index);
		ATestPossessionPlayerController* Controller = Cast<ATestPossessionPlayerController>(FlowController->GetOwner());
		if (IsValid(Controller))
		{
			ULayeredLBStrategy* LoadBalanceStrategy = GetLoadBalancingStrategy();
			if (LoadBalanceStrategy != nullptr)
			{
				uint32 WorkerId = LoadBalanceStrategy->WhoShouldHaveAuthority(*Controller);
				LogStep(ELogVerbosity::Log, FString::Printf(TEXT("Controller:%s authoritatived in worker: %d"), *Controller->GetFullName(), WorkerId));

				WorkerId = LoadBalanceStrategy->WhoShouldHaveAuthority(*Pawn);
				LogStep(ELogVerbosity::Log, FString::Printf(TEXT("Pawn:%s authoritatived in worker: %d"), *Pawn->GetFullName(), WorkerId));
				if (Controller->HasAuthority())
				{
					LogStep(ELogVerbosity::Log,
							FString::Printf(TEXT("%s try to remote possess %s"), *Controller->GetFullName(), *Pawn->GetFullName()));
					Controller->OnPossessEvent.AddDynamic(this, &ACrossServerMultiPossessionTest::OnPossess);
					Controller->OnUnPossessEvent.AddDynamic(this, &ACrossServerMultiPossessionTest::OnUnPossess);
					Controller->OnPossessFailedEvent.AddDynamic(this, &ACrossServerMultiPossessionTest::OnPossessFailed);
					USpatialPossession::RemotePossess(Controller, Pawn);
				}
			}
			else
			{
				FinishTest(EFunctionalTestResult::Error, TEXT("Failed to get LoadBalanceStrategy"));
			}
		}
		else
		{
			FinishTest(EFunctionalTestResult::Error, TEXT("Failed to get PlayerController"));
		}
	}
	else
	{
		FinishTest(EFunctionalTestResult::Error, TEXT("Failed to get the pawn for possession"));
	}
}

void ACrossServerMultiPossessionTest::OnPossess(APawn* Pawn, APlayerController* Controller)
{
	LogStep(ELogVerbosity::Log,
			FString::Printf(TEXT("Controller:%s OnPossess Pawn:%s"), *Controller->GetFullName(), *Pawn->GetFullName()));
}

void ACrossServerMultiPossessionTest::OnUnPossess(APlayerController* Controller)
{
	LogStep(ELogVerbosity::Log, FString::Printf(TEXT("Controller:%s OnUnPossess"), *Controller->GetFullName()));
}

void ACrossServerMultiPossessionTest::OnPossessFailed(ERemotePossessFailure FailureReason, APlayerController* Controller)
{
	LogStep(ELogVerbosity::Log, FString::Printf(TEXT("Controller:%s OnPossessFailure:%d"), *Controller->GetFullName(), FailureReason));
}
