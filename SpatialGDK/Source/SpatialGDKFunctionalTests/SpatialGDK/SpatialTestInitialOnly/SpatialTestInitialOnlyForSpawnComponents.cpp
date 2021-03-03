// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "SpatialTestInitialOnlyForSpawnComponents.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "SpatialFunctionalTestFlowController.h"
#include "SpatialGDKFunctionalTests/SpatialGDK/TestActors/TestMovementCharacter.h"
#include "SpatialGDKSettings.h"
#include "TestClasses/SpatialTestInitialOnlySpawnActorWithComponent.h"

/**
 * Basic initial only test case.
 * Spawn an actor with components in front of player, change initial only & replicate value at server side, check client side value.
 *
 * step 1: server 1 create a cube with replicate property rep1=1 and initial only property initial1=1.
 * step 2: client 1 checkout actor's components and print these properties, should got this: rep1=1, initial1=1.
 * step 3: server 1 change rep1=2, initial1=2.
 * step 4: client 1 should got this: rep1=2, initial1=1.
 * step 5: clean up.
 */

ASpatialTestInitialOnlyForSpawnComponents::ASpatialTestInitialOnlyForSpawnComponents()
	: Super()
{
	Author = "Jeff Xu";
	Description = TEXT("Spawn a actor in front of player, change initial only & replicate value at server side, check client side value.");
}

void ASpatialTestInitialOnlyForSpawnComponents::PrepareTest()
{
	Super::PrepareTest();

	AddStep(TEXT("Init test environment"), FWorkerDefinition::Server(1), nullptr, [this]() {
		// Spawn cube
		ASpatialTestInitialOnlySpawnActorWithComponent* SpawnActor = GetWorld()->SpawnActor<ASpatialTestInitialOnlySpawnActorWithComponent>(
			FVector(-50.0f, 0.0f, 40.0f), FRotator::ZeroRotator, FActorSpawnParameters());

		RegisterAutoDestroyActor(SpawnActor);

		// Set the PositionUpdateThresholdMaxCentimeeters to a lower value so that the spatial position updates can be sent every time the
		// character moves, decreasing the overall duration of the test
		PreviousMaximumDistanceThreshold = GetDefault<USpatialGDKSettings>()->PositionUpdateThresholdMaxCentimeters;
		GetMutableDefault<USpatialGDKSettings>()->PositionUpdateThresholdMaxCentimeters = 0.0f;

		AssertTrue(GetDefault<USpatialGDKSettings>()->bEnableInitialOnlyReplicationCondition, TEXT("Initial Only Enabled"));

		// Spawn the TestMovementCharacter actor for Client 1 to possess.
		ASpatialFunctionalTestFlowController* FlowController = GetFlowController(ESpatialFunctionalTestWorkerType::Client, 1);
		ATestMovementCharacter* TestCharacter =
			GetWorld()->SpawnActor<ATestMovementCharacter>(FVector(0.0f, 0.0f, 40.0f), FRotator::ZeroRotator, FActorSpawnParameters());
		APlayerController* PlayerController = Cast<APlayerController>(FlowController->GetOwner());

		// Set a reference to the previous Pawn so that it can be processed back in the last step of the test
		OriginalPawn = TPair<AController*, APawn*>(PlayerController, PlayerController->GetPawn());

		RegisterAutoDestroyActor(TestCharacter);
		PlayerController->Possess(TestCharacter);

		FinishStep();
	});

	AddStep(
		TEXT("client checkout actor"), FWorkerDefinition::Client(1),
		[this]() -> bool {
			bool IsReady = true;

			TArray<AActor*> SpawnActors;
			UGameplayStatics::GetAllActorsOfClass(GetWorld(), ASpatialTestInitialOnlySpawnActorWithComponent::StaticClass(), SpawnActors);
			for (AActor* Actor : SpawnActors)
			{
				ASpatialTestInitialOnlySpawnActorWithComponent* SpawnActor = Cast<ASpatialTestInitialOnlySpawnActorWithComponent>(Actor);
				if (SpawnActor == nullptr)
				{
					IsReady = false;
					break;
				}
				else if (SpawnActor->InitialOnlyComponent == nullptr)
				{
					IsReady = false;
					break;
				}
			}

			return IsReady;
		},
		[this]() {
			TArray<AActor*> SpawnActors;
			UGameplayStatics::GetAllActorsOfClass(GetWorld(), ASpatialTestInitialOnlySpawnActorWithComponent::StaticClass(), SpawnActors);
			for (AActor* Actor : SpawnActors)
			{
				ASpatialTestInitialOnlySpawnActorWithComponent* SpawnActor = Cast<ASpatialTestInitialOnlySpawnActorWithComponent>(Actor);
				if (SpawnActor != nullptr)
				{
					AssertTrue(SpawnActor->InitialOnlyComponent->Int_Initial == 1, TEXT("Check InitialOnlyComponent.Int_Initial value."));
					AssertTrue(SpawnActor->InitialOnlyComponent->Int_Replicate == 1,
							   TEXT("Check InitialOnlyComponent.Int_Replicate value."));
				}
			}

			FinishStep();
		});

	AddStep(TEXT("Server change value"), FWorkerDefinition::Server(1), nullptr, [this]() {
		TArray<AActor*> SpawnActors;
		UGameplayStatics::GetAllActorsOfClass(GetWorld(), ASpatialTestInitialOnlySpawnActorWithComponent::StaticClass(), SpawnActors);
		for (AActor* Actor : SpawnActors)
		{
			ASpatialTestInitialOnlySpawnActorWithComponent* SpawnActor = Cast<ASpatialTestInitialOnlySpawnActorWithComponent>(Actor);
			if (SpawnActor != nullptr)
			{
				SpawnActor->InitialOnlyComponent->Int_Initial = 2;
				SpawnActor->InitialOnlyComponent->Int_Replicate = 2;
			}
		}

		FinishStep();
	});

	AddStep(TEXT("Client check value"), FWorkerDefinition::Client(1), nullptr, [this]() {
		TArray<AActor*> SpawnActors;
		UGameplayStatics::GetAllActorsOfClass(GetWorld(), ASpatialTestInitialOnlySpawnActorWithComponent::StaticClass(), SpawnActors);
		for (AActor* Actor : SpawnActors)
		{
			ASpatialTestInitialOnlySpawnActorWithComponent* SpawnActor = Cast<ASpatialTestInitialOnlySpawnActorWithComponent>(Actor);
			if (SpawnActor != nullptr)
			{
				AssertTrue(SpawnActor->InitialOnlyComponent->Int_Initial == 1, TEXT("Check InitialOnlyComponent.Int_Initial value."));
				AssertTrue(SpawnActor->InitialOnlyComponent->Int_Replicate == 2, TEXT("Check InitialOnlyComponent.Int_Replicate value."));
			}
		}

		FinishStep();
	});

	AddStep(TEXT("Cleanup"), FWorkerDefinition::Server(1), nullptr, [this]() {
		// Possess the original pawn, so that other tests start from the expected, default set-up
		OriginalPawn.Key->Possess(OriginalPawn.Value);

		FinishStep();
	});
}

void ASpatialTestInitialOnlyForSpawnComponents::FinishTest(EFunctionalTestResult TestResult, const FString& Message)
{
	Super::FinishTest(TestResult, Message);

	// Restoring the PositionUpdateThresholdMaxCentimeters here catches most but not all of the cases when the test failing would cause
	// PositionUpdateThresholdMaxCentimeters to be changed.
	GetMutableDefault<USpatialGDKSettings>()->PositionUpdateThresholdMaxCentimeters = PreviousMaximumDistanceThreshold;
}
