// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "SpatialTestInitialOnlyForInterestActor.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "SpatialFunctionalTestFlowController.h"
#include "SpatialGDKFunctionalTests/SpatialGDK/TestActors/TestPossessionPawn.h"
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
	Description = TEXT("Spawn an actor in front of player, change initial only & replicate value at server side, check client side value.");
}

void ASpatialTestInitialOnlyForInterestActor::PrepareTest()
{
	Super::PrepareTest();

	AddStep(TEXT("Init test environment"), FWorkerDefinition::Server(1), nullptr, [this]() {
		// Spawn cube
		ASpatialTestInitialOnlySpawnActor* SpawnActor = GetWorld()->SpawnActor<ASpatialTestInitialOnlySpawnActor>(
			FVector(-1500.0f, 0.0f, 40.0f), FRotator::ZeroRotator, FActorSpawnParameters());

		RegisterAutoDestroyActor(SpawnActor);

		AssertTrue(GetDefault<USpatialGDKSettings>()->bEnableInitialOnlyReplicationCondition, TEXT("Initial Only Enabled"));

		// Spawn the TestPossessionPawn actor for Client 1 to possess.
		ASpatialFunctionalTestFlowController* FlowController = GetFlowController(ESpatialFunctionalTestWorkerType::Client, 1);
		ATestPossessionPawn* TestCharacter =
			GetWorld()->SpawnActor<ATestPossessionPawn>(FVector(1500.0f, 0.0f, 40.0f), FRotator::ZeroRotator, FActorSpawnParameters());
		APlayerController* PlayerController = Cast<APlayerController>(FlowController->GetOwner());

		// Set a reference to the previous Pawn so that it can be processed back in the last step of the test
		OriginalPawn = TPair<AController*, APawn*>(PlayerController, PlayerController->GetPawn());

		RegisterAutoDestroyActor(TestCharacter);
		PlayerController->Possess(TestCharacter);

		FinishStep();
	});

	AddStep(
		TEXT("Client checks test actor is not present in their world."), FWorkerDefinition::Client(1), nullptr, nullptr,
		[this](float DeltaTime) {
			TArray<AActor*> SpawnActors;
			UGameplayStatics::GetAllActorsOfClass(GetWorld(), ASpatialTestInitialOnlySpawnActor::StaticClass(), SpawnActors);
			RequireEqual_Int(SpawnActors.Num(), 0, TEXT("There should be no SpawnActor in the world."));

			FinishStep();
		},
		30.0f);

	AddStep(TEXT("Move character to cube"), FWorkerDefinition::Server(1), nullptr, [this]() {
		ASpatialFunctionalTestFlowController* FlowController = GetFlowController(ESpatialFunctionalTestWorkerType::Client, 1);
		APlayerController* PlayerController = Cast<APlayerController>(FlowController->GetOwner());
		ATestPossessionPawn* PlayerCharacter = Cast<ATestPossessionPawn>(PlayerController->GetPawn());

		// Move the character to the correct location
		// Now, beware that native unreal determines the position for net relevancy from the perspective of the client's CAMERA rather than
		// the character position, so this may be offset by as much as 300 units with our current camera offsets.
		PlayerCharacter->SetActorLocation(FVector(-1450.0f, 0.0f, 40.0f));

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
					AssertEqual_Int(SpawnActor->Int_Initial, 1, TEXT("Check Actor.Int_Initial value."));
					AssertEqual_Int(SpawnActor->Int_Replicate, 1, TEXT("Check Actor.Int_Replicate value."));
				}
			}

			FinishStep();
		});

	AddStep(TEXT("Server change value."), FWorkerDefinition::Server(1), nullptr, [this]() {
		TArray<AActor*> SpawnActors;
		UGameplayStatics::GetAllActorsOfClass(GetWorld(), ASpatialTestInitialOnlySpawnActor::StaticClass(), SpawnActors);
		AssertEqual_Int(SpawnActors.Num(), 1, TEXT("There should be exactly one InitialOnly actor in the world."));
		for (AActor* Actor : SpawnActors)
		{
			ASpatialTestInitialOnlySpawnActor* SpawnActor = Cast<ASpatialTestInitialOnlySpawnActor>(Actor);
			if (AssertIsValid(SpawnActor, TEXT("SpawnActor should be valid.")))
			{
				SpawnActor->Int_Initial = 2;
				SpawnActor->Int_Replicate = 2;
			}
		}

		FinishStep();
	});

	AddStep(TEXT("Check changed value."), FWorkerDefinition::Client(1), nullptr, nullptr, [this](float DeltaTime) {
		TArray<AActor*> SpawnActors;
		UGameplayStatics::GetAllActorsOfClass(GetWorld(), ASpatialTestInitialOnlySpawnActor::StaticClass(), SpawnActors);
		AssertEqual_Int(SpawnActors.Num(), 1, TEXT("There should be exactly one InitialOnly actor in the world."));
		for (AActor* Actor : SpawnActors)
		{
			ASpatialTestInitialOnlySpawnActor* SpawnActor = Cast<ASpatialTestInitialOnlySpawnActor>(Actor);
			if (AssertIsValid(SpawnActor, TEXT("SpawnActor should be valid.")))
			{
				RequireEqual_Int(SpawnActor->Int_Initial, 1, TEXT("Check Actor.Int_Initial value."));
				RequireEqual_Int(SpawnActor->Int_Replicate, 2, TEXT("Check Actor.Int_Replicate value."));
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
