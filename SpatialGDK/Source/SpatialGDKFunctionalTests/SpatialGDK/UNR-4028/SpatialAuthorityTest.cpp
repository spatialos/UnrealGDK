// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "SpatialAuthorityTest.h"
#include "Engine/World.h"
#include "Net/UnrealNetwork.h"
#include "SpatialAuthorityTestActor.h"
#include "SpatialAuthorityTestGameMode.h"
#include "SpatialAuthorityTestGameState.h"
#include "SpatialAuthorityTestReplicatedActor.h"
#include "SpatialFunctionalTestFlowController.h"
#include "SpatialAuthorityTestFlowController.h"

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

	FlowControllerActorClass = ASpatialAuthorityTestFlowController::StaticClass();

	Server1Position = FVector(-250.0f, -250.0f, 0.0f);
	Server2Position = FVector(-250.0f, 250.0f, 0.0f);
}

void ASpatialAuthorityTest::BeginPlay()
{
	Super::BeginPlay();

	// Replicated Level Actor. Server 1 should have Authority, again assuming that the Level is setup accordingly.
	{
		AddStep(TEXT("Replicated Level Actor - Server 1 Has Authority"), FWorkerDefinition::AllWorkers,	nullptr, nullptr,
			[this](float DeltaTime) {
				if (LevelReplicatedActor->AuthorityOnBeginPlay == 1 && LevelReplicatedActor->AuthorityOnTick == 1)
				{
					FinishStep();
				}
			}, 5.0f);
	}

	// Non-replicated Level Actor. Each Server should have Authority over their instance, Clients don't.
	{
		AddStep(TEXT("Non-replicated Level Actor - Each Server has Authority, Client doesn't know"), FWorkerDefinition::AllWorkers, nullptr, nullptr,
			[this](float DeltaTime) {
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
				if (DynamicReplicatedActor != nullptr
					&& DynamicReplicatedActor->AuthorityOnBeginPlay == 1
					&& DynamicReplicatedActor->AuthorityOnTick == 1)
				{
					FinishStep();
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
				// Allow it to continue working in Native / Single worker setups.
				int ExpectedAuthorityServer = GetNumberOfServerWorkers() > 1 ? 2 : 1;				
				if (DynamicReplicatedActor != nullptr
					&& DynamicReplicatedActor->AuthorityOnBeginPlay == 1
					&& DynamicReplicatedActor->AuthorityOnTick == ExpectedAuthorityServer)
				{
					FinishStep();
				}
			},
			5.0f);

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
					if (DynamicNonReplicatedActor->AuthorityOnBeginPlay == 1
						&& DynamicNonReplicatedActor->AuthorityOnTick == 1)
					{
						FinishStep();
					}
				});

		AddStep(TEXT("Non-replicated Dynamic Actor - Verify Dynamic Actor doesn't exist on others"), FWorkerDefinition::AllWorkers, nullptr,
				[this](){
					Timer = 0.5f;
				},
				[this](float DeltaTime) {
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
				ASpatialAuthorityTestGameMode* GameMode = GetWorld()->GetAuthGameMode<ASpatialAuthorityTestGameMode>();
				if(GameMode == nullptr)
				{
					FinishTest(EFunctionalTestResult::Failed, TEXT("This test requires ASpatialAuthorityTestGameMode"));
				}
				if(GameMode->AuthorityOnBeginPlay > 0 && GameMode->AuthorityOnBeginPlay == GameMode->AuthorityOnTick)
				{
					CrossServerSetGameModeAuthorityFromServerWorker(GetLocalFlowController()->WorkerDefinition.Id, GameMode->AuthorityOnBeginPlay);
					FinishStep();
				}
			}, 5.0f);

		AddStep(TEXT("GameMode - Verify consensus on Authority"), FWorkerDefinition::Server(1), nullptr, nullptr,
			[this](float DeltaTime)
			{
				int FirstAuthorityValue = GameModeServerAuthorities[0];
				if (FirstAuthorityValue > 0)
				{
					for(int i = 1; i < GameModeServerAuthorities.Num(); ++i)
					{
						if(FirstAuthorityValue != GameModeServerAuthorities[i])
						{
							return;
						}
					}

					FinishStep();
				}
			}, 5.0f);
	}

	// GameState.
	{
		AddStep(
			TEXT("GameState - Determine Authority by every Worker"), FWorkerDefinition::AllWorkers, nullptr, nullptr,
			[this](float DeltaTime) {
				ASpatialAuthorityTestGameState* GameState = GetWorld()->GetGameState<ASpatialAuthorityTestGameState>();
				if (GameState == nullptr)
				{
					FinishTest(EFunctionalTestResult::Failed, TEXT("This test requires ASpatialAuthorityTestGameState"));
				}
				if (GameState->AuthorityOnBeginPlay > 0 && GameState->AuthorityOnBeginPlay == GameState->AuthorityOnTick)
				{
					FWorkerDefinition LocalWorkerDefinition = GetLocalFlowController()->WorkerDefinition;
					if(LocalWorkerDefinition.Type == ESpatialFunctionalTestWorkerType::Server)
					{
						CrossServerSetGameStateAuthorityFromWorker(GetLocalFlowController()->WorkerDefinition,
																	GameState->AuthorityOnBeginPlay);
					}
					else
					{
						ASpatialAuthorityTestFlowController* LocalFlowControllerCast = Cast<ASpatialAuthorityTestFlowController>(GetLocalFlowController());
						if( LocalFlowControllerCast == nullptr)
						{
							FinishTest(EFunctionalTestResult::Failed, TEXT("Test requires ASpatialAuthorityTestFlowController"));
							return;
						}
						LocalFlowControllerCast->ServerSetGameStateAuthority(GameState->AuthorityOnBeginPlay);
					}
					FinishStep();
				}
			},
			5.0f);

		AddStep(
			TEXT("GameState - Verify consensus on Authority"), FWorkerDefinition::Server(1), nullptr, nullptr,
			[this](float DeltaTime) {
				int FirstAuthorityValue = GameModeServerAuthorities[0]; // Should match the Authority of GameMode
				if (FirstAuthorityValue > 0)
				{
					for (int i = 0; i < GameStateServerAuthorities.Num(); ++i)
					{
						if (FirstAuthorityValue != GameStateServerAuthorities[i])
						{
							return;
						}
					}

					for(int i = 0; i < GameStateClientAuthorities.Num(); ++i)
					{
						if(FirstAuthorityValue != GameStateClientAuthorities[i])
						{
							return;
						}
					}

					FinishStep();
				}
			},
			5.0f);
	}

}

