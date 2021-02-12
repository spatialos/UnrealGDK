// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "SpatialTestCharacterMigration.h"
#include "Components/BoxComponent.h"
#include "Engine/TriggerBox.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "SpatialFunctionalTestFlowController.h"
#include "SpatialGDKFunctionalTests/SpatialGDK/TestActors/TestMovementCharacter.h"

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
	: Super(FObjectInitializer::Get())
{
	Author = "Victoria";
	Description = TEXT("Test Character Migration");
	TimeLimit = 300;
}

void ASpatialTestCharacterMigration::PrepareTest()
{
	Super::PrepareTest();

	// Reset test
	FSpatialFunctionalTestStepDefinition ResetStepDefinition(/*bIsNativeDefinition*/ true);
	ResetStepDefinition.StepName = TEXT("Reset");
	ResetStepDefinition.TimeLimit = 0.0f;
	ResetStepDefinition.NativeStartEvent.BindLambda([this]() {
		bCharacterReachedDestination = false;
		bCharacterReachedOrigin = false;
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
	MoveForwardStepDefinition.TimeLimit = 0.0f;
	MoveForwardStepDefinition.NativeTickEvent.BindLambda([this](float DeltaTime) {
		AController* PlayerController = Cast<AController>(GetLocalFlowController()->GetOwner());
		ATestMovementCharacter* PlayerCharacter = Cast<ATestMovementCharacter>(PlayerController->GetPawn());

		PlayerCharacter->AddMovementInput(FVector(1, 0, 0), 10.0f, true);

		bCharacterReachedDestination =
			GetTargetDistanceOnLine(Origin, Destination, PlayerCharacter->GetActorLocation()) > -20.0f; // 20cm overlap

		if (bCharacterReachedDestination)
		{
			AssertTrue(bCharacterReachedDestination, TEXT("Player character has reached the destination on the autonomous proxy."));
			FinishStep();
		}
	});

	// Move character backward
	FSpatialFunctionalTestStepDefinition MoveBackwardStepDefinition(/*bIsNativeDefinition*/ true);
	MoveBackwardStepDefinition.StepName = TEXT("Client1MoveBackward");
	MoveBackwardStepDefinition.TimeLimit = 0.0f;
	MoveBackwardStepDefinition.NativeTickEvent.BindLambda([this](float DeltaTime) {
		AController* PlayerController = Cast<AController>(GetLocalFlowController()->GetOwner());
		ATestMovementCharacter* PlayerCharacter = Cast<ATestMovementCharacter>(PlayerController->GetPawn());

		PlayerCharacter->AddMovementInput(FVector(-1, 0, 0), 10.0f, true);

		bCharacterReachedOrigin = GetTargetDistanceOnLine(Destination, Origin, PlayerCharacter->GetActorLocation()) > -20.0f;
		; // 20cm overlap

		if (bCharacterReachedOrigin)
		{
			AssertTrue(bCharacterReachedOrigin, TEXT("Player character has reached the origin on the autonomous proxy."));
			FinishStep();
		}
	});

	// Universal setup step to create the TriggerBox and to set the helper variable
	AddStep(TEXT("UniversalSetupStep"), FWorkerDefinition::AllWorkers, nullptr, [this]() {
		bCharacterReachedDestination = false;
		bCharacterReachedOrigin = false;

		Destination = FVector(132.0f, 0.0f, 40.0f);
		Origin = FVector(-132.0f, 0.0f, 40.0f);

		FinishStep();
	});

	// Repeatedly move character forwards and backwards over the worker boundary and adding actors every time
	for (int i = 0; i < 5; i++)
	{
		if (i < 1)
		{
			AddStepFromDefinition(AddActorStepDefinition, FWorkerDefinition::AllServers);
		}

		AddStepFromDefinition(MoveForwardStepDefinition, FWorkerDefinition::Client(1));

		if (i < 1)
		{
			AddStepFromDefinition(AddActorStepDefinition, FWorkerDefinition::AllServers);
		}

		AddStepFromDefinition(MoveBackwardStepDefinition, FWorkerDefinition::Client(1));

		AddStepFromDefinition(ResetStepDefinition, FWorkerDefinition::AllWorkers);
	}
}
