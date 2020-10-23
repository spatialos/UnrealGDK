// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "SpatialTestCharacterMigration.h"
#include "Components/BoxComponent.h"
#include "Engine/TriggerBox.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "SpatialFunctionalTestFlowController.h"
#include "SpatialGDKFunctionalTests/SpatialGDK/TestActors/TestMovementCharacter.h"

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
}

void ASpatialTestCharacterMigration::OnOverlapBeginDestination(AActor* OverlappedActor, AActor* OtherActor)
{
	ATestMovementCharacter* OveralappingCharacter = Cast<ATestMovementCharacter>(OtherActor);

	if (OveralappingCharacter)
	{
		bCharacterReachedDestination = true;
	}
}

void ASpatialTestCharacterMigration::OnOverlapBeginOrigin(AActor* OverlappedActor, AActor* OtherActor)
{
	ATestMovementCharacter* OveralappingCharacter = Cast<ATestMovementCharacter>(OtherActor);

	if (OveralappingCharacter)
	{
		bCharacterReachedOrigin = true;
	}
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
		AController* PlayerController = UGameplayStatics::GetPlayerController(this, 0);

		FActorSpawnParameters SpawnParams;
		SpawnParams.Owner = PlayerController;
		AActor* TestActor = GetWorld()->SpawnActor<AActor>(AActor::StaticClass(), FTransform(), SpawnParams);
		TestActor->SetReplicates(true); // NOTE: this currently causes parent not to migrate after a delay and outputs a warning in the test
		RegisterAutoDestroyActor(TestActor);
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

		// Setup the destination trigger box
		ATriggerBox* TriggerBoxDestination =
			GetWorld()->SpawnActor<ATriggerBox>(FVector(132.0f, 0.0f, 40.0f), FRotator::ZeroRotator, FActorSpawnParameters());
		TriggerBoxDestination->Tags.Add("Destination");

		UBoxComponent* BoxComponentDestination = Cast<UBoxComponent>(TriggerBoxDestination->GetCollisionComponent());
		if (BoxComponentDestination)
		{
			BoxComponentDestination->SetBoxExtent(FVector(10.0f, 1.0f, 1.0f));
		}

		TriggerBoxDestination->OnActorBeginOverlap.AddDynamic(this, &ASpatialTestCharacterMigration::OnOverlapBeginDestination);
		RegisterAutoDestroyActor(TriggerBoxDestination);

		// Setup the origin trigger box
		ATriggerBox* TriggerBoxOrigin =
			GetWorld()->SpawnActor<ATriggerBox>(FVector(-132.0f, 0.0f, 40.0f), FRotator::ZeroRotator, FActorSpawnParameters());
		TriggerBoxDestination->Tags.Add("Origin");

		UBoxComponent* BoxComponentOrigin = Cast<UBoxComponent>(TriggerBoxOrigin->GetCollisionComponent());
		if (BoxComponentOrigin)
		{
			BoxComponentOrigin->SetBoxExtent(FVector(10.0f, 1.0f, 1.0f));
		}

		TriggerBoxOrigin->OnActorBeginOverlap.AddDynamic(this, &ASpatialTestCharacterMigration::OnOverlapBeginOrigin);
		RegisterAutoDestroyActor(TriggerBoxOrigin);

		FinishStep();
	});

	// The server checks if the clients received a TestCharacterMovement and moves them to the mentioned locations
	AddStep(TEXT("ServerSetupStep"), FWorkerDefinition::Server(1), nullptr, [this]() {
		for (ASpatialFunctionalTestFlowController* FlowController : GetFlowControllers())
		{
			if (FlowController->WorkerDefinition.Type == ESpatialFunctionalTestWorkerType::Server)
			{
				continue;
			}

			AController* PlayerController = Cast<AController>(FlowController->GetOwner());
			ATestMovementCharacter* PlayerCharacter = Cast<ATestMovementCharacter>(PlayerController->GetPawn());

			checkf(PlayerCharacter, TEXT("Client did not receive a TestMovementCharacter"));

			int FlowControllerId = FlowController->WorkerDefinition.Id;

			if (FlowControllerId == 1)
			{
				PlayerCharacter->SetActorLocation(FVector(0.0f, -25.0f, 50.0f));
			}
		}

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
