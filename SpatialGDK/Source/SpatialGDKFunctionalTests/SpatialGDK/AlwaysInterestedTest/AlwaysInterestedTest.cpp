// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "AlwaysInterestedTest.h"
#include "SpatialFunctionalTestFlowController.h"
#include "SpatialGDKFunctionalTests/SpatialGDK/TestActors/AlwaysInterestedTestActors.h"

#include "LoadBalancing/LayeredLBStrategy.h"

#include "Net/UnrealNetwork.h"

/**
 * This test tests that AlwaysInterested UProperties are replicated to clients and servers correctly.
 *
 * The test should include two servers and two clients
 * The flow is as follows:
 *  - Setup:
 *    - Server 1 spawns an actor with AlwaysInterested properties, and two test actors, one of which is set to be AlwaysInterested
 *  - Test:
 *    - Move the test actors to server 2 authority area, to ensure interest on server 1 is lost via authority.
 *    - Validate actors are correctly in or out of view for auth and non-auth servers, and owning and non-owning clients.
 *    - Move the test actors to server 1 authority area, to ensure interest on server 2 is lost via authority.
 *    - Validate actors are correctly in the view of auth and non-auth servers.
 *  - Cleanup:
 *    - Destroy the actors
 */

AAlwaysInterestedTest::AAlwaysInterestedTest()
	: Super()
{
	Author = "Mike";
	Description = TEXT("Test Always Interested Functionality");
}

void AAlwaysInterestedTest::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ThisClass, ActorWithAlwaysInterestedProperty);
	DOREPLIFETIME(ThisClass, InterestedInThisReplicatedActor);
	DOREPLIFETIME(ThisClass, NotInterestedInThisReplicatedActor);
}

