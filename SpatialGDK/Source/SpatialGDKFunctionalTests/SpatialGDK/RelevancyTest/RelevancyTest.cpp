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

			AController* PlayerController = Cast<AController>(GetFlowController(ESpatialFunctionalTestWorkerType::Client, 1)->GetOwner());
			if (!AssertTrue(IsValid(PlayerController), TEXT("Failed to retrieve player controller")))
			{
				return;
			}

			if (PlayerController->HasAuthority())
			{
				OnlyRelevantToOwnerTestActor =
					GetWorld()->SpawnActor<AOnlyRelevantToOwnerTestActor>(WorkerPos, FRotator::ZeroRotator, FActorSpawnParameters());
				UseOwnerRelevancyTestActor =
					GetWorld()->SpawnActor<AUseOwnerRelevancyTestActor>(WorkerPos, FRotator::ZeroRotator, FActorSpawnParameters());

				OnlyRelevantToOwnerTestActor->SetOwner(PlayerController);
				UseOwnerRelevancyTestActor->SetOwner(OnlyRelevantToOwnerTestActor);

				ActiveActors.Add(OnlyRelevantToOwnerTestActor);
				ActiveActors.Add(UseOwnerRelevancyTestActor);

				RegisterAutoDestroyActor(OnlyRelevantToOwnerTestActor);
				RegisterAutoDestroyActor(UseOwnerRelevancyTestActor);
			}

			ActiveActors.Add(AlwaysRelevantActor);
			ActiveActors.Add(AlwaysRelevantServerOnlyActor);

			RegisterAutoDestroyActor(AlwaysRelevantActor);
			RegisterAutoDestroyActor(AlwaysRelevantServerOnlyActor);

			FinishStep();
		});
	}

	{ // Step 1 - Check actors are ready on each server
		AddStep(
			TEXT("RelevancyTestReadyActors"), FWorkerDefinition::AllServers,
			[this]() -> bool {
				const bool bActorNotReady = ActiveActors.ContainsByPredicate([](const AActor* Actor) {
					return !Actor->IsActorReady();
				});
				return !bActorNotReady;
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
				int NumOnlyRelevantToOwnerActors = GetNumberOfActorsOfType<AOnlyRelevantToOwnerTestActor>(GetWorld());
				int NumUseOwnerRelevancyActors = GetNumberOfActorsOfType<AUseOwnerRelevancyTestActor>(GetWorld());
				int NumServers = GetNumberOfServerWorkers();

				RequireEqual_Int(NumAlwaysRelevantActors, NumServers, TEXT("Servers see expected number of always relevant actors"));
				RequireEqual_Int(NumAlwaysServerOnlyRelevantActors, NumServers,
								 TEXT("Servers see expected number of server-only always relevant actors"));
				RequireEqual_Int(NumOnlyRelevantToOwnerActors, 1, TEXT("Servers see expected number of only relevant to owner actors"));
				RequireEqual_Int(NumUseOwnerRelevancyActors, 1, TEXT("Servers see expected number of only relevant to owner actors"));
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

	{ // Step 4 - Check actors count is correct on owning client
		AddStep(
			TEXT("RelevancyTestCountActorsOnClients"), FWorkerDefinition::Client(1), nullptr, nullptr,
			[this](float DeltaTime) {
				int NumOnlyRelevantToOwnerActors = GetNumberOfActorsOfType<AOnlyRelevantToOwnerTestActor>(GetWorld());
				int NumUseOwnerRelevancyActors = GetNumberOfActorsOfType<AUseOwnerRelevancyTestActor>(GetWorld());

				RequireEqual_Int(NumOnlyRelevantToOwnerActors, 1,
								 TEXT("Owning client sees expected number of only relevant to owner actors"));
				RequireEqual_Int(NumUseOwnerRelevancyActors, 1, TEXT("Owning client see expected number of use owner relevancy actors"));
				FinishStep(); // This will only actually finish if requires are satisfied
			},
			StepTimeLimit);
	}

	{ // Step 5 - Check actors count is correct on non-owning client
		AddStep(
			TEXT("RelevancyTestCountActorsOnClients"), FWorkerDefinition::Client(2), nullptr, nullptr,
			[this](float DeltaTime) {
				int NumOnlyRelevantToOwnerActors = GetNumberOfActorsOfType<AOnlyRelevantToOwnerTestActor>(GetWorld());
				int NumUseOwnerRelevancyActors = GetNumberOfActorsOfType<AUseOwnerRelevancyTestActor>(GetWorld());

				RequireEqual_Int(NumOnlyRelevantToOwnerActors, 0,
								 TEXT("Non-owning client see expected number of only relevant to owner actors"));
				RequireEqual_Int(NumUseOwnerRelevancyActors, 0, TEXT("Owning client see expected number of use owner relevancy actors"));
				FinishStep(); // This will only actually finish if requires are satisfied
			},
			StepTimeLimit);
	}
}
