// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "SpatialAuthorityTest.h"
#include "SpatialAuthorityTestActor.h"
#include "SpatialAuthorityTestReplicatedActor.h"
#include "SpatialFunctionalTestFlowController.h"
#include "Engine/World.h"
#include "Net/UnrealNetwork.h"

/**
 * This test ensures that an RPC function declared in an interface can be called in Spatial to ensure parity with native Unreal.
 * It creates an actor, transfers ownership and then calls a client RPC on that actor. Finally, it verifies that the RPC was received.
 */
ASpatialAuthorityTest::ASpatialAuthorityTest()
{
	Author = "Nuno Afonso";
	Description = TEXT("Test HasAuthority under multi-worker setups");
}

void ASpatialAuthorityTest::BeginPlay()
{
	Super::BeginPlay();

	FVector Server1Position = FVector(-250.0f, -250.0f, 0.0f);
	FVector Server2Position = FVector(-250.0f, 250.0f, 0.0f);

	// Replicated Level Actor.
	{
		AddStep(TEXT("Replicated Level Actor - Server 1 Has Authority"), FWorkerDefinition::AllWorkers,	nullptr, nullptr,
			[this](ASpatialFunctionalTest* Test, float DeltaTime) {
				if (LevelReplicatedActor->AuthorityOnBeginPlay == 1 && LevelReplicatedActor->AuthorityOnTick == 1)
				{
					FinishStep();
				}
			}, 5.0f);
	}

	// Non-replicated Level Actor.
	{
		AddStep(TEXT("Non-replicated Level Actor - Each Server has Authority, Client doesn't know"), FWorkerDefinition::AllWorkers, nullptr, nullptr,
			[this](ASpatialFunctionalTest* Test, float DeltaTime) {
				if (GetLocalFlowController()->WorkerDefinition.Type == ESpatialFunctionalTestWorkerType::Server)
				{
					if (LevelActor->AuthorityOnBeginPlay == LevelActor->AuthorityOnTick
						&& LevelActor->AuthorityOnBeginPlay == GetLocalFlowController()->WorkerDefinition.Id)
					{
						FinishStep();
					}
				}
				else
				{
					if (LevelActor->AuthorityOnBeginPlay == LevelActor->AuthorityOnTick
						&& LevelActor->AuthorityOnBeginPlay == 0)
					{
						FinishStep(); // Clients don't have authority over Level non replicated.
					}
				}
			}, 5.0f);
	}

	// Replicated Dynamic Actor Spawned On Same Server.
	{
		AddStep(TEXT("Replicated Dynamic Actor Spawned On Same Server - Spawn"), FWorkerDefinition::Server(1), nullptr,
			[this, Server1Position](ASpatialFunctionalTest* Test)
			{
				ASpatialAuthorityTestReplicatedActor* Actor = GetWorld()->SpawnActor<ASpatialAuthorityTestReplicatedActor>(
					Server1Position, FRotator::ZeroRotator);
				//Actor->OwnerTest = this;
				CrossServerSetDynamicReplicatedActor(Actor);
				FinishStep();
			});

		AddStep(TEXT("Replicated Dynamic Actor Spawned On Same Server - Verify Server 1 Has Authority"), FWorkerDefinition::AllWorkers, nullptr, nullptr,
			[this](ASpatialFunctionalTest* Test, float DeltaTime)
			{
				if (DynamicReplicatedActor != nullptr
					&& DynamicReplicatedActor->AuthorityOnBeginPlay == 1
					&& DynamicReplicatedActor->AuthorityOnTick == 1)
				{
					FinishStep();
				}
			}, 5.0f);

		AddStep(TEXT("Replicated Dynamic Actor Spawned On Same Server - Destroy"), FWorkerDefinition::Server(1), nullptr,
			[this](ASpatialFunctionalTest* Test)
			{
				DynamicReplicatedActor->Destroy();
				CrossServerSetDynamicReplicatedActor(nullptr);
				FinishStep();
			});
	}

	// Replicated Dynamic Actor Spawned On Different Server.
	{
		AddStep(TEXT("Replicated Dynamic Actor Spawned On Different Server - Spawn"), FWorkerDefinition::Server(1), nullptr,
				[this, Server2Position](ASpatialFunctionalTest* Test) {
					ASpatialAuthorityTestReplicatedActor* Actor = GetWorld()->SpawnActor<ASpatialAuthorityTestReplicatedActor>(
						Server2Position, FRotator::ZeroRotator);
					// Actor->OwnerTest = this;
					CrossServerSetDynamicReplicatedActor(Actor);
					FinishStep();
				});

		AddStep(
			TEXT("Replicated Dynamic Actor Spawned On Different Server - Verify Server 1 Has Authority on BeginPlay and Server 2 on Tick"), FWorkerDefinition::AllWorkers, nullptr,
			nullptr,
			[this](ASpatialFunctionalTest* Test, float DeltaTime) {
				if (DynamicReplicatedActor != nullptr
					&& DynamicReplicatedActor->AuthorityOnBeginPlay == 1
					&& DynamicReplicatedActor->AuthorityOnTick == 2)
				{
					FinishStep();
				}
			},
			5.0f);

		// Now it's Server 2 destroying, since it has Authority over it.
		AddStep(TEXT("Replicated Dynamic Actor Spawned On Different Server - Destroy"), FWorkerDefinition::Server(2), nullptr,
				[this](ASpatialFunctionalTest* Test)
				{
					DynamicReplicatedActor->Destroy();
					CrossServerSetDynamicReplicatedActor(nullptr);
					FinishStep();
				});
	}

	// Non-replicated Dynamic Actor.
	{
		AddStep(TEXT("Non-replicated Dynamic Actor - Spawn"), FWorkerDefinition::Server(1), nullptr,
			[this, Server2Position](ASpatialFunctionalTest* Test)
			{
				// Spawning directly on Server 2.
				DynamicNonReplicatedActor = GetWorld()->SpawnActor<ASpatialAuthorityTestActor>(Server2Position, FRotator::ZeroRotator);
				FinishStep();
			});

		AddStep(TEXT("Non-replicated Dynamic Actor - Verify Authority on Server 1"), FWorkerDefinition::Server(1), nullptr, nullptr,
				[this](ASpatialFunctionalTest* Test, float DeltaTime)
				{
					if (DynamicNonReplicatedActor->AuthorityOnBeginPlay == 1
						&& DynamicNonReplicatedActor->AuthorityOnTick == 1)
					{
						FinishStep();
					}
				});

		float Timer = 0.5f;

		AddStep(TEXT("Non-replicated Dynamic Actor - Verify Dynamic Actor doesn't exist on others"), FWorkerDefinition::AllWorkers, nullptr, nullptr,
				[this, &Timer](ASpatialFunctionalTest* Test, float DeltaTime) {
					FWorkerDefinition LocalWorkerDefinition = GetLocalFlowController()->WorkerDefinition;
					if (LocalWorkerDefinition.Type == ESpatialFunctionalTestWorkerType::Server
						&& LocalWorkerDefinition.Id == 1)
					{
						FinishStep();
					}
					else
					{
						Timer -= DeltaTime;
						if (Timer < 0.0f)
						{
							int NumNonReplicatedActorsExpected = 1; // The one that is in the Map itself
							int NumNonReplicatedActorsInLevel = 0;
							for (TActorIterator<ASpatialAuthorityTestActor> It(GetWorld()); It; ++It)
							{
								if (!It->GetIsReplicated())
								{
									NumNonReplicatedActorsInLevel += 1;
								}
							}

							if (NumNonReplicatedActorsInLevel == NumNonReplicatedActorsExpected)
							{
								FinishStep();
							}
							else
							{
								FinishTest(EFunctionalTestResult::Failed,
									FString::Printf(TEXT("Was expecting only %d non replicated Actors, but found %d"),
										NumNonReplicatedActorsExpected, NumNonReplicatedActorsInLevel));
							}
						}
					}
				});
	}

	// GameMode Actor
	{

	}

	// GameState Actor
	{

	}

}

void ASpatialAuthorityTest::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ASpatialAuthorityTest, DynamicReplicatedActor);
}

void ASpatialAuthorityTest::CrossServerSetDynamicReplicatedActor_Implementation(ASpatialAuthorityTestReplicatedActor* Actor)
{
	DynamicReplicatedActor = Actor;
}
