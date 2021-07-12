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
 * This test moves a character backward and forward repeatedly between two workers, adding actors. Based on the SpatialTestCharacterMovement
 * test. This test requires the CharacterMovementTestGameMode, trying to run this test on a different game mode will fail.
 *
 * The test includes two servers and one client worker. The client worker begins with a PlayerController and a TestCharacterMovement
 *
 */

ASpatialTestCharacterMigration::ASpatialTestCharacterMigration()
	: Super()
{
	Author = "Victoria";
	Description = TEXT("Test Character Migration");
	TimeLimit = 300;

	Destination = FVector(132.0f, 0.0f, 40.0f);
	Origin = FVector(-132.0f, 0.0f, 40.0f);
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
			RequireEqual_Float(AverageSpeed, 0.0f, TEXT("Actor has become stationary"));
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
		for (TActorIterator<ATestMovementCharacter> Iter(GetWorld()); Iter; ++Iter)
		{
			ATestMovementCharacter* Character = *Iter;
			Character->AddMovementInput(FVector(1.0f, 0.0f, 0.0f), 10.0f, true);

			float PeakSpeed = Character->GetPeakSpeedInWindow();
			RequireEqual_Bool(PeakSpeed < 60.0f, true, TEXT("Check actor peak speed"));

			bool bReachDestination = GetTargetDistanceOnLine(Origin, Destination, Character->GetActorLocation()) > -20.0f; // 20cm overlap
			RequireEqual_Bool(bReachDestination, true, TEXT("Check reached destination"));
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
		for (TActorIterator<ATestMovementCharacter> Iter(GetWorld()); Iter; ++Iter)
		{
			ATestMovementCharacter* Character = *Iter;
			Character->AddMovementInput(FVector(-1.0f, 0.0f, 0.0f), 10.0f, true);

			float PeakSpeed = Character->GetPeakSpeedInWindow();
			RequireEqual_Bool(PeakSpeed < 60.0f, true, TEXT("Check actor peak speed"));

			bool bReachDestination = GetTargetDistanceOnLine(Destination, Origin, Character->GetActorLocation()) > -20.0f; // 20cm overlap
			RequireEqual_Bool(bReachDestination, true, TEXT("Check reached destination"));
			bAllCharactersReachedDestination &= bReachDestination;

			Count++;
		}

		RequireEqual_Int(Count, 2, TEXT("Check actor count"));
		FinishStep();
	});

	AddStepFromDefinition(WaitForStationaryActorStepDefinition, FWorkerDefinition::AllClients);
	for (int i = 0; i < 5; i++)
	{
		AddStepFromDefinition(MoveForwardStepDefinition, FWorkerDefinition::AllClients);
		AddStepFromDefinition(MoveBackwardStepDefinition, FWorkerDefinition::AllClients);
	}
}
