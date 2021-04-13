// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "PartiallyStablePathTest.h"

#include "EngineClasses/SpatialWorldSettings.h"
#include "PartiallyStablePathActor.h"
#include "PartiallyStablePathGameMode.h"
#include "PartiallyStablePathPawn.h"
#include "SpatialFunctionalTestFlowController.h"

#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"

/**
 * Partially Stable Path Test
 *
 * This test aims to verify that references to dynamically added subobjects with a partially
 * stable path are replicated correctly. A partially stable path means an object has a stable
 * path relative to its outer, but its full path is not stable, meaning one of its outers is
 * a dynamic actor.
 *
 * The test includes 1 server and 1 client.
 * The flow is as follows:
 * - Setup:
 * - Test:
 * - Cleanup:
 */

APartiallyStablePathTest::APartiallyStablePathTest()
{
	Author = TEXT("Valentyn");
	Description = TEXT("Test replicating references to subobjects with partially stable path.");
}

void APartiallyStablePathTest::PrepareTest()
{
	Super::PrepareTest();

	AddStep(TEXT("Spawn the test actor"), FWorkerDefinition::Server(1), nullptr, [this]() {
		// Verify the pawn has the right class
		ASpatialFunctionalTestFlowController* FlowController = GetFlowController(ESpatialFunctionalTestWorkerType::Client, 1);
		APlayerController* PlayerController = Cast<APlayerController>(FlowController->GetOwner());
		Pawn = Cast<APartiallyStablePathPawn>(PlayerController->GetPawn());
		AssertTrue(Pawn != nullptr, TEXT("The pawn is APartiallyStablePathPawn"));
		AssertFalse(Pawn->bServerRPCCalled, TEXT("Server RPC has not been called"));

		// Spawn the test actor
		APartiallyStablePathActor* TestActor = GetWorld()->SpawnActor<APartiallyStablePathActor>();

		RegisterAutoDestroyActor(TestActor);

		FinishStep();
	});

	AddStep(
		TEXT("Client calls a Server RPC"), FWorkerDefinition::Client(1),
		[this]() -> bool {
			TArray<AActor*> TestActors;
			UGameplayStatics::GetAllActorsOfClass(GetWorld(), APartiallyStablePathActor::StaticClass(), TestActors);

			return TestActors.Num() > 0;
		},
		[this]() {
			TArray<AActor*> TestActors;
			UGameplayStatics::GetAllActorsOfClass(GetWorld(), APartiallyStablePathActor::StaticClass(), TestActors);

			AssertEqual_Int(TestActors.Num(), 1, TEXT("Number of test actors on the client"));
			APartiallyStablePathActor* TestActor = Cast<APartiallyStablePathActor>(TestActors[0]);

			APlayerController* PlayerController = Cast<APlayerController>(GetLocalFlowController()->GetOwner());
			APartiallyStablePathPawn* Pawn = Cast<APartiallyStablePathPawn>(PlayerController->GetPawn());

			Pawn->ServerVerifyComponentReference(TestActor, TestActor->Component);

			FinishStep();
		},
		nullptr, 5.0f);

	AddStep(
		TEXT("Server receives the Server RPC"), FWorkerDefinition::Server(1), nullptr,
		[this]() {
			RequireTrue(Pawn->bServerRPCCalled, TEXT("Server RPC was called"));
			if (Pawn->bServerRPCCalled)
			{
				AssertTrue(Pawn->bRPCParamMatchesComponent, TEXT("Component reference passed into RPC matches the actual component"));
			}

			FinishStep();
		},
		nullptr, 5.0f);

	AddStep(
		TEXT("Reset the pawn"), FWorkerDefinition::Server(1), nullptr,
		[this]() {
			Pawn->bServerRPCCalled = false;
			Pawn->bRPCParamMatchesComponent = false;

			FinishStep();
		},
		nullptr);
}

UPartiallyStablePathTestMap::UPartiallyStablePathTestMap()
	: UGeneratedTestMap(EMapCategory::CI_PREMERGE, TEXT("PartiallyStablePathTestMap"))
{
	SetNumberOfClients(1);
}

void UPartiallyStablePathTestMap::CreateCustomContentForMap()
{
	ULevel* CurrentLevel = World->GetCurrentLevel();

	// Add the test
	AddActorToLevel<APartiallyStablePathTest>(CurrentLevel, FTransform::Identity);

	ASpatialWorldSettings* WorldSettings = CastChecked<ASpatialWorldSettings>(World->GetWorldSettings());
	WorldSettings->DefaultGameMode = APartiallyStablePathGameMode::StaticClass();
}
