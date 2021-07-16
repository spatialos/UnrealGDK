// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "SpatialTestCharacterMigration.h"

#include "Components/BoxComponent.h"
#include "Engine/TriggerBox.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"

#include "EngineClasses/SpatialNetDriver.h"
#include "EngineClasses/SpatialWorldSettings.h"
#include "SpatialFunctionalTestFlowController.h"
#include "SpatialGDKFunctionalTests/SpatialGDK/SpatialTestCharacterMovement/CharacterMovementTestGameMode.h"
#include "SpatialGDKFunctionalTests/SpatialGDK/TestActors/TestMovementCharacter.h"
#include "TestWorkerSettings.h"

namespace
{
float GetTargetDistanceOnLine(const FVector& From, const FVector& Target, const FVector& Location)
{
	FVector Norm = (Target - From);
	Norm.Normalize();
	FVector RelativePosition = Location - Target;
	return FVector::DotProduct(Norm, RelativePosition);
}
} // namespace

/**
 * This test moves a character backward and forward repeatedly between two workers ensuring migration occurs.
 * PlayerController owned actors are spawned on both workers to ensure owned actors do not hinder migration.
 * Based on the SpatialTestCharacterMovement test. This test requires the CharacterMovementTestGameMode; trying to run this test on a
 * different game mode will fail. The test includes two servers and one client worker. Each client worker begins with a PlayerController and
 * a TestMovementCharacter.
 */

ASpatialTestCharacterMigration::ASpatialTestCharacterMigration()
	: Super()
{
	Author = "Victoria";
	Description = TEXT("Test Character Migration");
	TimeLimit = 300;

	PositionOnServerOne = FVector(-200.0f, 0.0f, 40.0f);
	PositionOnServerTwo = FVector(200.0f, 0.0f, 40.0f);
}

