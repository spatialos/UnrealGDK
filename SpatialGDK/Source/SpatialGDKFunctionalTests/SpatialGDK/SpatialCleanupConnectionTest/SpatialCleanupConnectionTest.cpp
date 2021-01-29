// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "SpatialCleanupConnectionTest.h"
#include "GameFramework/PlayerController.h"

#include "SpatialFunctionalTestFlowController.h"

/**
* This tests whether client connections get cleaned up when a pawn enters and leaves the interest of server that is not authoritative over it.
*
* The test includes two server workers and one client worker.
* The client worker begins with a player controller which unpossesses its default pawn and posses a TestMovementCharacter instead.
* The test requires interest borders to be in line with the coordinates the pawn is moved to such that -50 is within the interest of server 2 and -500 outside of it.
* The flow is as follows:
*  - Setup:
*    - (Refer to above about placing instructions).
*  - Test:
*    - Server 1 spawns the pawn inside of server 1's interest but outside server 2's, we verify that we have all client connections present + 1 for spatial
*    - Server 2 verifies it has all client connections - 1 + 1 for spatial
*    - Server 1 moves the pawn to another location still in server 1 but also in server 2's interest, we verify that we have all client connections present + 1 for spatial
*    - Server 2 verifies that it now has all client connections + 1 for spatial
*    - Server 1 moves the pawn inside of server 1's interest but again outside server 2's, we verify that we have all client connections present + 1 for spatial
*	  - Server 2 verifies it has all client connections - 1 + 1 for spatial
*  - Cleanup:
*    - The player controller reposseses the default pawn so it's reset for any subsequent test run.
*/

ASpatialCleanupConnectionTest::ASpatialCleanupConnectionTest()
{
	Author = "Simon Sarginson";
	Description = TEXT("Tests if clients connections are cleaned up when leaving interest region of a non authorative server");
	SetNumRequiredClients(1);

	Server1Position = FVector(-500.0f, -500.0f, 50.0f);
	Server1PositionAndInInterestBorderServer2 = FVector(-50.0f, -50.0f, 50.0f);
	Server2Position = FVector(-500.0f, 500.0f, 50.0f);
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
						TEXT("Spawn: expected all client connections and one spatial connection"));
		FinishStep();
	});

	AddStep(
		TEXT("Post spawn check connections on server 2"), FWorkerDefinition::Server(2), nullptr, nullptr,
		[this](float delta) {
			USpatialNetDriver* Driver = Cast<USpatialNetDriver>(GetNetDriver());
			RequireEqual_Int(Driver->ClientConnections.Num(), (GetNumberOfClientWorkers() - 1) + 1,
							 TEXT("Spawn: expected one less client connection and one spatial connection"));
			FinishStep();
		},
		5.0f);

	AddStep(TEXT("Move player to location on server 1 that overlaps with interest border server 2"), FWorkerDefinition::Server(1), nullptr,
			[this]() {
				SpawnedPawn->SetActorLocation(Server1PositionAndInInterestBorderServer2);
				USpatialNetDriver* Driver = Cast<USpatialNetDriver>(GetNetDriver());
				AssertEqual_Int(Driver->ClientConnections.Num(), GetNumberOfClientWorkers() + 1,
								TEXT("Move into server 2 interest: expected all client connections and one spatial connection"));
				FinishStep();
			});

	AddStep(
		TEXT("Post move player connections on server 2"), FWorkerDefinition::Server(2), nullptr, nullptr,
		[this](float delta) {
			USpatialNetDriver* Driver = Cast<USpatialNetDriver>(GetNetDriver());
			RequireEqual_Int(Driver->ClientConnections.Num(), GetNumberOfClientWorkers() + 1,
							 TEXT("Move into server 2 interest: expected all client connections and one spatial connection"));
			FinishStep();
		},
		5.0f);

	AddStep(TEXT("Move player to location on server 1 outside of interest border server 2"), FWorkerDefinition::Server(1), nullptr,
			[this]() {
				SpawnedPawn->SetActorLocation(Server1Position);
				USpatialNetDriver* Driver = Cast<USpatialNetDriver>(GetNetDriver());
				AssertEqual_Int(Driver->ClientConnections.Num(), GetNumberOfClientWorkers() + 1,
								TEXT("Move out of server 2 interest: expected all client connections and one spatial connection"));
				FinishStep();
			});

	AddStep(
		TEXT("Post move 2 player connections on server 2"), FWorkerDefinition::Server(2), nullptr, nullptr,
		[this](float delta) {
			USpatialNetDriver* Driver = Cast<USpatialNetDriver>(GetNetDriver());
			RequireEqual_Int(Driver->ClientConnections.Num(), (GetNumberOfClientWorkers() - 1) + 1,
							 TEXT("Move out of server 2 interest: expected one less client connection and one spatial connection"));
			FinishStep();
		},
		5.0f);

	AddStep(TEXT("SpatialTestNetReferenceServerCleanup"), FWorkerDefinition::Server(1), nullptr, [this]() {
		// Possess the original pawn, so that other tests start from the expected, default set-up
		PlayerController->Possess(DefaultPawn);
		FinishStep();
	});
}
