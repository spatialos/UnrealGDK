// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "SpatialTestHandover.h"
#include "EngineClasses/SpatialNetDriver.h"
#include "Kismet/GameplayStatics.h"
#include "LoadBalancing/LayeredLBStrategy.h"

#include "HandoverCube.h"
#include "SpatialFunctionalTestFlowController.h"

/**
 * This test automated the Handover Gym which demonstrates that Entities correctly migrate between areas of authority.
 * This test requires the map it runs in to have the Multi worker enabled, with the BP_QuadrantZoningSettings set in order to be relevant.
 * The test includes 4 servers and 2 client workers.
 *
 * The flow is as follows:
 * - Setup:
 *	- Server 1 spawns a HandoverCube inside its authority area.
 *  - All servers set a reference to the HandoverCube and reset their local copy of the LocationIndex and AuthorityCheckIndex.
 * - Test:
 *	- At this stage, Server 1 should have authority over the HandoverCube.
 *  - The HandoverCube is moved in the authority area of Server 2.
 *  - At this stage, Server 2 should have authority over the HandoverCube.
 *  - Server 2 acquires a lock on the HandoverCube and moves it into the authority area of Server 4.
 *	- Since Server 2 has the lock on the HandoverCube it should still be authoritative over it.
 *  - Server 2 releases the lock on the HandoverCube.
 *  - At this point, Server 4 should become authoritative over the HandoverCube.
 *  - The HandoverCube is moved in the authority area of Server 3.
 *  - At this point, Server 3 should be authoritative over the HandoverCube.
 * - Clean-up:
 *	- The HandoverCube is destroyed.
 */
ASpatialTestHandover::ASpatialTestHandover()
	: Super()
{
	Author = "Andrei";
	Description = TEXT("Test Actor handover");

	Server1Position = FVector(-500.0f, -500.0f, 50.0f);
	Server2Position = FVector(500.0f, -500.0f, 50.0f);
	Server3Position = FVector(-500.0f, 500.0f, 50.0f);
	Server4Position = FVector(500.0f, 500.0f, 50.0f);
}

