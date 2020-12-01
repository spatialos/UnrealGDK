// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "SpatialTestCrossServerRPC.h"
#include "CrossServerRPCCube.h"
#include "EngineClasses/SpatialNetDriver.h"
#include "Kismet/GameplayStatics.h"
#include "LoadBalancing/AbstractLBStrategy.h"
#include "SpatialFunctionalTestFlowController.h"

/**
 * This test automates the Server to server RPC gym, that was used to demonstrate that actors owned by different servers correctly send
 * server-to-server RPCs. The test includes 4 server workers and 2 clients. NOTE: This test requires the map it runs in to have the
 * BP_QuadrantLBStrategy and OwnershipLockingPolicy set in order to be relevant.
 *
 * The flow is as follows:
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
	Author = "Andrei";
	Description = TEXT("Test CrossServer RPCs");
}

void ASpatialTestCrossServerRPC::PrepareTest()
{
	Super::PrepareTest();

	TArray<FVector> CubesLocations;
	CubesLocations.Add(FVector(250.0f, 250.0f, 75.0f));
	CubesLocations.Add(FVector(250.0f, -250.0f, 75.0f));
	CubesLocations.Add(FVector(-250.0f, -250.0f, 75.0f));
	CubesLocations.Add(FVector(-250.0f, 250.0f, 75.0f));

	AddStep(TEXT("EnsureSpatialOS"), FWorkerDefinition::Server(1), nullptr, [this]() {
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

	for (int i = 1; i <= 4; ++i)
	{
		FVector SpawnPosition = CubesLocations[i - 1];
		// Each server spawns a cube
		AddStep(TEXT("ServerSetupStep"), FWorkerDefinition::Server(i), nullptr, [this, SpawnPosition]() {
			ACrossServerRPCCube* TestCube =
				GetWorld()->SpawnActor<ACrossServerRPCCube>(SpawnPosition, FRotator::ZeroRotator, FActorSpawnParameters());
			RegisterAutoDestroyActor(TestCube);

			FinishStep();
		});
	}

	int NumCubes = CubesLocations.Num();

	// Each server sends an RPC to all cubes that it is NOT authoritive over.
	AddStep(
		TEXT("ServerSendRPCs"), FWorkerDefinition::AllServers,
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
		TEXT("Server1CheckRPCs"), FWorkerDefinition::Server(1), nullptr, nullptr,
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
}
