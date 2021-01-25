// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "SpatialTestCrossServerRPC.h"
#include "CrossServerRPCCube.h"
#include "EngineClasses/SpatialNetDriver.h"
#include "EngineClasses/SpatialPackageMapClient.h"
#include "Kismet/GameplayStatics.h"
#include "LoadBalancing/AbstractLBStrategy.h"
#include "NonReplicatedCrossServerRPCCube.h"
#include "SpatialFunctionalTestFlowController.h"

/**
 * This test automates the Server to server RPC gym, that was used to demonstrate that actors owned by different servers correctly send
 * server-to-server RPCs. The test includes 4 server workers and 2 clients. NOTE: This test requires the map it runs in to have the
 * BP_QuadrantLBStrategy and OwnershipLockingPolicy set in order to be relevant.
 *
 * The tests are in two sections: first startup actor RPC tests and then dynamic actor RPC tests.
 *
 * The flow for the startup actor tests is as follows:
 * - Setup:
 *  - The level contains one CrossServerRPCCube positioned in Server 4's authority area that is not replicated initially.
 *  - On all non-authoritative servers we turn on replication, explicitly set authority to non-authoritative and then send an RPC. These
 * specific steps were needed to recreate an error of the entity ID being incorrectly allocated on a non-auth server.
 * - Test
 *  - Check for valid entity IDs on all servers
 * - Clean-up
 *  - The level cubes is destroyed.
 *
 * The flow for the dynamic actor tests is as follows:
 * - Setup:
 *  - Each server spawns one CrossServerRPCCube.
 *  - Each server sends RPCs to all the cubes that are not under his authority.
 * - Test
 *  - Server 1 checks that all the expected RPCs were received by all cubes.
 * - Clean-up
 *  - The previously spawned cubes are destroyed.
 */

ASpatialTestCrossServerRPC::ASpatialTestCrossServerRPC()
	: Super()
{
	Author = "Andrei / Victoria";
	Description = TEXT("Test CrossServer RPCs");
}

