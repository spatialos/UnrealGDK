// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "SpatialTestHandover.h"
#include "Kismet/GameplayStatics.h"
#include "EngineClasses/SpatialNetDriver.h"
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

	SpawnLocation = FVector(-500.0f, -500.0f, 50.0f);

	TestLocations.Add(FVector(500.0f, -500.0f, 50.0f));
	TestLocations.Add(FVector(500.0f, 500.0f, 50.0f)); 
	TestLocations.Add(FVector(-500.0f, 500.0f, 50.0f));
}

void ASpatialTestHandover::BeginPlay()
{
	Super::BeginPlay();

	// Step Definition to move the HandoverCube to the corresponding test location.
	FSpatialFunctionalTestStepDefinition MoveCubeStepDefinition;
	MoveCubeStepDefinition.bIsNativeDefinition = true;
	MoveCubeStepDefinition.NativeStartEvent.BindLambda([this]()
		{
			if (LocationIndex >= TestLocations.Num() || LocationIndex < 0)
			{
				return;
			}

			if (HandoverCube->HasAuthority())
			{
				HandoverCube->SetActorLocation(TestLocations[LocationIndex]);
			}

			++LocationIndex;
			FinishStep();
		});

	// Step Definition to check if the correct server is authoritative over the HandoverCube.
	FSpatialFunctionalTestStepDefinition CheckAuthorityStepDefinition;
	CheckAuthorityStepDefinition.bIsNativeDefinition = true;
	CheckAuthorityStepDefinition.TimeLimit = 10.0f;
	CheckAuthorityStepDefinition.NativeTickEvent.BindLambda([this](float DeltaTime)
		{
			// If we have no locking server, then the HandoverCube is expected to have changed authority.
			if (HandoverCube->LockingServerID == 0)
			{
				// Early out if the HandoverCube did not change authority.
				if (HandoverCube->AuthorityChanges != AuthorityChanges + 1)
				{
					return;
				}
			}
			else
			{
				// If there is a locking server, then the HandoverCube should have not changed authority, therefore we early out if it has changed authority.
				if (HandoverCube->AuthorityChanges != AuthorityChanges)
				{
					return;
				}
			}

			// Update the number of expected AuthorityChanges the HandoverCube should have had so far.
			if (HandoverCube->LockingServerID == 0)
			{
				++AuthorityChanges;
			}

			// If a Server Worker holds a lock over the HandoverCube, then it should have authority over it, if not, let the Load Balancer decide.
			int ExpectedAuthoritativeServer = HandoverCube->LockingServerID ? HandoverCube->LockingServerID : LoadBalancingStrategy->WhoShouldHaveAuthority(*HandoverCube);
			
			// Make sure the correct Server has authority over the HandoverCube.
			if (GetLocalFlowController()->WorkerDefinition.Id == ExpectedAuthoritativeServer)
			{
				if (HandoverCube->HasAuthority())
				{
					FinishStep();
				}
			}
			else
			{
				// Also ensure that all other Servers are not authoritative over the HandoverCube.
				if (!HandoverCube->HasAuthority())
				{
					FinishStep();
				}
			}
		});

	// Server 1 spawns the HandoverCube under its authority area.
	AddStep(TEXT("SpatialTestHandoverServer1SpawnCube"), FWorkerDefinition::Server(1), nullptr, [this]()
		{
			HandoverCube = GetWorld()->SpawnActor<AHandoverCube>(SpawnLocation, FRotator::ZeroRotator, FActorSpawnParameters());
			RegisterAutoDestroyActor(HandoverCube);

			FinishStep();
		});

	// All servers set a reference to the HandoverCube and reset the LocationIndex and AuthorityCheckIndex.
	AddStep(TEXT("SpatialTestHandoverAllServersSetupStep"), FWorkerDefinition::AllServers, nullptr, nullptr, [this](float DeltaTime)
		{
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
					LocationIndex = 0;
					AuthorityChanges = 0;
					FinishStep();
				}
			}
		}, 5.0f);

	// Check that Server 1 is authoritative over the HandoverCube.
	AddStepFromDefinition(CheckAuthorityStepDefinition, FWorkerDefinition::AllServers);

	// Move the HandoverCube to the next location, which is inside the authority area of Server 2.
	AddStepFromDefinition(MoveCubeStepDefinition, FWorkerDefinition::AllServers);

	// Check that Server 2 is authoritative over the HandoverCube.
	AddStepFromDefinition(CheckAuthorityStepDefinition, FWorkerDefinition::AllServers);

	// Server 2 acquires a lock on the HandoverCube.
	AddStep(TEXT("SpatialTestHandoverServer2AcquireLock"), FWorkerDefinition::Server(2), nullptr, [this]()
		{
			HandoverCube->AcquireLock(2);
			FinishStep();
		});

	// Move the HandoverCube to the next location, which is inside the authority area of Server 4.
	AddStepFromDefinition(MoveCubeStepDefinition, FWorkerDefinition::AllServers);

	// Check that Server 2 is still  authoritative over the HandoverCube due to acquiring the lock earlier.
	AddStepFromDefinition(CheckAuthorityStepDefinition, FWorkerDefinition::AllServers);

	// Server 2 releases the lock on the HandoverCube.
	AddStep(TEXT("SpatialTestHandoverServer2ReleaseLock"), FWorkerDefinition::Server(2), nullptr, [this]()
		{
			HandoverCube->ReleaseLock();
			FinishStep();
		});

	// Check that Server 4 is now authoritative over the HandoverCube.
	AddStepFromDefinition(CheckAuthorityStepDefinition, FWorkerDefinition::AllServers);

	// Move the HandoverCube to the next location, which is inside the authority area of Server 3.
	AddStepFromDefinition(MoveCubeStepDefinition, FWorkerDefinition::AllServers);

	// Check that Server 3 is now authoritative over the HandoverCube.
	AddStepFromDefinition(CheckAuthorityStepDefinition, FWorkerDefinition::AllServers);
}
