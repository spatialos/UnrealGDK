// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "RelevancyTest.h"
#include "SpatialFunctionalTestFlowController.h"
#include "SpatialGDKFunctionalTests/SpatialGDK/TestActors/RelevancyTestActors.h"
#include "SpatialGDKFunctionalTests/SpatialGDK/TestActors/TestMovementCharacter.h"

#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "LoadBalancing/LayeredLBStrategy.h"

/**
 * This test tests that actors with bAlwaysRelevant are replicated to clients and servers correctly.
 *
 * The test should include two servers and two clients
 * The flow is as follows:
 *  - Setup:
 *  - Test:
 *  - Cleanup:
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