void ASpatialAuthorityTest::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ASpatialAuthorityTest, DynamicReplicatedActor);
	DOREPLIFETIME(ASpatialAuthorityTest, GameModeServerAuthorities);
	DOREPLIFETIME(ASpatialAuthorityTest, GameStateServerAuthorities);
	DOREPLIFETIME(ASpatialAuthorityTest, GameStateClientAuthorities);
}

void ASpatialAuthorityTest::StartTest()
{
	GameModeServerAuthorities.SetNum(GetNumberOfServerWorkers());
	GameStateServerAuthorities.SetNum(GetNumberOfServerWorkers());
	GameStateClientAuthorities.SetNum(GetNumberOfClientWorkers());

	// Make sure they're zero'ed for reruns to be more accurate
	for(int i = 0; i < GameModeServerAuthorities.Num(); ++i)
	{
		GameModeServerAuthorities[i] = 0;
		GameStateServerAuthorities[i] = 0;
	}

	for(int i = 0; i < GameStateClientAuthorities.Num(); ++i)
	{
		GameStateClientAuthorities[i] = 0;
	}

	Super::StartTest();
}

void ASpatialAuthorityTest::CrossServerSetDynamicReplicatedActor_Implementation(ASpatialAuthorityTestReplicatedActor* Actor)
{
	DynamicReplicatedActor = Actor;
}

void ASpatialAuthorityTest::CrossServerSetGameModeAuthorityFromServerWorker_Implementation(int ServerWorkerId, int Authority)
{
	GameModeServerAuthorities[ServerWorkerId-1] = Authority;
}

void ASpatialAuthorityTest::CrossServerSetGameStateAuthorityFromWorker_Implementation(const FWorkerDefinition& WorkerDefinition,
																					  int Authority)
{
	if(WorkerDefinition.Type == ESpatialFunctionalTestWorkerType::Server)
	{
		GameStateServerAuthorities[WorkerDefinition.Id-1] = Authority;
	}
	else
	{
		GameStateClientAuthorities[WorkerDefinition.Id - 1] = Authority;
	}
}
