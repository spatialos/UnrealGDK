// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "SpatialTestCharacterMovement.h"
#include "Components/BoxComponent.h"
#include "Engine/TriggerBox.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "SpatialFunctionalTestFlowController.h"
#include "TestMovementCharacter.h"

/**
 * This test tests if the movement of a character from a starting point to a Destination, performed on a client, is correctly replicated on
 *the server and on all other clients. Note: The Destination is a TriggerBox spawned locally on each connected worker, either client or
 *server. This test requires the CharacterMovementTestGameMode, trying to run this test on a different game mode will fail.
 *
 * The test includes a single server and two client workers. The client workers begin with a PlayerController and a TestCharacterMovement
 *
 * The flow is as follows:
 * - Setup:
 *    - The server and each client create a TriggerBox locally.
 *    - The server checks if the clients received a TestCharacterMovement and sets their position to (0.0f, 0.0f, 50.0f) for the first
 *client and (100.0f, 300.0f, 50.0f) for the second.
 *    - The client with ID 1 moves its character as an autonomous proxy towards the Destination.
 *  - Test:
 *     - The owning client asserts that his character has reached the Destination.
 *     - The server asserts that client's 1 character has reached the Destination on the server.
 *     - The second client checks that client's 1 character has reached the Destination.
 *  - Cleanup:
 *     - The trigger box is destroyed from all clients and servers
 */

ASpatialTestCharacterMovement::ASpatialTestCharacterMovement()
	: Super()
{
	Author = "Andrei";
	Description = TEXT("Test Character Movement");
}

void ASpatialTestCharacterMovement::OnOverlapBegin(AActor* OverlappedActor, AActor* OtherActor)
{
	ATestMovementCharacter* OveralappingCharacter = Cast<ATestMovementCharacter>(OtherActor);

	if (OveralappingCharacter)
	{
		bCharacterReachedDestination = true;
	}
}

void ASpatialTestCharacterMovement::BeginPlay()
{
	Super::BeginPlay();

	// Universal setup step to create the TriggerBox and to set the helper variable
	AddStep(TEXT("UniversalSetupStep"), FWorkerDefinition::AllWorkers, nullptr, [this](ASpatialFunctionalTest* NetTest) {
		bCharacterReachedDestination = false;

		ATriggerBox* TriggerBox =
			GetWorld()->SpawnActor<ATriggerBox>(FVector(232.0f, 0.0f, 40.0f), FRotator::ZeroRotator, FActorSpawnParameters());

		UBoxComponent* BoxComponent = Cast<UBoxComponent>(TriggerBox->GetCollisionComponent());
		if (BoxComponent)
		{
			BoxComponent->SetBoxExtent(FVector(10.0f, 1.0f, 1.0f));
		}

		TriggerBox->OnActorBeginOverlap.AddDynamic(this, &ASpatialTestCharacterMovement::OnOverlapBegin);
		RegisterAutoDestroyActor(TriggerBox);

		FinishStep();
	});

	// The server checks if the clients received a TestCharacterMovement and moves them to the mentioned locations
	AddStep(TEXT("SpatialTestCharacterMovementServerSetupStep"), FWorkerDefinition::Server(1), nullptr,
			[this](ASpatialFunctionalTest* NetTest) {
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
					else
					{
						PlayerCharacter->SetActorLocation(FVector(100.0f + 100 * FlowControllerId, 300.0f, 50.0f));
					}
				}

				FinishStep();
			});

	// Client 1 moves his character and asserts that it reached the Destination locally.
	AddStep(
		TEXT("SpatialTestCharacterMovementClient1Move"), FWorkerDefinition::Client(1),
		[](ASpatialFunctionalTest* NetTest) -> bool {
			AController* PlayerController = Cast<AController>(NetTest->GetLocalFlowController()->GetOwner());
			ATestMovementCharacter* PlayerCharacter = Cast<ATestMovementCharacter>(PlayerController->GetPawn());

			// Since the character is simulating gravity, it will drop from the original position close to (0, 0, 40), depending on the size
			// of the CapsuleComponent in the TestMovementCharacter
			return IsValid(PlayerCharacter) && PlayerCharacter->GetActorLocation().Equals(FVector(0.0f, 0.0f, 40.0f), 2.0f);
		},
		nullptr,
		[this](ASpatialFunctionalTest* NetTest, float DeltaTime) {
			AController* PlayerController = Cast<AController>(GetLocalFlowController()->GetOwner());
			ATestMovementCharacter* PlayerCharacter = Cast<ATestMovementCharacter>(PlayerController->GetPawn());

			PlayerCharacter->AddMovementInput(FVector(1, 0, 0), 1.0f);

			if (bCharacterReachedDestination)
			{
				AssertTrue(bCharacterReachedDestination, TEXT("Player character has reached the destination on the autonomous proxy."));
				FinishStep();
			}
		},
		3.0f);

	// Server asserts that the character of client 1 has reached the Destination.
	AddStep(
		TEXT("SpatialTestChracterMovementServerCheckMovementVisibility"), FWorkerDefinition::Server(1), nullptr, nullptr,
		[this](ASpatialFunctionalTest* NetTest, float DeltaTime) {
			if (bCharacterReachedDestination)
			{
				AssertTrue(bCharacterReachedDestination, TEXT("Player character has reached the destination on the server."));
				FinishStep();
			}
		},
		1.0f);

	// Client 2 asserts that the character of client 1 has reached the Destination.
	AddStep(
		TEXT("SpatialTestCharacterMovementClient2CheckMovementVisibility"), FWorkerDefinition::Client(2), nullptr, nullptr,
		[this](ASpatialFunctionalTest* NetTest, float DeltaTime) {
			if (bCharacterReachedDestination)
			{
				AssertTrue(bCharacterReachedDestination, TEXT("Player character has reached the destination on the simulated proxy"));
				FinishStep();
			}
		},
		1.0f);
}
