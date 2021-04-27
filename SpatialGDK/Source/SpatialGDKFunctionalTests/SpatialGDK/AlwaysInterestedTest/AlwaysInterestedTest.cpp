// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "AlwaysInterestedTest.h"
#include "SpatialFunctionalTestFlowController.h"
#include "SpatialGDKFunctionalTests/SpatialGDK/TestActors/AlwaysInterestedTestActors.h"

#include "LoadBalancing/LayeredLBStrategy.h"

#include "Net/UnrealNetwork.h"

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

AAlwaysInterestedTest::AAlwaysInterestedTest()
	: Super()
{
	Author = "Mike";
	Description = TEXT("Test Always Interested Functionality");
}

void AAlwaysInterestedTest::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ThisClass, AlwaysInterestedActor);
	DOREPLIFETIME(ThisClass, InterestedInThisReplicatedActor);
	DOREPLIFETIME(ThisClass, NotInterestedInThisReplicatedActor);
}

void AAlwaysInterestedTest::PrepareTest()
{
	Super::PrepareTest();

	{ // Step 0 - Spawn actors on server 1
		AddStep(TEXT("AlwaysInterested_SpawnActors"), FWorkerDefinition::Server(1), nullptr, [this]() {
			AssertTrue(HasAuthority(), TEXT("Server 1 requires authority over the test actor"));

			ULayeredLBStrategy* RootStrategy = GetLoadBalancingStrategy();
			UAbstractLBStrategy* DefaultStrategy = RootStrategy->GetLBStrategyForLayer(SpatialConstants::DefaultLayer);
			UGridBasedLBStrategy* GridStrategy = Cast<UGridBasedLBStrategy>(DefaultStrategy);
			AssertIsValid(GridStrategy, TEXT("Invalid LBS"));
			const FVector WorkerPos = GridStrategy->GetWorkerEntityPosition();

			AlwaysInterestedActor =
				GetWorld()->SpawnActor<AAlwaysInterestedTestActor>(WorkerPos, FRotator::ZeroRotator, FActorSpawnParameters());

			InterestedInThisReplicatedActor =
				GetWorld()->SpawnActor<ASmallNCDActor>(WorkerPos, FRotator::ZeroRotator, FActorSpawnParameters());

			NotInterestedInThisReplicatedActor =
				GetWorld()->SpawnActor<ASmallNCDActor>(WorkerPos, FRotator::ZeroRotator, FActorSpawnParameters());

			AlwaysInterestedActor->InterestedActors.Push(InterestedInThisReplicatedActor);

			AController* PlayerController = Cast<AController>(GetFlowController(ESpatialFunctionalTestWorkerType::Client, 1)->GetOwner());
			if (!AssertTrue(IsValid(PlayerController), TEXT("Failed to retrieve player controller")))
			{
				return;
			}

			if (!AssertTrue(PlayerController->HasAuthority(), TEXT("Failed to have authority over player controller")))
			{
				return;
			}

			AlwaysInterestedActor->SetOwner(PlayerController);

			RegisterAutoDestroyActor(AlwaysInterestedActor);
			RegisterAutoDestroyActor(InterestedInThisReplicatedActor);
			RegisterAutoDestroyActor(NotInterestedInThisReplicatedActor);

			FinishStep();
		});
	}

	{ // Step 1 - Move actors to server 2
		AddStep(
			TEXT("AlwaysInterested_MoveActors"), FWorkerDefinition::Server(1),
			[this]() -> bool {
				return (AlwaysInterestedActor->IsActorReady() && InterestedInThisReplicatedActor->IsActorReady()
						&& NotInterestedInThisReplicatedActor->IsActorReady());
			},
			[this]() {
				ULayeredLBStrategy* RootStrategy = GetLoadBalancingStrategy();
				UAbstractLBStrategy* DefaultStrategy = RootStrategy->GetLBStrategyForLayer(SpatialConstants::DefaultLayer);
				UGridBasedLBStrategy* GridStrategy = Cast<UGridBasedLBStrategy>(DefaultStrategy);
				AssertIsValid(GridStrategy, TEXT("Invalid LBS"));
				const UGridBasedLBStrategy::LBStrategyRegions WorkerRegions = GridStrategy->GetLBStrategyRegions();
				const VirtualWorkerId LocalWorker = GridStrategy->GetLocalVirtualWorkerId();

				auto GetWorkerPosition = [&](bool bLocalWorker) -> FVector {
					for (const auto& WorkerRegion : WorkerRegions)
					{
						if ((bLocalWorker && WorkerRegion.Key == LocalWorker) || (!bLocalWorker && WorkerRegion.Key != LocalWorker))
						{
							const FVector2D Centre = WorkerRegion.Value.GetCenter();
							return FVector{ Centre.X, Centre.Y, 0.f };
						}
					}
					return {};
				};

				FVector LocalWorkerPosition = GetWorkerPosition(true);
				FVector OtherWorkerPosition = GetWorkerPosition(false);

				// Move both interested actors to server 2's area
				InterestedInThisReplicatedActor->SetActorLocation(OtherWorkerPosition);
				NotInterestedInThisReplicatedActor->SetActorLocation(OtherWorkerPosition);

				FinishStep();
			});
	}

	{ // Step 2 - Validate visibility on server 2
		AddStep(
			TEXT("AlwaysInterested_ValidateVisibilityOnServer2"), FWorkerDefinition::Server(2), nullptr, nullptr,
			[this](float DeltaTime) {
				RequireTrue(!IsValid(AlwaysInterestedActor), TEXT("Shouldn't see always interested actor"));
				RequireTrue(IsValid(InterestedInThisReplicatedActor), TEXT("Should see interested actor via authority"));
				RequireTrue(IsValid(NotInterestedInThisReplicatedActor), TEXT("Should see not interested actor via authority"));
				FinishStep(); // This will only actually finish if requires are satisfied
			},
			StepTimeLimit);
	}

	{ // Step 3 - Validate visibility on server 1
		AddStep(
			TEXT("AlwaysInterested_ValidateVisibilityOnServer1"), FWorkerDefinition::Server(1), nullptr, nullptr,
			[this](float DeltaTime) {
				RequireTrue(IsValid(AlwaysInterestedActor), TEXT("Should see always interested actor via authority"));
				RequireTrue(IsValid(InterestedInThisReplicatedActor), TEXT("Should see interested actor via always interest"));
				RequireTrue(!IsValid(NotInterestedInThisReplicatedActor), TEXT("Shouldn't see not interested actor"));
				FinishStep(); // This will only actually finish if requires are satisfied
			},
			StepTimeLimit);
	}

	{ // Step 4 - Validate visibility on owning client
		AddStep(
			TEXT("AlwaysInterested_ValidateVisibilityOnOwningClient"), FWorkerDefinition::Client(1), nullptr, nullptr,
			[this](float DeltaTime) {
				RequireTrue(IsValid(AlwaysInterestedActor), TEXT("Should see actor not in view"));
				RequireTrue(IsValid(InterestedInThisReplicatedActor), TEXT("Should see actor in view"));
				RequireTrue(!IsValid(NotInterestedInThisReplicatedActor), TEXT("Shouldn't see actor in view"));
				FinishStep(); // This will only actually finish if requires are satisfied
			},
			StepTimeLimit);
	}

	{ // Step 5 - Validate visibility on non-owning client
		AddStep(
			TEXT("AlwaysInterested_ValidateVisibilityOnNonOwningClient"), FWorkerDefinition::Client(2), nullptr, nullptr,
			[this](float DeltaTime) {
				RequireTrue(!IsValid(AlwaysInterestedActor), TEXT("Shouldn't see actor not in view"));
				RequireTrue(!IsValid(InterestedInThisReplicatedActor), TEXT("Shouldn't see actor not in view"));
				RequireTrue(!IsValid(NotInterestedInThisReplicatedActor), TEXT("Shouldn't see actor not in view"));
				FinishStep(); // This will only actually finish if requires are satisfied
			},
			StepTimeLimit);
	}

	{ // Step 6 - Move actors back to original server
		AddStep(TEXT("AlwaysInterestedMoveActorsBack"), FWorkerDefinition::Server(2), nullptr, [this]() {
			ULayeredLBStrategy* RootStrategy = GetLoadBalancingStrategy();
			UAbstractLBStrategy* DefaultStrategy = RootStrategy->GetLBStrategyForLayer(SpatialConstants::DefaultLayer);
			UGridBasedLBStrategy* GridStrategy = Cast<UGridBasedLBStrategy>(DefaultStrategy);
			AssertIsValid(GridStrategy, TEXT("Invalid LBS"));
			const UGridBasedLBStrategy::LBStrategyRegions WorkerRegions = GridStrategy->GetLBStrategyRegions();
			const VirtualWorkerId LocalWorker = GridStrategy->GetLocalVirtualWorkerId();

			auto GetWorkerPosition = [&](bool bLocalWorker) -> FVector {
				for (const auto& WorkerRegion : WorkerRegions)
				{
					if ((bLocalWorker && WorkerRegion.Key == LocalWorker) || (!bLocalWorker && WorkerRegion.Key != LocalWorker))
					{
						const FVector2D Centre = WorkerRegion.Value.GetCenter();
						return FVector{ Centre.X, Centre.Y, 0.f };
					}
				}
				return {};
			};

			FVector LocalWorkerPosition = GetWorkerPosition(true);
			FVector OtherWorkerPosition = GetWorkerPosition(false);

			// Move both interested actors back to server 1's area
			AssertTrue(InterestedInThisReplicatedActor->HasAuthority(), TEXT("Must have authority over actor"));
			AssertTrue(NotInterestedInThisReplicatedActor->HasAuthority(), TEXT("Must have authority over actor"));
			InterestedInThisReplicatedActor->SetActorLocation(OtherWorkerPosition);
			NotInterestedInThisReplicatedActor->SetActorLocation(OtherWorkerPosition);

			FinishStep();
		});
	}
}
