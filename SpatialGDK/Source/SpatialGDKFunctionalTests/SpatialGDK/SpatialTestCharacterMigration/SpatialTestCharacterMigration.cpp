// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "SpatialTestCharacterMigration.h"

#include "Components/BoxComponent.h"
#include "Engine/TriggerBox.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"

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
 * PlayerController owned actors are spawned after each successful migration. This tests that owned actors do not hinder migration.
 * Based on the SpatialTestCharacterMovement test. This test requires the CharacterMovementTestGameMode, trying to run this test on a
 * different game mode will fail. The test includes two servers and one client worker. The client worker begins with a PlayerController and
 * a TestCharacterMovement
 */

ASpatialTestCharacterMigration::ASpatialTestCharacterMigration()
	: Super()
{
	Author = "Victoria";
	Description = TEXT("Test Character Migration");
	TimeLimit = 300;

	Destination = FVector(200.0f, 0.0f, 40.0f);
	Origin = FVector(-200.0f, 0.0f, 40.0f);
}

void ASpatialTestCharacterMigration::PrepareTest()
{
	Super::PrepareTest();

	// Wait for actor to be stationary
	FSpatialFunctionalTestStepDefinition WaitForStationaryActorStepDefinition(/*bIsNativeDefinition*/ true);
	WaitForStationaryActorStepDefinition.StepName = TEXT("WaitForStationaryActorStepDefinition");
	WaitForStationaryActorStepDefinition.TimeLimit = 2.0f;
	WaitForStationaryActorStepDefinition.NativeTickEvent.BindLambda([this](float DeltaTime) {
		for (TActorIterator<ATestMovementCharacter> Iter(GetWorld()); Iter; ++Iter)
		{
			ATestMovementCharacter* Character = *Iter;
			float AverageSpeed = Character->GetAverageSpeedOverWindow();
			RequireEqual_Float(AverageSpeed, 0.0f, FString::Printf(TEXT("%s is stationary"), *Character->GetName()));
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
	MoveForwardStepDefinition.StepName = TEXT("Client1MoveForward");
	MoveForwardStepDefinition.TimeLimit = 5.0f;
	MoveForwardStepDefinition.NativeTickEvent.BindLambda([this](float DeltaTime) {
		bool bAllCharactersReachedDestination = true;
		int32 Count = 0;

		UGameInstance* GameInstance = GetGameInstance();
		const FString WorkerId = GameInstance->GetSpatialWorkerId();
		for (TActorIterator<ATestMovementCharacter> Iter(GetWorld()); Iter; ++Iter)
		{
			ATestMovementCharacter* Character = *Iter;

			if (Character->IsLocallyControlled())
			{
				Character->AddMovementInput(FVector(1.0f, 0.0f, 0.0f), 10.0f, true);
				float PeakSpeed = Character->GetPeakSpeedInWindow();
				AssertValue_Float(PeakSpeed, EComparisonMethod::Less_Than, 650.0f, TEXT("Actor not speeding"));
			}

			bool bReachDestination = GetTargetDistanceOnLine(Origin, Destination, Character->GetActorLocation()) > -40.0f; // 20cm overlap
			RequireEqual_Bool(bReachDestination, true,
							  FString::Printf(TEXT("%s on %s reached destination"), *Character->GetName(), *WorkerId));
			bAllCharactersReachedDestination &= bReachDestination;

			Count++;
		}

		RequireEqual_Int(Count, 2, TEXT("Check actor count"));
		FinishStep();
	});

	// Move character backward
	FSpatialFunctionalTestStepDefinition MoveBackwardStepDefinition(/*bIsNativeDefinition*/ true);
	MoveBackwardStepDefinition.StepName = TEXT("Client1MoveBackward");
	MoveBackwardStepDefinition.TimeLimit = 5.0f;
	MoveBackwardStepDefinition.NativeTickEvent.BindLambda([this](float DeltaTime) {
		bool bAllCharactersReachedDestination = true;
		int32 Count = 0;

		UGameInstance* GameInstance = GetGameInstance();
		const FString WorkerId = GameInstance->GetSpatialWorkerId();
		for (TActorIterator<ATestMovementCharacter> Iter(GetWorld()); Iter; ++Iter)
		{
			ATestMovementCharacter* Character = *Iter;

			if (Character->IsLocallyControlled())
			{
				Character->AddMovementInput(FVector(-1.0f, 0.0f, 0.0f), 10.0f, true);
				float PeakSpeed = Character->GetPeakSpeedInWindow();
				AssertValue_Float(PeakSpeed, EComparisonMethod::Less_Than, 650.0f, TEXT("Actor not speeding"));
			}

			bool bReachDestination = GetTargetDistanceOnLine(Destination, Origin, Character->GetActorLocation()) > -40.0f; // 20cm overlap
			RequireEqual_Bool(bReachDestination, true,
							  FString::Printf(TEXT("%s on %s reached destination"), *Character->GetName(), *WorkerId));
			bAllCharactersReachedDestination &= bReachDestination;

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
