// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "SpatialTestNetOwnership.h"
#include "NetOwnershipCube.h"
#include "SpatialFunctionalTestFlowController.h"

#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"

/**
 * This test automates the Client Net Ownership gym which demonstrates that in a zoned environment, setting client net-ownership of an Actor allows server RPCs to be sent correctly.
 * The test runs with the BP_QuadrantZoningSettings and therefore includes 4 servers and 2 client workers.
 * NOTE: This test requires the map it runs in to use the BP_QuadrantZoningSettings or any other setting that creates a zoned environment in order to be relevant.
 *
 * The flow of the test has 2 phases, as follows:
 * - Phase 1 setup:
 *  - Server 1 spawns a NetOwnershipCube.
 *  - All workers set the reference to the spawned NetOwnershipCube.
 *  - The server that is authoritative over the NetOwnershipCube sets its owner to the PlayerController of Client 1.
 *  - The NetOwnershipCube, toghether with Client1's possesed Pawn, is moved by the server that has authority over it to 2 different test locations, such that it changes the authoritative server once.
 *  - Client 1 sends an RPC from the NetOwnershipCube at each test location.
 * - Phase 1 test:
 *  - All workers test that all sent RPCs have been received by the NetOwnershipCube.
 *
 * - Phase 2 setup
 *  - The authoritative server sets the NetOwnershipCube's Owner to null.
 *  - Client 1 sends an RPC from the NetOwnershipCube.
 * - Phase 2 test:
 *  - All workers test that the RPC sent was ignored.
 * 
 * - Test Cleanup
 *  - The NetOwnershipCube is destroyed.
 */

ASpatialTestNetOwnership::ASpatialTestNetOwnership()
	: Super()
{
	Author = "Andrei";
	Description = TEXT("Test Net Ownership");
}

void ASpatialTestNetOwnership::BeginPlay()
{
	Super::BeginPlay();

	// Step definition for Client 1 to send a Server RPC
	FSpatialFunctionalTestStepDefinition ClientSendRPCStepDefinition;
	ClientSendRPCStepDefinition.bIsNativeDefinition = true;
	ClientSendRPCStepDefinition.TimeLimit = 5.0f;
	ClientSendRPCStepDefinition.NativeStartEvent.BindLambda([this](ASpatialFunctionalTest* NetTest)
		{
			NetOwnershipCube->ServerIncreaseRPCCount();

			FinishStep();
		});

	// Test Phase 1

	// Server 1 spawns the NetOwnershipCube and registers it for auto-destroy.
	AddStep(TEXT("SpatialTestNetOwnershipServerSpawnCube"), FWorkerDefinition::Server(1), nullptr, [this](ASpatialFunctionalTest* NetTest)
		{
			ANetOwnershipCube* Cube = GetWorld()->SpawnActor<ANetOwnershipCube>(FVector::ZeroVector, FRotator::ZeroRotator, FActorSpawnParameters());
			RegisterAutoDestroyActor(Cube);

			FinishStep();
		});

	AddStep(TEXT("SpatialTestNetOwnershipAllWorkersSetReference"), FWorkerDefinition::AllWorkers, nullptr, nullptr, [this](ASpatialFunctionalTest* NetTest, float DeltaTime)
		{
			TArray<AActor*> NetOwnershipCubes;
			UGameplayStatics::GetAllActorsOfClass(GetWorld(), ANetOwnershipCube::StaticClass(), NetOwnershipCubes);

			// Make sure the cube is visible before trying to set the Test's variable.
			if (NetOwnershipCubes.Num() != 1)
			{
				return;
			}

			NetOwnershipCube = Cast<ANetOwnershipCube>(NetOwnershipCubes[0]);

			FinishStep();
		}, 5.0f);

	//  The authoritative server sets the owner of the NetOwnershipCube to Client's 1 PlayerController
	AddStep(TEXT("SpatialTestNetOwnershipServerSetOwner"), FWorkerDefinition::AllServers, nullptr, [this](ASpatialFunctionalTest* NetTest)
		{
			if (NetOwnershipCube->HasAuthority())
			{
				APlayerController* PlayerController = Cast<APlayerController>(GetFlowController(ESpatialFunctionalTestWorkerType::Client, 1)->GetOwner());
				NetOwnershipCube->SetOwner(PlayerController);
			}

			FinishStep();
		});

	// The locations where the NetOwnershipCube will be when Client 1 will send an RPC. These are specifically set to make the NetOwnershipCube's authoritative server change according to the BP_QuadrantZoningSettings.
	TArray<FVector> TestLocations;
	TestLocations.Add(FVector(250.0f, -250.0f, 0.0f));
	TestLocations.Add(FVector(-250.0f, -250.0f, 0.0f));

	for (int i = 1; i <= TestLocations.Num(); ++i)
	{
		// The authoritative server moves the cube and Client1's Pawn to the corresponding test location
		AddStep(TEXT("SpatialTestNetOwnershipServerMoveCube"), FWorkerDefinition::AllServers, nullptr, [this, i, TestLocations](ASpatialFunctionalTest* NetTest)
			{	
				APlayerController* PlayerController = Cast<APlayerController>(GetFlowController(ESpatialFunctionalTestWorkerType::Client, 1)->GetOwner());
				APawn* PlayerPawn = PlayerController->GetPawn();

				if (PlayerPawn->HasAuthority())
				{
					PlayerPawn->SetActorLocation(TestLocations[i - 1]);
				}

				if (NetOwnershipCube->HasAuthority())
				{
					NetOwnershipCube->SetActorLocation(TestLocations[i - 1]);
				}

				FinishStep();
			});

		//  Client 1 sends a ServerRPC from the Cube.
		AddStepFromDefinition(ClientSendRPCStepDefinition, FWorkerDefinition::Client(1));

		//  All workers check that the number of RPCs received by the server authoritative over NetOwnershipCube the is correct.
		AddStep(TEXT("SpatialTestNetOwnershipAllWorkersTestCount"), FWorkerDefinition::AllWorkers, nullptr, nullptr, [this, i](ASpatialFunctionalTest* NetTest, float DeltaTime)
			{
				if (NetOwnershipCube->ReceivedRPCs == i)
				{
					FinishStep();
				}
			}, 5.0f);
	}

	// Test Phase 2

	//  The authoritative server sets the owner of the cube to be nullptr
	AddStep(TEXT("SpatialTestNetOwnershipServerSetOwnerToNull"), FWorkerDefinition::AllServers, nullptr, [this](ASpatialFunctionalTest* NetTest)
		{
			if (NetOwnershipCube->HasAuthority())
			{
				NetOwnershipCube->SetOwner(nullptr);
			}

			FinishStep();
		});

	//  Client 1 sends a ServerRPC from the NetOwnershipCube that should be ignored.
	AddStepFromDefinition(ClientSendRPCStepDefinition, FWorkerDefinition::Client(1));

	//  All workers check that the number of RPCs received by the authoritative server is correct.
	AddStep(TEXT("SpatialTestNetOwnershipAllWorkersTestCount2"), FWorkerDefinition::AllWorkers, nullptr, nullptr, [this, TestLocations](ASpatialFunctionalTest* NetTest, float DeltaTime)
		{
			if (NetOwnershipCube->ReceivedRPCs == TestLocations.Num())
			{
				FinishStep();
			}
		}, 5.0f);
}