void ASpatialTestCrossServerRPC::PrepareTest()
{
	Super::PrepareTest();

	// Pre-test checks
	AddStep(TEXT("Pre-test check"), FWorkerDefinition::Server(1), nullptr, [this]() {
		USpatialNetDriver* SpatialNetDriver = Cast<USpatialNetDriver>(GetNetDriver());

		if (SpatialNetDriver == nullptr || SpatialNetDriver->LoadBalanceStrategy == nullptr)
		{
			FinishTest(EFunctionalTestResult::Error, TEXT("Test requires SpatialOS enabled with Load-Balancing Strategy"));
		}
		else
		{
			FinishStep();
		}
	});

	// Startup actor tests
	//
	// Repro the error case of the entity ID being incorrectly allocated on a non-auth server - expect warnings
	if (HasAuthority())
	{
		// Expect this error from each non-authoritative server
		AddExpectedLogError(TEXT("before receiving entity from runtime. This RPC will be dropped. Please update code execution to wait for "
								 "actor ready state"),
							3, false);
	}
	AddStep(TEXT("Startup actor tests: repro error case for entity ID after RPC on non-auth server"), FWorkerDefinition::AllServers,
			nullptr, [this]() {
				int LocalWorkerId = GetLocalWorkerId();

				// These specific steps were needed to recreate an error of the entity ID being incorrectly allocated on a non-auth server.
				// The level actor is located on server 4 in the map and is not replicated initially. Therefore, all actors are initially
				// authoritative. First we turn on replication on the level actor, then we change it to non-auth and finally send a cross
				// server RPC. It is the ProcessRPC call that was causing an incorrect entity ID to be allocated in this specific case but
				// this has now been fixed and we are expecting a warning error in this case.
				if (LocalWorkerId != 4)
				{
					LevelCube->TurnOnReplication();
					LevelCube->SetNonAuth();
					LevelCube->CrossServerTestRPC(LocalWorkerId);
				}
				FinishStep();
			});

	AddStep(TEXT("Startup actor tests: Post-RPC entity ID check"), FWorkerDefinition::AllServers, nullptr, nullptr,
			[this](float DeltaTime) {
				// Before the fix, incorrect entity IDs were generated on non-auth servers 1, 2 and 3. So this step confirms that the entity
				// IDs remain unset. Also, it confirms the entity ID is unset on server 4 the auth server before we do the next step of
				// turning on replication which correctly sets the entity ID on all servers.
				CheckInvalidEntityID(LevelCube);
			});

	// Normal case - on server which should have authority if replication was initially on
	AddStep(TEXT("Startup actor tests: Auth server - Set replicated"), FWorkerDefinition::Server(4), nullptr, [this]() {
		LevelCube->TurnOnReplication();
		FinishStep();
	});

	AddStep(
		TEXT("Startup actor tests: Auth server - Record entity id"), FWorkerDefinition::Server(4),
		[this]() -> bool {
			// Make sure actor is ready before recording the entity id
			return LevelCube->IsActorReady();
		},
		[this]() {
			LevelCube->RecordEntityId();
			FinishStep();
		});

	AddStep(
		TEXT("Startup actor tests: Post-Auth entity ID check"), FWorkerDefinition::AllServers,
		[this]() -> bool {
			// Make sure actor is ready before checking the entity id
			return LevelCube->IsActorReady();
		},
		nullptr,
		[this](float DeltaTime) {
			CheckValidEntityID(LevelCube);
		});

	// Clean up
	AddStep(TEXT("Startup actor tests: Auth server - Destroy startup actor"), FWorkerDefinition::Server(4), nullptr, [this]() {
		LevelCube->Destroy();
		LevelCube = nullptr;
		FinishStep();
	});

	// Dynamic actor tests
	TArray<FVector> CubesLocations;
	CubesLocations.Add(FVector(250.0f, 250.0f, 75.0f));
	CubesLocations.Add(FVector(250.0f, -250.0f, 75.0f));
	CubesLocations.Add(FVector(-250.0f, -250.0f, 75.0f));
	CubesLocations.Add(FVector(-250.0f, 250.0f, 75.0f));

	for (int i = 1; i <= CubesLocations.Num(); ++i)
	{
		FVector SpawnPosition = CubesLocations[i - 1];
		// Each server spawns a cube
		AddStep(TEXT("Dynamic actor tests: ServerSetupStep"), FWorkerDefinition::Server(i), nullptr, [this, SpawnPosition]() {
			ACrossServerRPCCube* TestCube =
				GetWorld()->SpawnActor<ACrossServerRPCCube>(SpawnPosition, FRotator::ZeroRotator, FActorSpawnParameters());
			RegisterAutoDestroyActor(TestCube);
			FinishStep();
		});
	}

	AddStep(TEXT("Dynamic actor tests: Auth server - Record entity id"), FWorkerDefinition::AllServers, nullptr, [this]() {
		TArray<AActor*> TestCubes;
		UGameplayStatics::GetAllActorsOfClass(GetWorld(), ACrossServerRPCCube::StaticClass(), TestCubes);

		int LocalWorkerId = GetLocalWorkerId();

		for (AActor* Cube : TestCubes)
		{
			if (Cube->HasAuthority())
			{
				ACrossServerRPCCube* CrossServerRPCCube = Cast<ACrossServerRPCCube>(Cube);
				CrossServerRPCCube->RecordEntityId();
			}
		}
		FinishStep();
	});

	int NumCubes = CubesLocations.Num();

	// Each server sends an RPC to all cubes that it is NOT authoritive over.
	AddStep(
		TEXT("Dynamic actor tests: ServerSendRPCs"), FWorkerDefinition::AllServers,
		[this, NumCubes]() -> bool {
			// Make sure that all cubes were spawned and are visible to all servers before trying to send the RPCs.
			TArray<AActor*> TestCubes;
			UGameplayStatics::GetAllActorsOfClass(GetWorld(), ACrossServerRPCCube::StaticClass(), TestCubes);

			UAbstractLBStrategy* LBStrategy = Cast<USpatialNetDriver>(GetNetDriver())->LoadBalanceStrategy;

			// Since the servers are spawning the cubes in positions that don't belong to them
			// we need to wait for all the authority changes to happen, and this can take a bit.
			int LocalWorkerId = GetLocalWorkerId();
			int NumCubesWithAuthority = 0;
			int NumCubesShouldHaveAuthority = 0;
			for (AActor* Cube : TestCubes)
			{
				if (Cube->HasAuthority())
				{
					NumCubesWithAuthority += 1;
				}
				if (LBStrategy->WhoShouldHaveAuthority(*Cube) == LocalWorkerId)
				{
					NumCubesShouldHaveAuthority += 1;
				}
			}

			// So only when we have all cubes present and we only have authority over the one we should we can progress.
			return TestCubes.Num() == NumCubes && NumCubesWithAuthority == 1 && NumCubesShouldHaveAuthority == 1;
		},
		[this]() {
			TArray<AActor*> TestCubes;
			UGameplayStatics::GetAllActorsOfClass(GetWorld(), ACrossServerRPCCube::StaticClass(), TestCubes);

			int LocalWorkerId = GetLocalWorkerId();

			for (AActor* Cube : TestCubes)
			{
				if (!Cube->HasAuthority())
				{
					ACrossServerRPCCube* CrossServerRPCCube = Cast<ACrossServerRPCCube>(Cube);
					CrossServerRPCCube->CrossServerTestRPC(LocalWorkerId);
				}
			}

			FinishStep();
		});

	// Server 1 checks if all cubes received the expected number of RPCs.
	AddStep(
		TEXT("Dynamic actor tests: Server1CheckRPCs"), FWorkerDefinition::Server(1), nullptr, nullptr,
		[this](float DeltaTime) {
			TArray<AActor*> TestCubes;
			UGameplayStatics::GetAllActorsOfClass(GetWorld(), ACrossServerRPCCube::StaticClass(), TestCubes);

			int CorrectCubes = 0;

			for (AActor* Cube : TestCubes)
			{
				ACrossServerRPCCube* CrossServerRPCCube = Cast<ACrossServerRPCCube>(Cube);

				int ReceivedRPCS = CrossServerRPCCube->ReceivedCrossServerRPCS.Num();

				if (ReceivedRPCS == TestCubes.Num() - 1)
				{
					CorrectCubes++;
				}
			}

			if (CorrectCubes == TestCubes.Num())
			{
				FinishStep();
			}
		},
		10.0f);

	AddStep(TEXT("Dynamic actor tests: Post-RPC entity ID check"), FWorkerDefinition::Server(1), nullptr, [this]() {
		TArray<AActor*> TestCubes;
		UGameplayStatics::GetAllActorsOfClass(GetWorld(), ACrossServerRPCCube::StaticClass(), TestCubes);

		for (AActor* Cube : TestCubes)
		{
			ACrossServerRPCCube* CrossServerRPCCube = Cast<ACrossServerRPCCube>(Cube);
			CheckValidEntityID(CrossServerRPCCube);
		}
	});
}

void ASpatialTestCrossServerRPC::CheckInvalidEntityID(ACrossServerRPCCube* TestCube)
{
	USpatialNetDriver* SpatialNetDriver = Cast<USpatialNetDriver>(GetNetDriver());
	Worker_EntityId Entity = SpatialNetDriver->PackageMap->GetEntityIdFromObject(TestCube);
	RequireTrue((Entity == SpatialConstants::INVALID_ENTITY_ID), TEXT("Not expecting a valid entity ID"));
	FinishStep();
}

void ASpatialTestCrossServerRPC::CheckValidEntityID(ACrossServerRPCCube* TestCube)
{
	USpatialNetDriver* SpatialNetDriver = Cast<USpatialNetDriver>(GetNetDriver());
	Worker_EntityId Entity = SpatialNetDriver->PackageMap->GetEntityIdFromObject(TestCube);
	RequireTrue((Entity != SpatialConstants::INVALID_ENTITY_ID), TEXT("Expected a valid entity ID"));
	RequireTrue((Entity == TestCube->AuthEntityId), TEXT("Expected entity ID to be the same as the auth server"));
	FinishStep();
}
