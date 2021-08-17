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
	DOREPLIFETIME(ThisClass, OtherInterestedInThisReplicatedActor);
}

void AAlwaysInterestedTest::PrepareTest()
{
	Super::PrepareTest();

	const float StepTimeLimit = 5.0f;

	{ // Step 0 - Cache worker positions, and spawn actors on auth server
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

			if (HasAuthority())
			{
				ActorWithAlwaysInterestedProperty =
					GetWorld()->SpawnActor<AAlwaysInterestedTestActor>(LocalWorkerPosition, FRotator::ZeroRotator, FActorSpawnParameters());

				InterestedInThisReplicatedActor =
					GetWorld()->SpawnActor<ASmallNCDActor>(LocalWorkerPosition, FRotator::ZeroRotator, FActorSpawnParameters());

				NotInterestedInThisReplicatedActor =
					GetWorld()->SpawnActor<ASmallNCDActor>(LocalWorkerPosition, FRotator::ZeroRotator, FActorSpawnParameters());

				// This actor is used later as a replacement for InterestedInThisReplicatedActor, so isn't immediate added to
				// AlwaysInterested
				OtherInterestedInThisReplicatedActor =
					GetWorld()->SpawnActor<ASmallNCDActor>(LocalWorkerPosition, FRotator::ZeroRotator, FActorSpawnParameters());

				ActorWithAlwaysInterestedProperty->InterestedActors.Push(InterestedInThisReplicatedActor);

				AController* PlayerController1 =
					Cast<AController>(GetFlowController(ESpatialFunctionalTestWorkerType::Client, 1)->GetOwner());
				if (!AssertTrue(IsValid(PlayerController1), TEXT("Should have spawned a PlayerController 1")))
				{
					return;
				}

				if (!AssertTrue(PlayerController1->HasAuthority(), TEXT("Should have authority over the PlayerController 1")))
				{
					return;
				}

				AController* PlayerController2 =
					Cast<AController>(GetFlowController(ESpatialFunctionalTestWorkerType::Client, 2)->GetOwner());
				if (!AssertTrue(IsValid(PlayerController2), TEXT("Should have spawned a PlayerController 2")))
				{
					return;
				}

				if (!AssertTrue(PlayerController2->HasAuthority(), TEXT("Should have authority over the PlayerController 2")))
				{
					return;
				}

				// Move PCs so they are roughly at worker's position, but with an offset to not trigger the interest of each other,
				// or the SmallNCDActors located at the worker's position.
				OriginalPawn1Position = PlayerController1->GetPawn()->GetActorLocation();
				OriginalPawn2Position = PlayerController2->GetPawn()->GetActorLocation();
				PlayerController1->GetPawn()->SetActorLocation(LocalWorkerPosition + FVector(200.f, 0.f, 0.f));
				PlayerController2->GetPawn()->SetActorLocation(LocalWorkerPosition - FVector(200.f, 0.f, 0.f));

				ActorWithAlwaysInterestedProperty->SetOwner(PlayerController1);

				RegisterAutoDestroyActor(ActorWithAlwaysInterestedProperty);
				RegisterAutoDestroyActor(InterestedInThisReplicatedActor);
				RegisterAutoDestroyActor(NotInterestedInThisReplicatedActor);
				RegisterAutoDestroyActor(OtherInterestedInThisReplicatedActor);
			}

			FinishStep();
		});
	}

	{ // Step 1 - Move actors to server 2
		AddStep(
			TEXT("AlwaysInterested_MoveActors"), FWorkerDefinition::Server(1),
			[this]() -> bool {
				return (ActorWithAlwaysInterestedProperty->IsActorReady() && InterestedInThisReplicatedActor->IsActorReady()
						&& NotInterestedInThisReplicatedActor->IsActorReady() && OtherInterestedInThisReplicatedActor->IsActorReady());
			},
			[this]() {
				// Move first two interested actors to server 2's area
				AssertTrue(InterestedInThisReplicatedActor->HasAuthority(),
						   TEXT("Must have authority over InterestedInThisReplicatedActor"));
				AssertTrue(NotInterestedInThisReplicatedActor->HasAuthority(),
						   TEXT("Must have authority over NotInterestedInThisReplicatedActor"));
				InterestedInThisReplicatedActor->SetActorLocation(OtherWorkerPosition);
				NotInterestedInThisReplicatedActor->SetActorLocation(OtherWorkerPosition);

				FinishStep();
			});
	}

	{ // Step 2 - Validate visibility on server 2
		AddStep(
			TEXT("AlwaysInterested_ValidateVisibilityOnServer2"), FWorkerDefinition::Server(2), nullptr, nullptr,
			[this](float DeltaTime) {
				RequireTrue(!IsValid(ActorWithAlwaysInterestedProperty), TEXT("Shouldn't see root actor"));
				RequireTrue(IsValid(InterestedInThisReplicatedActor), TEXT("Should see interested actor via authority"));
				RequireTrue(IsValid(NotInterestedInThisReplicatedActor), TEXT("Should see not-interested actor via authority"));
				RequireTrue(!IsValid(OtherInterestedInThisReplicatedActor), TEXT("Shouldn't see other interested actor"));
				FinishStep(); // This will only actually finish if requires are satisfied
			},
			StepTimeLimit);
	}

	{ // Step 3 - Validate visibility on server 1
		AddStep(
			TEXT("AlwaysInterested_ValidateVisibilityOnServer1"), FWorkerDefinition::Server(1), nullptr, nullptr,
			[this](float DeltaTime) {
				RequireTrue(IsValid(ActorWithAlwaysInterestedProperty), TEXT("Should see root actor via authority"));
				RequireTrue(IsValid(InterestedInThisReplicatedActor), TEXT("Should see interested actor via always interest"));
				RequireTrue(!IsValid(NotInterestedInThisReplicatedActor), TEXT("Shouldn't see not-interested actor"));
				RequireTrue(IsValid(OtherInterestedInThisReplicatedActor), TEXT("Should see other interested actor via authority"));
				FinishStep(); // This will only actually finish if requires are satisfied
			},
			StepTimeLimit);
	}

	{ // Step 4 - Validate visibility on owning client
		AddStep(
			TEXT("AlwaysInterested_ValidateVisibilityOnOwningClient"), FWorkerDefinition::Client(1), nullptr, nullptr,
			[this](float DeltaTime) {
				RequireTrue(IsValid(ActorWithAlwaysInterestedProperty), TEXT("Should see root actor via ownership"));
				RequireTrue(IsValid(InterestedInThisReplicatedActor), TEXT("Should see interested actor via always interest"));
				RequireTrue(!IsValid(NotInterestedInThisReplicatedActor), TEXT("Shouldn't see not-interested actor"));
				RequireTrue(!IsValid(OtherInterestedInThisReplicatedActor), TEXT("Shouldn't see other interested actor"));
				FinishStep(); // This will only actually finish if requires are satisfied
			},
			StepTimeLimit);
	}

	{ // Step 5 - Validate visibility on non-owning client
		AddStep(
			TEXT("AlwaysInterested_ValidateVisibilityOnNonOwningClient"), FWorkerDefinition::Client(2), nullptr, nullptr,
			[this](float DeltaTime) {
				RequireTrue(!IsValid(ActorWithAlwaysInterestedProperty), TEXT("Shouldn't see root actor"));
				RequireTrue(!IsValid(InterestedInThisReplicatedActor), TEXT("Shouldn't see interested actor"));
				RequireTrue(!IsValid(NotInterestedInThisReplicatedActor), TEXT("Shouldn't see not-interested actor"));
				RequireTrue(!IsValid(OtherInterestedInThisReplicatedActor), TEXT("Shouldn't see other interested actor"));
				FinishStep(); // This will only actually finish if requires are satisfied
			},
			StepTimeLimit);
	}

	{ // Step 6 - Replace Interest Actors
		AddStep(TEXT("AlwaysInterested_ReplaceInterestActors"), FWorkerDefinition::Server(1), nullptr, [this]() {
			// Replace InterestedInThisReplicatedActor with OtherInterestedInThisReplicatedActor
			AssertTrue(ActorWithAlwaysInterestedProperty->HasAuthority(),
					   TEXT("Must have authority over ActorWithAlwaysInterestedProperty"));
			ActorWithAlwaysInterestedProperty->InterestedActors[0] = OtherInterestedInThisReplicatedActor;

			FinishStep();
		});
	}

	{ // Step 7 - Validate visibility on owning client
		AddStep(
			TEXT("AlwaysInterested_ValidateVisibilityOnOwningClient_Part2"), FWorkerDefinition::Client(1), nullptr, nullptr,
			[this](float DeltaTime) {
				RequireTrue(IsValid(ActorWithAlwaysInterestedProperty), TEXT("Should see root actor via ownership"));
				RequireTrue(!IsValid(InterestedInThisReplicatedActor), TEXT("Shouldn't see interested actor"));
				RequireTrue(!IsValid(NotInterestedInThisReplicatedActor), TEXT("Shouldn't see not-interested actor"));
				RequireTrue(IsValid(OtherInterestedInThisReplicatedActor), TEXT("Should see other interested actor via interest"));
				FinishStep(); // This will only actually finish if requires are satisfied
			},
			StepTimeLimit);
	}

	{ // Step 8 - Move other interest actors
		AddStep(TEXT("AlwaysInterested_MoveOtherInterestActor"), FWorkerDefinition::Server(1), nullptr, [this]() {
			// Replace InterestedInThisReplicatedActor with OtherInterestedInThisReplicatedActor
			AssertTrue(OtherInterestedInThisReplicatedActor->HasAuthority(),
					   TEXT("Must have authority over OtherInterestedInThisReplicatedActor"));
			OtherInterestedInThisReplicatedActor->SetActorLocation(OtherWorkerPosition);

			FinishStep();
		});
	}

	{ // Step 9 - Validate new visibility on server 2
		AddStep(
			TEXT("AlwaysInterested_ValidateVisibilityOnServer2_Part2"), FWorkerDefinition::Server(2), nullptr, nullptr,
			[this](float DeltaTime) {
				RequireTrue(!IsValid(ActorWithAlwaysInterestedProperty), TEXT("Shouldn't see root actor"));
				RequireTrue(IsValid(InterestedInThisReplicatedActor), TEXT("Should see interested actor via authority"));
				RequireTrue(IsValid(NotInterestedInThisReplicatedActor), TEXT("Should see not-interested actor via authority"));
				RequireTrue(IsValid(OtherInterestedInThisReplicatedActor), TEXT("Should see other interested actor via authority"));
				FinishStep(); // This will only actually finish if requires are satisfied
			},
			StepTimeLimit);
	}

	{ // Step 10 - Validate visibility on server 1
		AddStep(
			TEXT("AlwaysInterested_ValidateVisibilityOnServer1_Part2"), FWorkerDefinition::Server(1), nullptr, nullptr,
			[this](float DeltaTime) {
				RequireTrue(IsValid(ActorWithAlwaysInterestedProperty), TEXT("Should see root actor via authority"));
				RequireTrue(!IsValid(InterestedInThisReplicatedActor), TEXT("Shouldn't see interested actor"));
				RequireTrue(!IsValid(NotInterestedInThisReplicatedActor), TEXT("Shouldn't see not-interested actor"));
				RequireTrue(IsValid(OtherInterestedInThisReplicatedActor), TEXT("Should see other interested actor via interest"));
				FinishStep(); // This will only actually finish if requires are satisfied
			},
			StepTimeLimit);
	}

	{ // Step 11 - Move actors back to original server
		AddStep(TEXT("AlwaysInterestedMoveActorsBack"), FWorkerDefinition::Server(2), nullptr, [this]() {
			// Move both interested actors back to server 1's area
			AssertTrue(InterestedInThisReplicatedActor->HasAuthority(), TEXT("Must have authority over InterestedInThisReplicatedActor"));
			AssertTrue(NotInterestedInThisReplicatedActor->HasAuthority(),
					   TEXT("Must have authority over NotInterestedInThisReplicatedActor"));
			AssertTrue(OtherInterestedInThisReplicatedActor->HasAuthority(),
					   TEXT("Must have authority over OtherInterestedInThisReplicatedActor"));
			InterestedInThisReplicatedActor->SetActorLocation(OtherWorkerPosition);
			NotInterestedInThisReplicatedActor->SetActorLocation(OtherWorkerPosition);
			OtherInterestedInThisReplicatedActor->SetActorLocation(OtherWorkerPosition);

			FinishStep();
		});
	}

	{ // Step 12 - Validate visibility on server 1 again
		AddStep(
			TEXT("AlwaysInterested_ValidateVisibilityOnServer1_Part3"), FWorkerDefinition::Server(1), nullptr, nullptr,
			[this](float DeltaTime) {
				RequireTrue(IsValid(ActorWithAlwaysInterestedProperty), TEXT("Should see root actor via authority"));
				// There's currently an issue where if an authoritative server loses interest over an actor reference, it won't
				// correct the reference when the actor comes back into view - UNR-3917
				// RequireTrue(IsValid(InterestedInThisReplicatedActor), TEXT("Should see interested actor via authority"));
				// RequireTrue(IsValid(NotInterestedInThisReplicatedActor), TEXT("Should see not-interested actor via authority"));
				RequireEqual_Int(GetNumberOfActorsOfType<ASmallNCDActor>(GetWorld()), 3, TEXT("Should see all actors via authority"));
				RequireTrue(IsValid(OtherInterestedInThisReplicatedActor), TEXT("Should see other interested actor via authority"));
				FinishStep(); // This will only actually finish if requires are satisfied
			},
			StepTimeLimit);
	}

	{ // Step 13 - Validate visibility on server 2 again
		AddStep(
			TEXT("AlwaysInterested_ValidateVisibilityOnServer2_Part3"), FWorkerDefinition::Server(2), nullptr, nullptr,
			[this](float DeltaTime) {
				RequireTrue(!IsValid(ActorWithAlwaysInterestedProperty), TEXT("Shouldn't see root actor"));
				RequireTrue(!IsValid(InterestedInThisReplicatedActor), TEXT("Shouldn't see interested actor"));
				RequireTrue(!IsValid(NotInterestedInThisReplicatedActor), TEXT("Shouldn't see not-interested actor"));
				RequireTrue(!IsValid(OtherInterestedInThisReplicatedActor), TEXT("Shouldn't see other interested actor"));
				RequireEqual_Int(GetNumberOfActorsOfType<ASmallNCDActor>(GetWorld()), 0, TEXT("Shouldn't see any Interest actors"));
				FinishStep(); // This will only actually finish if requires are satisfied
			},
			StepTimeLimit);
	}

	{ // Step 14 - Remove Interest Actors
		AddStep(TEXT("AlwaysInterested_ReplaceInterestActors"), FWorkerDefinition::Server(1), nullptr, [this]() {
			// Remove AlwaysInterested properties
			AssertTrue(ActorWithAlwaysInterestedProperty->HasAuthority(),
					   TEXT("Must have authority over ActorWithAlwaysInterestedProperty"));
			ActorWithAlwaysInterestedProperty->InterestedActors.Empty();

			FinishStep();
		});
	}

	{ // Step 15 - Validate visibility on owning client
		AddStep(
			TEXT("AlwaysInterested_ValidateVisibilityOnOwningClient_Part3"), FWorkerDefinition::Client(1), nullptr, nullptr,
			[this](float DeltaTime) {
				RequireTrue(IsValid(ActorWithAlwaysInterestedProperty), TEXT("Should see root actor via ownership"));
				RequireTrue(!IsValid(InterestedInThisReplicatedActor), TEXT("Shouldn't see interested actor"));
				RequireTrue(!IsValid(NotInterestedInThisReplicatedActor), TEXT("Shouldn't see not-interested actor"));
				RequireTrue(!IsValid(OtherInterestedInThisReplicatedActor), TEXT("Shouldn't see other interested actor"));
				FinishStep(); // This will only actually finish if requires are satisfied
			},
			StepTimeLimit);
	}

	{ // Step 16 - Test cleanup
		AddStep(TEXT("AlwaysInterested_TestCleanup"), FWorkerDefinition::Server(1), nullptr, [this]() {
			// Move Pawns back to starting positions otherwise other tests in this map may run incorrectly.
			AController* PlayerController1 = Cast<AController>(GetFlowController(ESpatialFunctionalTestWorkerType::Client, 1)->GetOwner());
			if (!AssertTrue(IsValid(PlayerController1), TEXT("Should have spawned a PlayerController 1")))
			{
				return;
			}

			if (!AssertTrue(PlayerController1->HasAuthority(), TEXT("Should have authority over the PlayerController 1")))
			{
				return;
			}

			AController* PlayerController2 = Cast<AController>(GetFlowController(ESpatialFunctionalTestWorkerType::Client, 2)->GetOwner());
			if (!AssertTrue(IsValid(PlayerController2), TEXT("Should have spawned a PlayerController 2")))
			{
				return;
			}

			if (!AssertTrue(PlayerController2->HasAuthority(), TEXT("Should have authority over the PlayerController 2")))
			{
				return;
			}

			PlayerController1->GetPawn()->SetActorLocation(OriginalPawn1Position);
			PlayerController2->GetPawn()->SetActorLocation(OriginalPawn2Position);

			FinishStep();
		});
	}
}
