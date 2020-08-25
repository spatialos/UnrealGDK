// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "SpatialAuthorityTest.h"
#include "Engine/World.h"
#include "Net/UnrealNetwork.h"
#include "SpatialAuthorityTestActor.h"
#include "SpatialAuthorityTestActorComponent.h"
#include "SpatialAuthorityTestGameMode.h"
#include "SpatialAuthorityTestGameState.h"
#include "SpatialAuthorityTestReplicatedActor.h"
#include "SpatialFunctionalTestFlowController.h"
#include "SpatialGDK/Public/EngineClasses/SpatialNetDriver.h"

/** This Test is meant to check that HasAuthority() rules are respected on different occasions. We check
  * in BeginPlay and Tick, and in the following use cases:
  *		- replicated level actor
  *		- non-replicated level actor
  *		- dynamic replicated actor (one time with spatial authority and another without)
  *		- dynamic non-replicated actor
  *		- GameMode (which only exists on Servers)
  *		- GameState
  * Keep in mind that we're assuming a 1x2 Grid Load-Balancing Strategy, otherwise the ownership of
  * these actors may be something completely different (specially important for actors placed in the Level).
  * You have some flexibility to change the Server1/2Position properties to test in different Load-Balancing Strategies.
  */
ASpatialAuthorityTest::ASpatialAuthorityTest()
{
	Author = "Nuno Afonso";
	Description = TEXT("Test HasAuthority under multi-worker setups. It also ensures it works in Native");

	Server1Position = FVector(-250.0f, -250.0f, 0.0f);
	Server2Position = FVector(-250.0f, 250.0f, 0.0f);
}