void ASpatialTestHandover::PrepareTest()
{
	Super::PrepareTest();

	// Server 1 spawns the HandoverCube under its authority area.
	AddStep(TEXT("SpatialTestHandoverServer1SpawnCube"), FWorkerDefinition::Server(1), nullptr, [this]() {
		HandoverCube = GetWorld()->SpawnActor<AHandoverCube>(Server1Position, FRotator::ZeroRotator, FActorSpawnParameters());
		RegisterAutoDestroyActor(HandoverCube);

		FinishStep();
	});

	float StepTimeLimit = 10.0f;

	// All servers set a reference to the HandoverCube and reset the LocationIndex and AuthorityCheckIndex.
	AddStep(
		TEXT("SpatialTestHandoverAllServersSetupStep"), FWorkerDefinition::AllServers, nullptr, nullptr,
		[this](float DeltaTime) {
			TArray<AActor*> HandoverCubes;
			UGameplayStatics::GetAllActorsOfClass(GetWorld(), AHandoverCube::StaticClass(), HandoverCubes);

			if (HandoverCubes.Num() == 1)
			{
				HandoverCube = Cast<AHandoverCube>(HandoverCubes[0]);

				USpatialNetDriver* NetDriver = Cast<USpatialNetDriver>(GetWorld()->GetNetDriver());

				AssertTrue(IsValid(NetDriver), TEXT("This test should be run with Spatial Networking"));

				LoadBalancingStrategy = Cast<ULayeredLBStrategy>(NetDriver->LoadBalanceStrategy);

				if (IsValid(HandoverCube) && IsValid(LoadBalancingStrategy))
				{
					// Reset the LocationIndex and the AuthorityChanges to allow multiple executions of the test.
					//LocationIndex = 0;
					//AuthorityChanges = 0;
					FinishStep();
				}
			}
		},
		StepTimeLimit);

	// Check that Server 1 is authoritative over the HandoverCube.
	AddStep(
		TEXT("SpatialTestHandoverServer1AuthorityAndPosition"), FWorkerDefinition::AllServers, nullptr, nullptr,
		[this](float DeltaTime) {
			RequireHandoverCubeAuthorityAndPosition(1, Server1Position);
			FinishStep();
		},
		StepTimeLimit);

	// Move the HandoverCube to the next location, which is inside the authority area of Server 2.
	AddStep(
		TEXT("SpatialTestHandoverServer1MoveToServer2"), FWorkerDefinition::Server(1), nullptr, nullptr,
		[this](float DeltaTime) {
			if( MoveHandoverCube(Server2Position))
			{
				FinishStep();
			}
		},
		StepTimeLimit);

	// Check that Server 2 is authoritative over the HandoverCube.
	AddStep(
		TEXT("SpatialTestHandoverServer2AuthorityAndPosition"), FWorkerDefinition::AllServers, nullptr, nullptr,
		[this](float DeltaTime) {
			RequireHandoverCubeAuthorityAndPosition(2, Server2Position);
			FinishStep();
		},
		StepTimeLimit);


	// Server 2 acquires a lock on the HandoverCube.
	AddStep(TEXT("SpatialTestHandoverServer2AcquireLock"), FWorkerDefinition::Server(2), nullptr, [this]() {
		HandoverCube->AcquireLock(2);
		FinishStep();
	});

	// Move the HandoverCube to the next location, which is inside the authority area of Server 4.
	AddStep(
		TEXT("SpatialTestHandoverServer2MoveToServer4"), FWorkerDefinition::Server(2), nullptr, nullptr,
		[this](float DeltaTime) {
			if (MoveHandoverCube(Server4Position))
			{
				FinishStep();
			}
		},
		StepTimeLimit);

	// Check that Server 2 is still  authoritative over the HandoverCube due to acquiring the lock earlier.
	AddStep(
		TEXT("SpatialTestHandoverServer2AuthorityAndServer4Position"), FWorkerDefinition::AllServers, nullptr, nullptr,
		[this](float DeltaTime) {
			RequireHandoverCubeAuthorityAndPosition(2, Server4Position);
			FinishStep();
		},
		StepTimeLimit);

	// Server 2 releases the lock on the HandoverCube.
	AddStep(TEXT("SpatialTestHandoverServer2ReleaseLock"), FWorkerDefinition::Server(2), nullptr, [this]() {
		HandoverCube->ReleaseLock();
		FinishStep();
	});

	// Check that Server 4 is now authoritative over the HandoverCube.
	AddStep(
		TEXT("SpatialTestHandoverServer4AuthorityAndPosition"), FWorkerDefinition::AllServers, nullptr, nullptr,
		[this](float DeltaTime) {
			RequireHandoverCubeAuthorityAndPosition(4, Server4Position);
			FinishStep();
		},
		StepTimeLimit);

	// Move the HandoverCube to the next location, which is inside the authority area of Server 3.
	AddStep(
		TEXT("SpatialTestHandoverServer4MoveToServer3"), FWorkerDefinition::Server(4), nullptr, nullptr,
		[this](float DeltaTime) {
			if (MoveHandoverCube(Server3Position))
			{
				FinishStep();
			}
		},
		StepTimeLimit);

	// Check that Server 3 is now authoritative over the HandoverCube.
	AddStep(
		TEXT("SpatialTestHandoverServer3AuthorityAndPosition"), FWorkerDefinition::AllServers, nullptr, nullptr,
		[this](float DeltaTime) {
			RequireHandoverCubeAuthorityAndPosition(3, Server3Position);
			FinishStep();
		},
		StepTimeLimit);
}

void ASpatialTestHandover::RequireHandoverCubeAuthorityAndPosition(int WorkerShouldHaveAuthority, FVector ExpectedPosition)
{
	if (!ensureMsgf(GetLocalWorkerType() == ESpatialFunctionalTestWorkerType::Server, TEXT("Should only be called in Servers")))
	{
		return;
	}

	RequireEqual_Vector(HandoverCube->GetActorLocation(), ExpectedPosition, FString::Printf(TEXT("HandoverCube in %s"), *ExpectedPosition.ToCompactString()), 1.0f);

	if(WorkerShouldHaveAuthority == GetLocalWorkerId())
	{
		RequireTrue(HandoverCube->HasAuthority(), TEXT("Has Authority"));
	}
	else
	{
		RequireFalse(HandoverCube->HasAuthority(), TEXT("Doesn't Have Authority"));
	}
}

bool ASpatialTestHandover::MoveHandoverCube(FVector Position)
{
	if (HandoverCube->HasAuthority())
	{
		HandoverCube->SetActorLocation(Position);
		return true;
	}

	return false;
}
