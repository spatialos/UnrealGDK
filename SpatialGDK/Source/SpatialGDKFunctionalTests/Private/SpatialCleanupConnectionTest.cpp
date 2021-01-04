// Fill out your copyright notice in the Description page of Project Settings.

#include "SpatialCleanupConnectionTest.h"

#include "SpatialFunctionalTestFlowController.h"

ASpatialCleanupConnectionTest::ASpatialCleanupConnectionTest()
{
	Author = "Simon Sarginson";
	Description = TEXT("Tests if clients connections are cleaned up when leaving interest region of a non authorative server");

	Server1Position = FVector(-500.0f, -500.0f, 50.0f);
	Server1PositionAndInInterestBorderServer2 = FVector(-50.0f, -50.0f, 50.0f);
	Server2Position = FVector(500.0f, -500.0f, 50.0f);
}

void ASpatialCleanupConnectionTest::PrepareTest()
{
	Super::PrepareTest();

	// Note that we always have at least one client connection which is not a client connection at all but the connection to SpatialOS,
	// so all client connections counts are + 1
	AddStep(TEXT("Spawn player on server 1"), FWorkerDefinition::Server(1), nullptr, [this]() {
		USpatialNetDriver* Driver = Cast<USpatialNetDriver>(GetNetDriver());
		AssertIsValid(Driver, TEXT("Test is exclusive to using SpatialNetDriver"));
		ASpatialFunctionalTestFlowController* ClientOneFlowController = GetFlowController(ESpatialFunctionalTestWorkerType::Client, 1);
		PlayerController = Cast<APlayerController>(ClientOneFlowController->GetOwner());
		AssertIsValid(PlayerController, TEXT("Must have valid PlayerController for test"));
		DefaultPawn = PlayerController->GetPawn();
		PlayerController->UnPossess();
		SpawnedPawn = GetWorld()->SpawnActor<ATestMovementCharacter>(Server1Position, FRotator::ZeroRotator, FActorSpawnParameters());
		RegisterAutoDestroyActor(SpawnedPawn);
		PlayerController->Possess(SpawnedPawn);

		AssertEqual_Int(Driver->ClientConnections.Num(), GetNumberOfClientWorkers() + 1,
						TEXT("Spawn: expected 1 spatial connection + number of clients"));
		FinishStep();
	});

	AddStep(
		TEXT("Post spawn check connections on server 2"), FWorkerDefinition::Server(2), nullptr, nullptr,
		[this](float delta) {
			USpatialNetDriver* Driver = Cast<USpatialNetDriver>(GetNetDriver());
			RequireEqual_Int(Driver->ClientConnections.Num(), GetNumberOfClientWorkers(),
							 TEXT("Spawn: expected client connections only"));
			FinishStep();
		},
		5.0f);

	AddStep(TEXT("Move player to location on server 1 that overlaps with interest border server 2"), FWorkerDefinition::Server(1), nullptr,
			[this]() {
				SpawnedPawn->SetActorLocation(Server1PositionAndInInterestBorderServer2);
				USpatialNetDriver* Driver = Cast<USpatialNetDriver>(GetNetDriver());
				AssertEqual_Int(Driver->ClientConnections.Num(), GetNumberOfClientWorkers() + 1,
								TEXT("Move into server 2 interest: expected 1 spatial connection + number of clients"));
				FinishStep();
			});

	AddStep(
		TEXT("Post move player connections on server 2"), FWorkerDefinition::Server(2), nullptr, nullptr,
		[this](float delta) {
			USpatialNetDriver* Driver = Cast<USpatialNetDriver>(GetNetDriver());
			RequireEqual_Int(Driver->ClientConnections.Num(), GetNumberOfClientWorkers() + 1,
							 TEXT("Move into server 2 interest: expected 1 spatial connection + number of clients"));
			FinishStep();
		},
		5.0f);

	AddStep(TEXT("Move player to location on server 1 outside of interest border server 2"), FWorkerDefinition::Server(1), nullptr,
			[this]() {
				SpawnedPawn->SetActorLocation(Server1Position);
				USpatialNetDriver* Driver = Cast<USpatialNetDriver>(GetNetDriver());
				AssertEqual_Int(Driver->ClientConnections.Num(), GetNumberOfClientWorkers() + 1,
								TEXT("Move out of server 2 interest: expected 1 spatial connection + number of clients"));
				FinishStep();
			});

	AddStep(
		TEXT("Post move 2 player connections on server 2"), FWorkerDefinition::Server(2), nullptr, nullptr,
		[this](float delta) {
			USpatialNetDriver* Driver = Cast<USpatialNetDriver>(GetNetDriver());
			RequireEqual_Int(Driver->ClientConnections.Num(), GetNumberOfClientWorkers(),
							 TEXT("Move out of server 2 interest: expected client connections only"));
			FinishStep();
		},
		5.0f);

	AddStep(TEXT("SpatialTestNetReferenceServerCleanup"), FWorkerDefinition::Server(1), nullptr, [this]() {
		// Possess the original pawn, so that other tests start from the expected, default set-up
		PlayerController->Possess(DefaultPawn);
		FinishStep();
	});
}