void ASpatialAuthorityTest::BeginPlay()
{
	Super::BeginPlay();

	ResetTimer();

	if(HasAuthority())
	{
		NumHadAuthorityOverGameMode = 0;
		NumHadAuthorityOverGameState = 0;
	}

	// Replicated Level Actor. Server 1 should have Authority, again assuming that the Level is setup accordingly.
	{
		AddStep(TEXT("Replicated Level Actor - Server 1 Has Authority"), FWorkerDefinition::AllWorkers,	nullptr, nullptr,
			[this](float DeltaTime) {
				Timer -= DeltaTime;
				if (Timer <= 0)
				{
					const FWorkerDefinition& LocalWorkerDefinition = GetLocalFlowController()->WorkerDefinition;
					if (LocalWorkerDefinition.Type == ESpatialFunctionalTestWorkerType::Server && LocalWorkerDefinition.Id == 1)
					{
						if (VerifyTestActor(LevelReplicatedActor, 1, 1, 1, 0))
						{
							FinishStep();
						}
					}
					else
					{
					
						if (VerifyTestActor(LevelReplicatedActor, 0, 0, 0, 0))
						{
							FinishStep();
						}
					}
				}
			}, 5.0f);
	}

	// Non-replicated Level Actor. Each Server should have Authority over their instance, Clients don't.
	{
		AddStep(TEXT("Non-replicated Level Actor - Each Server has Authority, Client doesn't know"), FWorkerDefinition::AllWorkers, nullptr, nullptr,
			[this](float DeltaTime) {
				// Since this actor already was in level and we wait for timer in the previous step, we don't need to wait
				// in this one again.
				const FWorkerDefinition& LocalWorkerDefinition = GetLocalFlowController()->WorkerDefinition;
				if (LocalWorkerDefinition.Type == ESpatialFunctionalTestWorkerType::Server)
				{
					// Note: Non-replicated actors never get OnAuthorityGained() called.
					if (VerifyTestActor(LevelActor, LocalWorkerDefinition.Id, LocalWorkerDefinition.Id, 0, 0))
					{
						FinishStep();
					}
				}
				else
				{
					if (VerifyTestActor(LevelActor, 0, 0, 0, 0))
					{
						FinishStep(); // Clients don't have authority over non-replicated Level Actors.
					}
				}
			}, 5.0f);
	}

	// Replicated Dynamic Actor Spawned On Same Server. Server 1 should have Authority.
	{
		AddStep(TEXT("Replicated Dynamic Actor Spawned On Same Server - Spawn"), FWorkerDefinition::Server(1), nullptr,
			[this]()
			{
				ASpatialAuthorityTestReplicatedActor* Actor = GetWorld()->SpawnActor<ASpatialAuthorityTestReplicatedActor>(
					Server1Position, FRotator::ZeroRotator);
				CrossServerSetDynamicReplicatedActor(Actor);
				FinishStep();
			});

		AddStep(TEXT("Replicated Dynamic Actor Spawned On Same Server - Verify Server 1 Has Authority"), FWorkerDefinition::AllWorkers, nullptr, nullptr,
			[this](float DeltaTime)
			{
				Timer -= DeltaTime;
				if (Timer <= 0)
				{
					const FWorkerDefinition& LocalWorkerDefinition = GetLocalFlowController()->WorkerDefinition;
					if( LocalWorkerDefinition.Type == ESpatialFunctionalTestWorkerType::Server && LocalWorkerDefinition.Id == 1)
					{
						if (VerifyTestActor(DynamicReplicatedActor, 1, 1, 1, 0))
						{
							FinishStep();
						}
					}
					else
					{
						if (VerifyTestActor(DynamicReplicatedActor, 0, 0, 0, 0))
						{
							FinishStep();
						}
					}
				}
			}, 5.0f);

		AddStep(TEXT("Replicated Dynamic Actor Spawned On Same Server - Destroy"), FWorkerDefinition::Server(1), nullptr,
			[this]()
			{
				DynamicReplicatedActor->Destroy();
				CrossServerSetDynamicReplicatedActor(nullptr);
				FinishStep();
			});
	}

	// Replicated Dynamic Actor Spawned On Different Server. Server 1 should have Authority on BeginPlay, Server 2 on Tick
	{
		AddStep(TEXT("Replicated Dynamic Actor Spawned On Different Server - Spawn"), FWorkerDefinition::Server(1), nullptr,
				[this]() {
					ASpatialAuthorityTestReplicatedActor* Actor = GetWorld()->SpawnActor<ASpatialAuthorityTestReplicatedActor>(
						Server2Position, FRotator::ZeroRotator);
					CrossServerSetDynamicReplicatedActor(Actor);
					FinishStep();
				});

		AddStep(
			TEXT("Replicated Dynamic Actor Spawned On Different Server - Verify Server 1 Has Authority on BeginPlay and Server 2 on Tick"), FWorkerDefinition::AllWorkers, nullptr,
			nullptr,
			[this](float DeltaTime) {
				Timer -= DeltaTime;
				if(Timer <= 0)
				{
					const FWorkerDefinition& LocalWorkerDefinition = GetLocalFlowController()->WorkerDefinition;
					if(LocalWorkerDefinition.Type == ESpatialFunctionalTestWorkerType::Server)
					{
						// Allow it to continue working in Native / Single worker setups.
						if (GetNumberOfServerWorkers() > 1)
						{
							if (LocalWorkerDefinition.Id == 1)
							{
								// Note: Depending on timing / data migration, Tick may or may not have ran
								// in Server 1, so we need to check both situations.
								if (VerifyTestActor(DynamicReplicatedActor, 1, 1, 1, 1)
									|| VerifyTestActor(DynamicReplicatedActor, 1, 0, 1, 1))
								{
									FinishStep();
								}
							}
							else if (LocalWorkerDefinition.Id == 2)
							{
								if (VerifyTestActor(DynamicReplicatedActor, 0, 2, 1, 0)
									&& DynamicReplicatedActor->AuthorityComponent->ReplicatedAuthWorkerIdOnBeginPlay == 1)
								{
									FinishStep();
								}
							}
							else
							{
								if (VerifyTestActor(DynamicReplicatedActor, 0, 0, 0, 0))
								{
									FinishStep();
								}
							}
						}
						else // Support for Native / Single Worker.
						{
							if (VerifyTestActor(DynamicReplicatedActor, 1, 1, 1, 0))
							{
								FinishStep();
							}
						}
					}
					else // Clients.
					{
						if (VerifyTestActor(DynamicReplicatedActor, 0, 0, 0, 0))
						{
							FinishStep();
						}
					}
				}
			}, 5.0f);

		// Now it's Server 2 destroying, since it has Authority over it.
		AddStep(TEXT("Replicated Dynamic Actor Spawned On Different Server - Destroy"), FWorkerDefinition::Server(2), nullptr,
				[this]()
				{
					DynamicReplicatedActor->Destroy();
					CrossServerSetDynamicReplicatedActor(nullptr);
					FinishStep();
				});
	}

	// Non-replicated Dynamic Actor. Server 1 should have Authority.
	{
		AddStep(TEXT("Non-replicated Dynamic Actor - Spawn"), FWorkerDefinition::Server(1), nullptr,
			[this]()
			{
				// Spawning directly on Server 2, but since it's non-replicated it shouldn't migrate to Server 2.
				DynamicNonReplicatedActor = GetWorld()->SpawnActor<ASpatialAuthorityTestActor>(Server2Position, FRotator::ZeroRotator);
				FinishStep();
			});

		AddStep(TEXT("Non-replicated Dynamic Actor - Verify Authority on Server 1"), FWorkerDefinition::Server(1), nullptr, nullptr,
				[this](float DeltaTime)
				{
					// Not replicated so OnAuthorityGained() is not called.
					if (VerifyTestActor(DynamicNonReplicatedActor, 1, 1, 0, 0))
					{
						FinishStep();
					}
				}, 5.0f);

		AddStep(TEXT("Non-replicated Dynamic Actor - Verify Dynamic Actor doesn't exist on others"), FWorkerDefinition::AllWorkers, nullptr, nullptr,
				[this](float DeltaTime) {
					const FWorkerDefinition& LocalWorkerDefinition = GetLocalFlowController()->WorkerDefinition;
					if (LocalWorkerDefinition.Type == ESpatialFunctionalTestWorkerType::Server
						&& LocalWorkerDefinition.Id == 1)
					{
						FinishStep();
					}
					else
					{
						Timer -= DeltaTime;
						if (Timer <= 0)
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

		// Destroy to be able to re-run.
		AddStep(TEXT("Non-replicated Dynamic Actor - Destroy"), FWorkerDefinition::Server(1), nullptr,
				[this]() {
					DynamicNonReplicatedActor->Destroy();
					DynamicNonReplicatedActor = nullptr;
					FinishStep();
				});
	}

	// GameMode.
	{
		AddStep(TEXT("GameMode - Determine Authority by every Server"), FWorkerDefinition::AllServers, nullptr, nullptr,
			[this](float DeltaTime){
				// This is running very far from the start, no need to wait more time.
				ASpatialAuthorityTestGameMode* GameMode = GetWorld()->GetAuthGameMode<ASpatialAuthorityTestGameMode>();
				if (GameMode == nullptr)
				{
					FinishTest(EFunctionalTestResult::Failed, TEXT("This test requires ASpatialAuthorityTestGameMode"));
					return;
				}

				bool bIsStateValid;

				USpatialAuthorityTestActorComponent* AuthorityComponent = GameMode->AuthorityComponent;
				// Either it's bigger than 0 and all match (in the Authoritative Server), or all equal to zero (except for replicated).
				if (AuthorityComponent->AuthWorkerIdOnBeginPlay > 0)
				{
					bool bIsUsingSpatial = Cast<USpatialNetDriver>(GetNetDriver()) != nullptr;

					// Currently the behaviour is that if you're using native, OnAuthorityGained is never called.
					int NumExpectedAuthorityGains = bIsUsingSpatial ? 1 : 0;

					bIsStateValid = AuthorityComponent->AuthWorkerIdOnBeginPlay == AuthorityComponent->ReplicatedAuthWorkerIdOnBeginPlay
						   && AuthorityComponent->AuthWorkerIdOnBeginPlay == AuthorityComponent->AuthWorkerIdOnTick
						   && AuthorityComponent->NumAuthorityGains == NumExpectedAuthorityGains && AuthorityComponent->NumAuthorityLosses == 0;
				}
				else
				{
					bIsStateValid = AuthorityComponent->ReplicatedAuthWorkerIdOnBeginPlay != 0 && AuthorityComponent->AuthWorkerIdOnBeginPlay == 0
									&& AuthorityComponent->AuthWorkerIdOnTick == 0 && AuthorityComponent->NumAuthorityGains == 0
									&& AuthorityComponent->NumAuthorityLosses == 0;
				}

				if(bIsStateValid)
				{
					if( AuthorityComponent->AuthWorkerIdOnBeginPlay)
					{
						CrossServerNotifyHadAuthorityOverGameMode();
					}
					FinishStep();
				}
			}, 50.0f);
	}

	// GameState.
	{
		AddStep(
			TEXT("GameState - Determine Authority by all Workers"), FWorkerDefinition::AllWorkers, nullptr, nullptr,
			[this](float DeltaTime) {
				// Again, no need to wait for more time.
				ASpatialAuthorityTestGameState* GameState = GetWorld()->GetGameState<ASpatialAuthorityTestGameState>();
				if (GameState == nullptr) // GameMode already checked on previous step.
				{
					FinishTest(EFunctionalTestResult::Failed, TEXT("This test requires ASpatialAuthorityTestGameState"));
					return;
				}

				bool bIsStateValid;

				USpatialAuthorityTestActorComponent* AuthorityComponent = GameState->AuthorityComponent;
				// Either it's bigger than 0 and all match the GameMode Authority (in the Authoritative Server),
				// or all equal to zero (except for replicated).
				if (AuthorityComponent->AuthWorkerIdOnBeginPlay > 0)
				{
					ASpatialAuthorityTestGameMode* GameMode = GetWorld()->GetAuthGameMode<ASpatialAuthorityTestGameMode>();
					int GameModeAuthority = GameMode->AuthorityComponent->ReplicatedAuthWorkerIdOnBeginPlay;
					bIsStateValid = AuthorityComponent->AuthWorkerIdOnBeginPlay == GameModeAuthority
						   && AuthorityComponent->ReplicatedAuthWorkerIdOnBeginPlay == GameModeAuthority
						   && AuthorityComponent->AuthWorkerIdOnTick == GameModeAuthority
						   && AuthorityComponent->NumAuthorityGains == 1
						   && AuthorityComponent->NumAuthorityLosses == 0;
				}
				else
				{

					bIsStateValid = AuthorityComponent->ReplicatedAuthWorkerIdOnBeginPlay != 0
						   && AuthorityComponent->AuthWorkerIdOnBeginPlay == 0
						   && AuthorityComponent->AuthWorkerIdOnTick == 0
						   && AuthorityComponent->NumAuthorityGains == 0
						   && AuthorityComponent->NumAuthorityLosses == 0;
				}

				if(bIsStateValid)
				{
					if( AuthorityComponent->AuthWorkerIdOnBeginPlay > 0)
					{
						CrossServerNotifyHadAuthorityOverGameState();
					}
					FinishStep();
				}
			},
			5.0f);
	}

	// Verify GameMode/State Unique Authority
	{
		AddStep(TEXT("Verify GameMode/State Unique Authority"), FWorkerDefinition::Server(1), nullptr, nullptr,
		[this](float DeltaTime){
			if(NumHadAuthorityOverGameMode == 1 && NumHadAuthorityOverGameState == 1)
			{
				// Reset in case we run the Test multiple times in a row.
				NumHadAuthorityOverGameMode = 0;
				NumHadAuthorityOverGameState = 0;
				FinishStep();
			}
		}, 5.0f);
	}

}

void ASpatialAuthorityTest::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ASpatialAuthorityTest, DynamicReplicatedActor);
	DOREPLIFETIME(ASpatialAuthorityTest, NumHadAuthorityOverGameMode);
	DOREPLIFETIME(ASpatialAuthorityTest, NumHadAuthorityOverGameState);
}

void ASpatialAuthorityTest::CrossServerSetDynamicReplicatedActor_Implementation(ASpatialAuthorityTestReplicatedActor* Actor)
{
	DynamicReplicatedActor = Actor;
}

void ASpatialAuthorityTest::CrossServerNotifyHadAuthorityOverGameMode_Implementation()
{
	NumHadAuthorityOverGameMode += 1;
}

void ASpatialAuthorityTest::CrossServerNotifyHadAuthorityOverGameState_Implementation()
{
	NumHadAuthorityOverGameState += 1;
}

bool ASpatialAuthorityTest::VerifyTestActor(ASpatialAuthorityTestActor* Actor, int AuthorityOnBeginPlay, int AuthorityOnTick
												, int NumAuthorityGains, int NumAuthorityLosses)
{
	if (!IsValid(Actor) || !Actor->HasActorBegunPlay())
	{
		return false;
	}
	
	return Actor->AuthorityComponent->AuthWorkerIdOnBeginPlay == AuthorityOnBeginPlay
		&& Actor->AuthorityComponent->AuthWorkerIdOnTick == AuthorityOnTick
		&& Actor->AuthorityComponent->NumAuthorityGains == NumAuthorityGains
		&& Actor->AuthorityComponent->NumAuthorityLosses == NumAuthorityLosses;
}
