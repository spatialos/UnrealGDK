// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "SpatialTestCharacterMigration.h"
#include "Components/BoxComponent.h"
#include "Engine/TriggerBox.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "SpatialFunctionalTestFlowController.h"
#include "SpatialGDKFunctionalTests/SpatialGDK/TestActors/TestMovementCharacter.h"

/**
 * This test moves a character between two workers. Based on the SpatialTestCharacterMovement test.
 * This test requires the CharacterMovementTestGameMode, trying to run this test on a different game mode will fail.
 *
 * The test includes two servers and one client worker. The client worker begins with a PlayerController and a TestCharacterMovement
 *
 * The flow is as follows:
 * - Setup:
 *    - The servers and client create trigger boxes locally.
 *    - The server 1 checks if the client received a TestCharacterMovement and sets their position to (0.0f, 0.0f, 50.0f).
 *    - The client moves its character forward as an autonomous proxy towards the Destination.
 *	  - The client moves its character backward as an autonomous proxy towards the Origin.
 *  - Test:
 *     - The owning client asserts that his character has reached the Destination.
 *     - The server asserts that client's character has reached the Destination on the server.
 *     - The owning client asserts that his character has reached the Origin.
 *     - The server asserts that client's character has reached the Origin on the server.
 *  - Cleanup:
 *     - The trigger boxes are destroyed from all clients and servers
 */

ASpatialTestCharacterMigration::ASpatialTestCharacterMigration()
	: Super()
{
	Author = "Victoria";
	Description = TEXT("Test Character Migration");
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

	// Universal setup step to create the TriggerBox and to set the helper variable
	AddStep(TEXT("UniversalSetupStep"), FWorkerDefinition::AllWorkers, nullptr, [this]() {
		bCharacterReachedDestination = false;
		bCharacterReachedOrigin = false;

		// Setup the destination trigger box
		ATriggerBox* TriggerBoxDestination =
			GetWorld()->SpawnActor<ATriggerBox>(FVector(232.0f, 0.0f, 40.0f), FRotator::ZeroRotator, FActorSpawnParameters());
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
			GetWorld()->SpawnActor<ATriggerBox>(FVector(-232.0f, 0.0f, 40.0f), FRotator::ZeroRotator, FActorSpawnParameters());
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
				PlayerCharacter->SetActorLocation(FVector(0.0f, 0.0f, 50.0f));
			}
		}

		FinishStep();
	});

	// Client 1 moves his character forward and asserts that it reached the Destination locally.
	AddStep(
		TEXT("Client1MoveForward"), FWorkerDefinition::Client(1),
		[this]() -> bool {
			AController* PlayerController = Cast<AController>(GetLocalFlowController()->GetOwner());
			ATestMovementCharacter* PlayerCharacter = Cast<ATestMovementCharacter>(PlayerController->GetPawn());

			// Since the character is simulating gravity, it will drop from the original position close to (0, 0, 40), depending on the size
			// of the CapsuleComponent in the TestMovementCharacter. However, depending on physics is not good for tests, so I'm
			// changing this test to only compare Z (height) coordinate.
			return IsValid(PlayerCharacter) && FMath::IsNearlyEqual(PlayerCharacter->GetActorLocation().Z, 40.0f, 2.0f);
		},
		nullptr,
		[this](float DeltaTime) {
			AController* PlayerController = Cast<AController>(GetLocalFlowController()->GetOwner());
			ATestMovementCharacter* PlayerCharacter = Cast<ATestMovementCharacter>(PlayerController->GetPawn());

			PlayerCharacter->AddMovementInput(FVector(1, 0, 0), 1.0f);

			if (bCharacterReachedDestination)
			{
				AssertTrue(bCharacterReachedDestination, TEXT("Player character has reached the destination on the autonomous proxy."));
				FinishStep();
			}
		},
		10.0f);

	// Server asserts that the character of client 1 has reached the Destination.
	AddStep(
		TEXT("ServerCheckForwardMovementVisibility"), FWorkerDefinition::Server(1), nullptr, nullptr,
		[this](float DeltaTime) {
			if (bCharacterReachedDestination)
			{
				AssertTrue(bCharacterReachedDestination, TEXT("Player character has reached the destination on the server."));
				FinishStep();
			}
		},

		5.0f);

	// Client 1 moves his character and asserts that it reached the Destination locally.
	AddStep(
		TEXT("Client1MoveBackward"), FWorkerDefinition::Client(1),
		nullptr,
		nullptr,
		[this](float DeltaTime) {
			AController* PlayerController = Cast<AController>(GetLocalFlowController()->GetOwner());
			ATestMovementCharacter* PlayerCharacter = Cast<ATestMovementCharacter>(PlayerController->GetPawn());

			PlayerCharacter->AddMovementInput(FVector(-1, 0, 0), 1.0f);

			if (bCharacterReachedOrigin)
			{
				AssertTrue(bCharacterReachedOrigin, TEXT("Player character has reached the origin on the autonomous proxy."));
				FinishStep();
			}
		},
		10.0f);

	// Server asserts that the character of client 1 has reached the Destination.
	AddStep(
		TEXT("ServerCheckBackwardMovementVisibility"), FWorkerDefinition::Server(1), nullptr, nullptr,
		[this](float DeltaTime) {
			if (bCharacterReachedOrigin)
			{
				AssertTrue(bCharacterReachedOrigin, TEXT("Player character has reached the origin on the server."));
				FinishStep();
			}
		},

		5.0f);
}