void AAlwaysInterestedTest::PrepareTest()
{
	Super::PrepareTest();

	const float StepTimeLimit = 5.0f;

	{ // Step 0 - Cache worker positions
		AddStep(TEXT("AlwaysInterested_SpawnActors"), FWorkerDefinition::AllServers, nullptr, [this]() {
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

			LocalWorkerPosition = GetWorkerPosition(true);
			OtherWorkerPosition = GetWorkerPosition(false);

			FinishStep();
		});
	}

	{ // Step 1 - Spawn actors on server 1
		AddStep(TEXT("AlwaysInterested_SpawnActors"), FWorkerDefinition::Server(1), nullptr, [this]() {
			AssertTrue(HasAuthority(), TEXT("Server 1 requires authority over the test actor"));

			ActorWithAlwaysInterestedProperty =
				GetWorld()->SpawnActor<AAlwaysInterestedTestActor>(LocalWorkerPosition, FRotator::ZeroRotator, FActorSpawnParameters());

			InterestedInThisReplicatedActor =
				GetWorld()->SpawnActor<ASmallNCDActor>(LocalWorkerPosition, FRotator::ZeroRotator, FActorSpawnParameters());

			NotInterestedInThisReplicatedActor =
				GetWorld()->SpawnActor<ASmallNCDActor>(LocalWorkerPosition, FRotator::ZeroRotator, FActorSpawnParameters());

			ActorWithAlwaysInterestedProperty->InterestedActors.Push(InterestedInThisReplicatedActor);

			AController* PlayerController = Cast<AController>(GetFlowController(ESpatialFunctionalTestWorkerType::Client, 1)->GetOwner());
			if (!AssertTrue(IsValid(PlayerController), TEXT("Failed to retrieve player controller")))
			{
				return;
			}

			if (!AssertTrue(PlayerController->HasAuthority(), TEXT("Failed to have authority over player controller")))
			{
				return;
			}

			PlayerController->GetPawn()->SetActorLocation(LocalWorkerPosition);

			ActorWithAlwaysInterestedProperty->SetOwner(PlayerController);

			RegisterAutoDestroyActor(ActorWithAlwaysInterestedProperty);
			RegisterAutoDestroyActor(InterestedInThisReplicatedActor);
			RegisterAutoDestroyActor(NotInterestedInThisReplicatedActor);

			FinishStep();
		});
	}

	{ // Step 2 - Move actors to server 2
		AddStep(
			TEXT("AlwaysInterested_MoveActors"), FWorkerDefinition::Server(1),
			[this]() -> bool {
				return (ActorWithAlwaysInterestedProperty->IsActorReady() && InterestedInThisReplicatedActor->IsActorReady()
						&& NotInterestedInThisReplicatedActor->IsActorReady());
			},
			[this]() {
				// Move both interested actors to server 2's area
				AssertTrue(InterestedInThisReplicatedActor->HasAuthority(), TEXT("Must have authority over actor"));
				AssertTrue(NotInterestedInThisReplicatedActor->HasAuthority(), TEXT("Must have authority over actor"));
				InterestedInThisReplicatedActor->SetActorLocation(OtherWorkerPosition);
				NotInterestedInThisReplicatedActor->SetActorLocation(OtherWorkerPosition);

				FinishStep();
			});
	}

	{ // Step 3 - Validate visibility on server 2
		AddStep(
			TEXT("AlwaysInterested_ValidateVisibilityOnServer2"), FWorkerDefinition::Server(2), nullptr, nullptr,
			[this](float DeltaTime) {
				RequireTrue(!IsValid(ActorWithAlwaysInterestedProperty), TEXT("Shouldn't see root actor"));
				RequireTrue(IsValid(InterestedInThisReplicatedActor), TEXT("Should see interested actor via authority"));
				RequireTrue(IsValid(NotInterestedInThisReplicatedActor), TEXT("Should see not-interested actor via authority"));
				FinishStep(); // This will only actually finish if requires are satisfied
			},
			StepTimeLimit);
	}

	{ // Step 4 - Validate visibility on server 1
		AddStep(
			TEXT("AlwaysInterested_ValidateVisibilityOnServer1"), FWorkerDefinition::Server(1), nullptr, nullptr,
			[this](float DeltaTime) {
				RequireTrue(IsValid(ActorWithAlwaysInterestedProperty), TEXT("Should see root actor via authority"));
				RequireTrue(IsValid(InterestedInThisReplicatedActor), TEXT("Should see interested actor via always interest"));
				RequireTrue(!IsValid(NotInterestedInThisReplicatedActor), TEXT("Shouldn't see not-interested actor"));
				FinishStep(); // This will only actually finish if requires are satisfied
			},
			StepTimeLimit);
	}

	{ // Step 5 - Validate visibility on owning client
		AddStep(
			TEXT("AlwaysInterested_ValidateVisibilityOnOwningClient"), FWorkerDefinition::Client(1), nullptr, nullptr,
			[this](float DeltaTime) {
				RequireTrue(IsValid(ActorWithAlwaysInterestedProperty), TEXT("Should see root actor via ownership"));
				RequireTrue(IsValid(InterestedInThisReplicatedActor), TEXT("Should see interested actor via always interest"));
				RequireTrue(!IsValid(NotInterestedInThisReplicatedActor), TEXT("Shouldn't see not-interested actor"));
				FinishStep(); // This will only actually finish if requires are satisfied
			},
			StepTimeLimit);
	}

	{ // Step 6 - Validate visibility on non-owning client
		AddStep(
			TEXT("AlwaysInterested_ValidateVisibilityOnNonOwningClient"), FWorkerDefinition::Client(2), nullptr, nullptr,
			[this](float DeltaTime) {
				RequireTrue(!IsValid(ActorWithAlwaysInterestedProperty), TEXT("Shouldn't see root actor"));
				RequireTrue(!IsValid(InterestedInThisReplicatedActor), TEXT("Shouldn't see interested actor"));
				RequireTrue(!IsValid(NotInterestedInThisReplicatedActor), TEXT("Shouldn't see not-interested actor"));
				FinishStep(); // This will only actually finish if requires are satisfied
			},
			StepTimeLimit);
	}

	{ // Step 7 - Move actors back to original server
		AddStep(TEXT("AlwaysInterestedMoveActorsBack"), FWorkerDefinition::Server(2), nullptr, [this]() {
			// Move both interested actors back to server 1's area
			AssertTrue(InterestedInThisReplicatedActor->HasAuthority(), TEXT("Must have authority over actor"));
			AssertTrue(NotInterestedInThisReplicatedActor->HasAuthority(), TEXT("Must have authority over actor"));
			InterestedInThisReplicatedActor->SetActorLocation(OtherWorkerPosition);
			NotInterestedInThisReplicatedActor->SetActorLocation(OtherWorkerPosition);

			FinishStep();
		});
	}

	{ // Step 8 - Validate visibility on server 1 again
		AddStep(
			TEXT("AlwaysInterested_ValidateVisibilityOnServer1Again"), FWorkerDefinition::Server(1), nullptr, nullptr,
			[this](float DeltaTime) {
				RequireTrue(IsValid(ActorWithAlwaysInterestedProperty), TEXT("Should see root actor via authority"));
				RequireTrue(IsValid(InterestedInThisReplicatedActor), TEXT("Should see interested actor via authority"));
				// There's currently an issue where if an authoritative server loses interest over an actor reference, it won't
				// correct the reference when the actor comes back into view.
				// RequireTrue(IsValid(NotInterestedInThisReplicatedActor), TEXT("Should see not-interested actor via authority"));
				RequireEqual_Int(GetNumberOfActorsOfType<ASmallNCDActor>(GetWorld()), 2,
								 TEXT("Should see not-interested actor via authority"));
				FinishStep(); // This will only actually finish if requires are satisfied
			},
			StepTimeLimit);
	}

	{ // Step 9 - Validate visibility on server 2 again
		AddStep(
			TEXT("AlwaysInterested_ValidateVisibilityOnServer2Again"), FWorkerDefinition::Server(2), nullptr, nullptr,
			[this](float DeltaTime) {
				RequireTrue(!IsValid(ActorWithAlwaysInterestedProperty), TEXT("Shouldn't root actor"));
				RequireTrue(!IsValid(InterestedInThisReplicatedActor), TEXT("Shouldn't see interested actor"));
				RequireTrue(!IsValid(NotInterestedInThisReplicatedActor), TEXT("Shouldn't see not-interested actor"));
				FinishStep(); // This will only actually finish if requires are satisfied
			},
			StepTimeLimit);
	}
}
