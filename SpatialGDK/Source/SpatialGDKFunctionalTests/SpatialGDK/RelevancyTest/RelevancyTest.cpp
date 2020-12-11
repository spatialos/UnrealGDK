// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "RelevancyTest.h"
#include "SpatialFunctionalTestFlowController.h"
#include "SpatialGDKFunctionalTests/SpatialGDK/TestActors/RelevancyTestActors.h"

#include "LoadBalancing/LayeredLBStrategy.h"

/**
 * This test tests that actors with bAlwaysRelevant are replicated to clients and servers correctly.
 *
 * The test should include two servers and two clients
 * The flow is as follows:
 *  - Setup:
 *    - Each server spawns an AAlwaysRelevantTestActor and an AAlwaysRelevantServerOnlyTestActor
 *  - Test:
 *    - Each server validates they can see all AAlwaysRelevantTestActor and all AAlwaysRelevantServerOnlyTestActor
 *    - Each client validates they can see all AAlwaysRelevantTestActor and no AAlwaysRelevantServerOnlyTestActor
 *  - Cleanup:
 *    - Destroy the actors
 */

const static float StepTimeLimit = 5.0f;

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

	{ // Step 0 - Spawn actors on each server
		AddStep(TEXT("RelevancyTestSpawnActors"), FWorkerDefinition::AllServers, nullptr, [this]() {
			ULayeredLBStrategy* RootStrategy = GetLoadBalancingStrategy();
			UAbstractLBStrategy* DefaultStrategy = RootStrategy->GetLBStrategyForLayer(SpatialConstants::DefaultLayer);
			UGridBasedLBStrategy* GridStrategy = Cast<UGridBasedLBStrategy>(DefaultStrategy);
			AssertIsValid(GridStrategy, TEXT("Invalid LBS"));
			const FVector WorkerPos = GridStrategy->GetWorkerEntityPosition();

			AlwaysRelevantActor =
				GetWorld()->SpawnActor<AAlwaysRelevantTestActor>(WorkerPos, FRotator::ZeroRotator, FActorSpawnParameters());
			AlwaysRelevantServerOnlyActor =
				GetWorld()->SpawnActor<AAlwaysRelevantServerOnlyTestActor>(WorkerPos, FRotator::ZeroRotator, FActorSpawnParameters());

			RegisterAutoDestroyActor(AlwaysRelevantActor);
			RegisterAutoDestroyActor(AlwaysRelevantServerOnlyActor);

			FinishStep();
		});
	}

	{ // Step 1 - Check actors are ready on each server
		AddStep(
			TEXT("RelevancyTestReadyActors"), FWorkerDefinition::AllServers,
			[this]() -> bool {
				return (AlwaysRelevantActor->IsActorReady() && AlwaysRelevantServerOnlyActor->IsActorReady());
			},
			[this]() {
				FinishStep();
			});
	}

	{ // Step 2 - Check actors count is correct on servers
		AddStep(
			TEXT("RelevancyTestCountActorsOnServers"), FWorkerDefinition::AllServers, nullptr, nullptr,
			[this](float DeltaTime) {
				int NumAlwaysRelevantActors = GetNumberOfActorsOfType<AAlwaysRelevantTestActor>(GetWorld());
				int NumAlwaysServerOnlyRelevantActors = GetNumberOfActorsOfType<AAlwaysRelevantServerOnlyTestActor>(GetWorld());
				int NumServers = GetNumberOfServerWorkers();

				RequireEqual_Int(NumAlwaysRelevantActors, NumServers, TEXT("Servers see expected number of always relevant actors"));
				RequireEqual_Int(NumAlwaysServerOnlyRelevantActors, NumServers,
								 TEXT("Servers see expected number of server-only always relevant actors"));
				FinishStep(); // This will only actually finish if requires are satisfied
			},
			StepTimeLimit);
	}

	{ // Step 3 - Check actors count is correct on clients
		AddStep(
			TEXT("RelevancyTestCountActorsOnClients"), FWorkerDefinition::AllClients, nullptr, nullptr,
			[this](float DeltaTime) {
				int NumAlwaysRelevantActors = GetNumberOfActorsOfType<AAlwaysRelevantTestActor>(GetWorld());
				int NumAlwaysServerOnlyRelevantActors = GetNumberOfActorsOfType<AAlwaysRelevantServerOnlyTestActor>(GetWorld());
				int NumServers = GetNumberOfServerWorkers();

				RequireEqual_Int(NumAlwaysRelevantActors, NumServers, TEXT("Client see expected number of always relevant actors"));
				RequireEqual_Int(NumAlwaysServerOnlyRelevantActors, 0, TEXT("Client see no always relevant server-only actors"));
				FinishStep(); // This will only actually finish if requires are satisfied
			},
			StepTimeLimit);
	}
}