void ASpatialTestCharacterMigration::PrepareTest()
{
	Super::PrepareTest();

	// Wait for actor to be stationary
	FSpatialFunctionalTestStepDefinition WaitForStationaryActorStepDefinition(/*bIsNativeDefinition*/ true);
	WaitForStationaryActorStepDefinition.StepName = TEXT("WaitForStationaryActorStepDefinition");
	WaitForStationaryActorStepDefinition.NativeTickEvent.BindLambda([this](float DeltaTime) {
		const UGameInstance* GameInstance = GetGameInstance();
		const FString WorkerId = GameInstance->GetSpatialWorkerId();
		for (const ATestMovementCharacter* Character : TActorRange<ATestMovementCharacter>(GetWorld()))
		{
			RequireEqual_Float(Character->Speed, 0.0f, FString::Printf(TEXT("%s on %s is stationary"), *Character->GetName(), *WorkerId));
		}
		FinishStep();
	});

	// Add actor to controller
	FSpatialFunctionalTestStepDefinition AddActorStepDefinition(/*bIsNativeDefinition*/ true);
	AddActorStepDefinition.StepName = TEXT("Add actor to player controller");
	AddActorStepDefinition.TimeLimit = 0.0f;
	AddActorStepDefinition.NativeStartEvent.BindLambda([this]() {
		for (ASpatialFunctionalTestFlowController* FlowController : GetFlowControllers())
		{
			AController* PlayerController = Cast<AController>(FlowController->GetOwner());

			FActorSpawnParameters SpawnParams;
			SpawnParams.Owner = PlayerController;
			AActor* TestActor = GetWorld()->SpawnActor<AActor>(AActor::StaticClass(), FTransform(), SpawnParams);
			TestActor->SetReplicates(
				true); // NOTE: this currently causes parent not to migrate after a delay and outputs a warning in the test
			RegisterAutoDestroyActor(TestActor);
		}
		FinishStep();
	});

	// Move character forward
	FSpatialFunctionalTestStepDefinition MoveForwardStepDefinition(/*bIsNativeDefinition*/ true);
	MoveForwardStepDefinition.StepName = TEXT("MoveForwardStepDefinition");
	MoveForwardStepDefinition.NativeTickEvent.BindLambda([this](float DeltaTime) {
		int32 Count = 0;

		const UGameInstance* GameInstance = GetGameInstance();
		const FString WorkerId = GameInstance->GetSpatialWorkerId();

		const USpatialNetDriver* SpatialNetDriver = Cast<USpatialNetDriver>(GetWorld()->GetNetDriver());
		const bool bLoadBalanceStrategyValid = SpatialNetDriver != nullptr && SpatialNetDriver->LoadBalanceStrategy != nullptr;

		for (ATestMovementCharacter* Character : TActorRange<ATestMovementCharacter>(GetWorld()))
		{
			if (Character->IsLocallyControlled())
			{
				Character->AddMovementInput(FVector(1.0f, 0.0f, 0.0f), 10.0f, true);

				const float MaxExpectedSpeed = Character->GetCharacterMovement()->MaxWalkSpeed * 1.2f;
				AssertValue_Float(Character->Speed, EComparisonMethod::Less_Than, MaxExpectedSpeed, TEXT("Actor not speeding"));
			}

			bool bShouldRunRequires = true;
			if (bLoadBalanceStrategyValid)
			{
				const VirtualWorkerId ExpectedVirtualWorker = 2;
				const VirtualWorkerId LocalVirtualWorker = SpatialNetDriver->LoadBalanceStrategy->GetLocalVirtualWorkerId();
				bShouldRunRequires = LocalVirtualWorker == ExpectedVirtualWorker;

				if (bShouldRunRequires)
				{
					const VirtualWorkerId VirtualWorker = SpatialNetDriver->LoadBalanceStrategy->WhoShouldHaveAuthority(*Character);
					RequireEqual_Int(VirtualWorker, ExpectedVirtualWorker,
									 FString::Printf(TEXT("%s on %s crossed boundary"), *Character->GetName(), *WorkerId));
				}
			}

			if (bShouldRunRequires)
			{
				const bool bReachDestination =
					GetTargetDistanceOnLine(PositionOnServerOne, PositionOnServerTwo, Character->GetActorLocation())
					> -40.0f; // 40cm overlap
				RequireEqual_Bool(bReachDestination, true,
								  FString::Printf(TEXT("%s on %s reached destination"), *Character->GetName(), *WorkerId));
			}

			Count++;
		}

		RequireEqual_Int(Count, 2, TEXT("Check actor count"));
		FinishStep();
	});

	// Move character backward
	FSpatialFunctionalTestStepDefinition MoveBackwardStepDefinition(/*bIsNativeDefinition*/ true);
	MoveBackwardStepDefinition.StepName = TEXT("MoveBackwardStepDefinition");
	MoveBackwardStepDefinition.NativeTickEvent.BindLambda([this](float DeltaTime) {
		int32 Count = 0;

		const UGameInstance* GameInstance = GetGameInstance();
		const FString WorkerId = GameInstance->GetSpatialWorkerId();

		const USpatialNetDriver* SpatialNetDriver = Cast<USpatialNetDriver>(GetWorld()->GetNetDriver());
		const bool bLoadBalanceStrategyValid = SpatialNetDriver != nullptr && SpatialNetDriver->LoadBalanceStrategy != nullptr;

		for (ATestMovementCharacter* Character : TActorRange<ATestMovementCharacter>(GetWorld()))
		{
			if (Character->IsLocallyControlled())
			{
				Character->AddMovementInput(FVector(-1.0f, 0.0f, 0.0f), 10.0f, true);

				const float MaxExpectedSpeed = Character->GetCharacterMovement()->MaxWalkSpeed * 1.2f;
				AssertValue_Float(Character->Speed, EComparisonMethod::Less_Than, MaxExpectedSpeed, TEXT("Actor not speeding"));
			}

			bool bShouldRunRequires = true;
			if (bLoadBalanceStrategyValid)
			{
				const VirtualWorkerId ExpectedVirtualWorker = 1;
				const VirtualWorkerId LocalVirtualWorker = SpatialNetDriver->LoadBalanceStrategy->GetLocalVirtualWorkerId();
				bShouldRunRequires = LocalVirtualWorker == ExpectedVirtualWorker;

				if (bShouldRunRequires)
				{
					const VirtualWorkerId VirtualWorker = SpatialNetDriver->LoadBalanceStrategy->WhoShouldHaveAuthority(*Character);
					RequireEqual_Int(VirtualWorker, ExpectedVirtualWorker,
									 FString::Printf(TEXT("%s on %s crossed boundary"), *Character->GetName(), *WorkerId));
				}
			}

			if (bShouldRunRequires)
			{
				const bool bReachDestination =
					GetTargetDistanceOnLine(PositionOnServerTwo, PositionOnServerOne, Character->GetActorLocation())
					> -40.0f; // 40cm overlap
				RequireEqual_Bool(bReachDestination, true,
								  FString::Printf(TEXT("%s on %s reached destination"), *Character->GetName(), *WorkerId));
			}

			Count++;
		}

		RequireEqual_Int(Count, 2, TEXT("Check actor count"));
		FinishStep();
	});

	AddStepFromDefinition(WaitForStationaryActorStepDefinition, FWorkerDefinition::AllClients);
	for (int i = 0; i < 5; i++)
	{
		if (i == 0)
		{
			AddStepFromDefinition(AddActorStepDefinition, FWorkerDefinition::AllServers);
		}

		AddStepFromDefinition(MoveForwardStepDefinition, FWorkerDefinition::AllWorkers);

		if (i == 0)
		{
			AddStepFromDefinition(AddActorStepDefinition, FWorkerDefinition::AllServers);
		}

		AddStepFromDefinition(MoveBackwardStepDefinition, FWorkerDefinition::AllWorkers);
	}
}
