// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "SpatialTestInitialOnlyForInterestActor.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "SpatialFunctionalTestFlowController.h"
#include "SpatialGDKFunctionalTests/SpatialGDK/TestActors/TestMovementCharacter.h"
#include "SpatialGDKSettings.h"
#include "TestClasses/SpatialTestInitialOnlySpawnActor.h"

/**
 * Basic initial only test case.
 * Spawn an actor without player's interest, player move to actor, change initial only & replicate value at server side, check client side
 * value.
 *
 * step 1: server 1 create a cube with replicate property rep1=1 and initial only property initial1=1.
 * step 2: client 1 move to cube.
 * step 3: client 1 should got this: rep1=1, initial1=1.
 * step 4: server 1 change rep1=2, initial1=2.
 * step 5: client 1 should got this: rep1=2, initial1=1.
 * step 6: clean up.
 */

ASpatialTestInitialOnlyForInterestActor::ASpatialTestInitialOnlyForInterestActor()
	: Super()
{
	Author = "Jeff Xu";
	Description = TEXT("Spawn a actor in front of player, change initial only & replicate value at server side, check client side value.");
}

void ASpatialTestInitialOnlyForInterestActor::PrepareTest()
{
	Super::PrepareTest();

	AddStep(TEXT("Init test environment"), FWorkerDefinition::Server(1), nullptr, [this]() {
		// Spawn cube
		ASpatialTestInitialOnlySpawnActor* SpawnActor = GetWorld()->SpawnActor<ASpatialTestInitialOnlySpawnActor>(
			FVector(-400.0f, 0.0f, 40.0f), FRotator::ZeroRotator, FActorSpawnParameters());

		RegisterAutoDestroyActor(SpawnActor);

		// Set the PositionUpdateThresholdMaxCentimeeters to a lower value so that the spatial position updates can be sent every time the
		// character moves, decreasing the overall duration of the test
		PreviousMaximumDistanceThreshold = GetDefault<USpatialGDKSettings>()->PositionUpdateThresholdMaxCentimeters;
		GetMutableDefault<USpatialGDKSettings>()->PositionUpdateThresholdMaxCentimeters = 0.0f;

		AssertTrue(GetDefault<USpatialGDKSettings>()->bEnableInitialOnlyReplicationCondition, TEXT("Initial Only Enabled"));

		// Spawn the TestMovementCharacter actor for Client 1 to possess.
		ASpatialFunctionalTestFlowController* FlowController = GetFlowController(ESpatialFunctionalTestWorkerType::Client, 1);
		ATestMovementCharacter* TestCharacter =
			GetWorld()->SpawnActor<ATestMovementCharacter>(FVector(400.0f, 0.0f, 40.0f), FRotator::ZeroRotator, FActorSpawnParameters());
		APlayerController* PlayerController = Cast<APlayerController>(FlowController->GetOwner());

		// Set a reference to the previous Pawn so that it can be processed back in the last step of the test
		OriginalPawn = TPair<AController*, APawn*>(PlayerController, PlayerController->GetPawn());

		RegisterAutoDestroyActor(TestCharacter);
		PlayerController->Possess(TestCharacter);

		FinishStep();
	});

	AddStep(TEXT("Move character to cube"), FWorkerDefinition::Server(1), nullptr, [this]() {
		ASpatialFunctionalTestFlowController* FlowController = GetFlowController(ESpatialFunctionalTestWorkerType::Client, 1);
		APlayerController* PlayerController = Cast<APlayerController>(FlowController->GetOwner());
		ATestMovementCharacter* PlayerCharacter = Cast<ATestMovementCharacter>(PlayerController->GetPawn());

		// Move the character to the correct location
		PlayerCharacter->SetActorLocation(FVector(-350.0f, 0.0f, 40.0f));

		FinishStep();
	});

	AddStep(
		TEXT("Check default value."), FWorkerDefinition::Client(1),
		[this]() -> bool {
			bool IsReady = false;
			TArray<AActor*> SpawnActors;
			UGameplayStatics::GetAllActorsOfClass(GetWorld(), ASpatialTestInitialOnlySpawnActor::StaticClass(), SpawnActors);
			for (AActor* Actor : SpawnActors)
			{
				ASpatialTestInitialOnlySpawnActor* SpawnActor = Cast<ASpatialTestInitialOnlySpawnActor>(Actor);
				if (SpawnActor != nullptr)
				{
					IsReady = true;
				}
			}
			return IsReady;
		},
		[this]() {
			TArray<AActor*> SpawnActors;
			UGameplayStatics::GetAllActorsOfClass(GetWorld(), ASpatialTestInitialOnlySpawnActor::StaticClass(), SpawnActors);
			for (AActor* Actor : SpawnActors)
			{
				ASpatialTestInitialOnlySpawnActor* SpawnActor = Cast<ASpatialTestInitialOnlySpawnActor>(Actor);
				if (SpawnActor != nullptr)
				{
					AssertTrue(SpawnActor->Int_Initial == 1, TEXT("Check Actor.Int_Initial value."));
					AssertTrue(SpawnActor->Int_Replicate == 1, TEXT("Check Actor.Int_Replicate value."));
				}
			}

			FinishStep();
		});

	AddStep(TEXT("Server change value."), FWorkerDefinition::Server(1), nullptr, [this]() {
		TArray<AActor*> SpawnActors;
		UGameplayStatics::GetAllActorsOfClass(GetWorld(), ASpatialTestInitialOnlySpawnActor::StaticClass(), SpawnActors);
		for (AActor* Actor : SpawnActors)
		{
			ASpatialTestInitialOnlySpawnActor* SpawnActor = Cast<ASpatialTestInitialOnlySpawnActor>(Actor);
			if (SpawnActor != nullptr)
			{
				SpawnActor->Int_Initial = 2;
				SpawnActor->Int_Replicate = 2;
			}
		}

		FinishStep();
	});

	AddStep(
		TEXT("Check changed value."), FWorkerDefinition::Client(1),
		[this]() -> bool {
			bool IsReady = false;
			TArray<AActor*> SpawnActors;
			UGameplayStatics::GetAllActorsOfClass(GetWorld(), ASpatialTestInitialOnlySpawnActor::StaticClass(), SpawnActors);
			for (AActor* Actor : SpawnActors)
			{
				ASpatialTestInitialOnlySpawnActor* SpawnActor = Cast<ASpatialTestInitialOnlySpawnActor>(Actor);
				if (SpawnActor != nullptr)
				{
					IsReady = true;
				}
			}
			return IsReady;
		},
		[this]() {
			TArray<AActor*> SpawnActors;
			UGameplayStatics::GetAllActorsOfClass(GetWorld(), ASpatialTestInitialOnlySpawnActor::StaticClass(), SpawnActors);
			for (AActor* Actor : SpawnActors)
			{
				ASpatialTestInitialOnlySpawnActor* SpawnActor = Cast<ASpatialTestInitialOnlySpawnActor>(Actor);
				if (SpawnActor != nullptr)
				{
					AssertTrue(SpawnActor->Int_Initial == 1, TEXT("Check Actor.Int_Initial value."));
					AssertTrue(SpawnActor->Int_Replicate == 2, TEXT("Check Actor.Int_Replicate value."));
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

void ASpatialTestInitialOnlyForInterestActor::FinishTest(EFunctionalTestResult TestResult, const FString& Message)
{
	Super::FinishTest(TestResult, Message);

	// Restoring the PositionUpdateThresholdMaxCentimeters here catches most but not all of the cases when the test failing would cause
	// PositionUpdateThresholdMaxCentimeters to be changed.
	GetMutableDefault<USpatialGDKSettings>()->PositionUpdateThresholdMaxCentimeters = PreviousMaximumDistanceThreshold;
}
