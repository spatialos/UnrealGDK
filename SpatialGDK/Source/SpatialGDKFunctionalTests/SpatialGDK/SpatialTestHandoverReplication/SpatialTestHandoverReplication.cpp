// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "SpatialTestHandoverReplication.h"
#include "EngineClasses/SpatialNetDriver.h"
#include "Kismet/GameplayStatics.h"
#include "LoadBalancing/LayeredLBStrategy.h"

#include "DynamicReplicationHandoverCube.h"
#include "SpatialFunctionalTestFlowController.h"

/**
 * This tests that an Actor's bReplicate flag is properly handed over when dynamically set.
 * Tests UNR-4441
 * This test contains 4 Server and 2 Client workers.
 *
 * The flow is as follows:
 * - Setup:
 *	- Server 1 spawns a HandoverCube with bReplicates set to false inside its authority area. (ADynamicReplicationHandoverCube)
 *	- The bReplicates flag is set to true after the end of the Actor's initialization.
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
 *  - At this point, Server 3 should be authoritative over the Handover	Cube.
 * - Clean-up:
 *	- The HandoverCube is destroyed.
 */

ASpatialTestHandoverReplication::ASpatialTestHandoverReplication()
	: Super()
{
	Author = "Antoine Cordelle";
	Description = TEXT("Test dynamically set replication for an actor");

	Server1Position = FVector(-500.0f, -500.0f, 50.0f);
	Server2Position = FVector(500.0f, -500.0f, 50.0f);
	Server3Position = FVector(-500.0f, 500.0f, 50.0f);
	Server4Position = FVector(500.0f, 500.0f, 50.0f);
}

void ASpatialTestHandoverReplication::PrepareTest()
{
	Super::PrepareTest();

	AddStep(TEXT("Server spawns one DynamicReplicationHandoverCube"), FWorkerDefinition::Server(1), nullptr, [this]() {
		HandoverCube =
			GetWorld()->SpawnActor<ADynamicReplicationHandoverCube>(Server1Position, FRotator::ZeroRotator, FActorSpawnParameters());
		RegisterAutoDestroyActor(HandoverCube);
		FinishStep();
	});

	AddStep(TEXT("Server sets Actor's replication to true"), FWorkerDefinition::Server(1), nullptr, [this]() {
		HandoverCube->SetReplicates(true);
		FinishStep();
	});

	const float StepTimeLimit = 10.0f;

	// All servers set a reference to the HandoverCube and reset the LocationIndex and AuthorityCheckIndex.
	AddStep(
		TEXT("SpatialTestHandoverAllServersSetupStep"), FWorkerDefinition::AllServers, nullptr, nullptr,
		[this](float DeltaTime) {
			TArray<AActor*> HandoverCubes;
			UGameplayStatics::GetAllActorsOfClass(GetWorld(), ADynamicReplicationHandoverCube::StaticClass(), HandoverCubes);

			if (HandoverCubes.Num() == 1)
			{
				HandoverCube = Cast<ADynamicReplicationHandoverCube>(HandoverCubes[0]);

				USpatialNetDriver* NetDriver = Cast<USpatialNetDriver>(GetWorld()->GetNetDriver());

				AssertTrue(IsValid(NetDriver), TEXT("This test should be run with Spatial Networking"));

				LoadBalancingStrategy = Cast<ULayeredLBStrategy>(NetDriver->LoadBalanceStrategy);

				if (IsValid(HandoverCube) && IsValid(LoadBalancingStrategy))
				{
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
			if (MoveHandoverCube(Server2Position))
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

void ASpatialTestHandoverReplication::RequireHandoverCubeAuthorityAndPosition(int WorkerShouldHaveAuthority, FVector ExpectedPosition)
{
	if (!ensureMsgf(GetLocalWorkerType() == ESpatialFunctionalTestWorkerType::Server, TEXT("Should only be called in Servers")))
	{
		return;
	}

	RequireEqual_Vector(HandoverCube->GetActorLocation(), ExpectedPosition,
						FString::Printf(TEXT("HandoverCube in %s"), *ExpectedPosition.ToCompactString()), 1.0f);

	if (WorkerShouldHaveAuthority == GetLocalWorkerId())
	{
		RequireTrue(HandoverCube->HasAuthority(), TEXT("Has Authority"));
	}
	else
	{
		RequireFalse(HandoverCube->HasAuthority(), TEXT("Doesn't Have Authority"));
	}
}

bool ASpatialTestHandoverReplication::MoveHandoverCube(FVector Position)
{
	if (HandoverCube->HasAuthority())
	{
		HandoverCube->SetActorLocation(Position);
		return true;
	}

	return false;
}
