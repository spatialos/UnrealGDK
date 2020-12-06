// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "RelevancyTest.h"
#include "SpatialFunctionalTestFlowController.h"
#include "SpatialGDKFunctionalTests/SpatialGDK/TestActors/RelevancyTestActors.h"
#include "SpatialGDKFunctionalTests/SpatialGDK/TestActors/TestMovementCharacter.h"

#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "LoadBalancing/LayeredLBStrategy.h"

/**
 * This test tests if a bHidden Actor is replicating properly to Server and Clients.
 *
 * The test includes a single server and two client workers.
 * The flow is as follows:
 *  - Setup:
 *	  - One cube actor already placed in the level at Location FVector(0.0f, 0.0f, 80.0f).
 *    - The Server spawns a TestMovementCharacter and makes Client 1 possess it.
 *  - Test:
 *    - Each worker tests if it can initially see the AReplicatedVisibilityTestActor.
 *    - After ensuring possession happened, the Server moves Client 1's Character to a remote location, so it cannot see the
 *AReplicatedVisibilityTestActor.
 *    - After ensuring movement replicated correctly, Client 1 checks it can no longer see the AReplicatedVisibilityTestActor.
 *	  - The Server sets the AReplicatedVisibilityTestActor to hidden.
 *	  - All Clients check they can no longer see the AReplicatedVisibilityTestActor.
 *	  - The Server moves the character of Client 1 back close to its spawn location, so that the AReplicatedVisibilityTestActor is in its
 *interest area.
 *	  - All Clients check they can still not see the AReplicatedVisibilityTestActor.
 *	  - The Server sets the AReplicatedVisibilityTestActor to not be hidden.
 *	  - All Clients check they can now see the AReplicatedVisibilityTestActor.
 *  - Cleanup:
 *    - Client 1 repossesses its default pawn.
 *    - The spawned Character is destroyed.
 */

const static float StepTimeLimit = 10.0f;

ARelevancyTest::ARelevancyTest()
	: Super()
{
	Author = "Mike";
	Description = TEXT("Test Actor Relevancy");
}

template <typename T>
int GetNumberOfActorsOfType(UWorld* World)
{
	int Counter = 0;
	for (TActorIterator<T> Iter(World); Iter; ++Iter)
	{
		Counter++;
	}

	return Counter;
}

void ARelevancyTest::PrepareTest()
{
	Super::PrepareTest();

	GetNumberOfActorsOfType<AAlwaysRelevantTestActor>(GetWorld());

	{ // Step 0 - Spawn actors on each server
		AddStep(TEXT("VisibilityTestServerSetup"), FWorkerDefinition::AllServers, nullptr, [this]() {
			ULayeredLBStrategy* RootStrategy = GetLoadBalancingStrategy();
			UAbstractLBStrategy* DefaultStrategy = RootStrategy->GetLBStrategyForLayer(SpatialConstants::DefaultLayer);
			UGridBasedLBStrategy* GridStrategy = Cast<UGridBasedLBStrategy>(DefaultStrategy);
			AssertTrue(GridStrategy != nullptr, TEXT("Invalid LBS"));
			const FVector WorkerPos = GridStrategy->GetWorkerEntityPosition();

			AlwaysRelevantActor =
				GetWorld()->SpawnActor<AAlwaysRelevantTestActor>(WorkerPos, FRotator::ZeroRotator, FActorSpawnParameters());
			AlwaysRelevantServerOnlyActor =
				GetWorld()->SpawnActor<AAlwaysRelevantServerOnlyTestActor>(WorkerPos, FRotator::ZeroRotator, FActorSpawnParameters());

			FinishStep();
		});
	}

	{ // Step 1 - Check actors are ready on each server
		AddStep(
			TEXT("VisibilityTestServerSetup"), FWorkerDefinition::AllServers,
			[this]() -> bool {
				return (AlwaysRelevantActor->IsActorReady() && AlwaysRelevantServerOnlyActor->IsActorReady());
			},
			[this]() {
				FinishStep();
			});
	}

	{ // Step 2 - Check actors count is correct on servers
		AddStep(
			TEXT("VisibilityTestServerSetup"), FWorkerDefinition::AllServers, nullptr, nullptr,
			[this](float DeltaTime) {
				int NumAlwaysRelevantActors = GetNumberOfActorsOfType<AAlwaysRelevantTestActor>(GetWorld());
				int NumAlwaysServerOnlyRelevantActors = GetNumberOfActorsOfType<AAlwaysRelevantServerOnlyTestActor>(GetWorld());
				int NumClients = GetNumberOfClientWorkers();
				int NumServers = GetNumberOfServerWorkers();

				if (NumAlwaysRelevantActors == NumServers && NumAlwaysServerOnlyRelevantActors == NumServers)
				{
					FinishStep();
				}
			},
			StepTimeLimit);
	}

	{ // Step 3 - Check actors count is correct on clients
		AddStep(
			TEXT("VisibilityTestServerSetup"), FWorkerDefinition::AllClients, nullptr, nullptr,
			[this](float DeltaTime) {
				int NumAlwaysRelevantActors = GetNumberOfActorsOfType<AAlwaysRelevantTestActor>(GetWorld());
				int NumAlwaysServerOnlyRelevantActors = GetNumberOfActorsOfType<AAlwaysRelevantServerOnlyTestActor>(GetWorld());
				int NumServers = GetNumberOfServerWorkers();

				if (NumAlwaysRelevantActors == NumServers && NumAlwaysServerOnlyRelevantActors == 0)
				{
					FinishStep();
				}
			},
			StepTimeLimit);
	}
}
