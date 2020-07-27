// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "SpatialTestCrossServerRPC.h"
#include "CrossServerRPCCube.h"
#include "Kismet/GameplayStatics.h"
#include "SpatialFunctionalTestFlowController.h"

/**
 * This test automates the Server to server RPC gym, that was used to demonstrate that actors owned by different servers correctly send server-to-server RPCs.
 * The test includes 4 server workers and 2 clients.
 * NOTE: This test requires the map it runs in to have the BP_QuadrantLBStrategy and OwnershipLockingPolicy set in order to be relevant. 
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

void ASpatialTestCrossServerRPC::BeginPlay()
{
	Super::BeginPlay();

	TArray<FVector> CubesLocations;
	CubesLocations.Add(FVector(250.0f, 250.0f, 75.0f));
	CubesLocations.Add(FVector(250.0f, -250.0f, 75.0f));
	CubesLocations.Add(FVector(-250.0f, -250.0f, 75.0f));
	CubesLocations.Add(FVector(-250.0f, 250.0f, 75.0f));

	for (int i = 1; i <= 4; ++i)
	{
		// Each server spawns a cube
		AddStep(TEXT("ServerSetupStep"), FWorkerDefinition::Server(i), nullptr, [this, i, CubesLocations](ASpatialFunctionalTest* NetTest)
			{
				ACrossServerRPCCube* TestCube = GetWorld()->SpawnActor<ACrossServerRPCCube>(CubesLocations[i-1], FRotator::ZeroRotator, FActorSpawnParameters());
				RegisterAutoDestroyActor(TestCube);

				FinishStep();
			});
	}

	// Each server sends an RPC to all cubes that it is NOT authoritive over.
	AddStep(TEXT("ServerSendRPCs"), FWorkerDefinition::AllServers, [this, CubesLocations](ASpatialFunctionalTest* NetTest)
		{
			// Make sure that all cubes were spawned and are visible to all servers before trying to send the RPCs.
			TArray<AActor*> TestCubes;
			UGameplayStatics::GetAllActorsOfClass(GetWorld(), ACrossServerRPCCube::StaticClass(), TestCubes);

			return TestCubes.Num() == CubesLocations.Num();
		}, [this](ASpatialFunctionalTest* NetTest)
		{
			TArray<AActor*> TestCubes;
			UGameplayStatics::GetAllActorsOfClass(GetWorld(), ACrossServerRPCCube::StaticClass(), TestCubes);

			for (AActor* CubeWithAuthority : TestCubes)
			{
				if (CubeWithAuthority->HasAuthority())
				{
					for (AActor* OtherActor : TestCubes)
					{
						if (CubeWithAuthority != OtherActor)
						{
							ACrossServerRPCCube* OtherCube = Cast<ACrossServerRPCCube>(OtherActor);
							OtherCube->CrossServerTestRPC(GetLocalFlowController()->GetWorkerDefinition().Id);
						}
					}

					break;
				}
			}

			FinishStep();
		});

	// Server 1 checks if all cubes received the expected number of RPCs.
	AddStep(TEXT("Server1CheckRPCs"), FWorkerDefinition::Server(1), nullptr, nullptr, [this](ASpatialFunctionalTest* NetTest, float DeltaTime)
		{
			TArray<AActor*> TestCubes;
			UGameplayStatics::GetAllActorsOfClass(GetWorld(), ACrossServerRPCCube::StaticClass(), TestCubes);

			int TotalNumberOfCubes = TestCubes.Num();
			int CorrectCubes = 0;

			for (int i = 0; i < TotalNumberOfCubes; ++i)
			{
				ACrossServerRPCCube* CurrentCube = Cast<ACrossServerRPCCube>(TestCubes[i]);

				int ReceivedRPCS = CurrentCube->ReceivedCrossServerRPCS.Num();

				if (ReceivedRPCS == TotalNumberOfCubes - 1)
				{
					AssertEqual_Int(ReceivedRPCS, TotalNumberOfCubes - 1, FString::Printf(TEXT("The cube with index %d has received all the expected RPCs!"), i));
					CorrectCubes++;
				}
			}

			if (CorrectCubes == TestCubes.Num())
			{
				FinishStep();
			}

		}, 5.0f);
}

