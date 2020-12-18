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
#include "SpatialGDK/Public/Utils/SpatialStatics.h"

/** This Test is meant to check that HasAuthority() rules are respected on different occasions. We check
 * in BeginPlay and Tick, and in the following use cases:
 *		- replicated level actor
 *		- replicated level actor on border
 *		- non-replicated level actor
 *		- dynamic replicated actor (one time with spatial authority and another without)
 *		- dynamic non-replicated actor
 *		- dynamic replicated actor spawned on border from server 1
 *		- dynamic replicated actor spawned on border from server 2
 *		- dynamic replicated actor spawned on border from server 3
 *		- dynamic replicated actor spawned on border from server 4
 *		- dynamic non-replicated actor spawned on border
 *		- non-replicated actor spawned on client
 *		- GameMode (which only exists on Servers)
 *		- GameState
 * Keep in mind that we're assuming a 2x2 Grid Load-Balancing Strategy, otherwise the ownership of
 * these actors may be something completely different (specially important for actors placed in the Level).
 * You have some flexibility to change the Server Position properties to test in different Load-Balancing Strategies.
 */
ASpatialAuthorityTest::ASpatialAuthorityTest()
{
	Author = "Nuno Afonso";
	Description = TEXT("Test HasAuthority under multi-worker setups. It also ensures it works in Native");

	Server1Position = FVector(-250.0f, -250.0f, 0.0f);
	Server2Position = FVector(250.0f, -250.0f, 0.0f);
	Server3Position = FVector(-250.0f, 250.0f, 0.0f);
	Server4Position = FVector(250.0f, 250.0f, 0.0f);
	BorderPosition = FVector(0.0f, 0.0f, 0.0f);
}

void ASpatialAuthorityTest::PrepareTest()
{
	Super::PrepareTest();

	FSpatialFunctionalTestStepDefinition NonReplicatedVerifyAuthorityStepDefinition(/*bIsNativeDefinition*/ true);
	NonReplicatedVerifyAuthorityStepDefinition.StepName = TEXT("Non-replicated Dynamic Actor - Verify Authority on Server 1");
	NonReplicatedVerifyAuthorityStepDefinition.TimeLimit = 5.0f;
	NonReplicatedVerifyAuthorityStepDefinition.NativeStartEvent.BindLambda([this]() {
		// Not replicated so OnAuthorityGained() is not called.
		if (VerifyTestActor(DynamicNonReplicatedActor, ESpatialHasAuthority::ServerAuth, 1, 1, 0, 0, 0, 0))
		{
			FinishStep();
		}
	});

	FSpatialFunctionalTestStepDefinition NonReplicatedVerifyNonAuthorityStepDefinition(/*bIsNativeDefinition*/ true);
	NonReplicatedVerifyNonAuthorityStepDefinition.StepName =
		TEXT("Non-replicated Dynamic Actor - Verify Dynamic Actor doesn't exist on non authoritative workers");
	NonReplicatedVerifyNonAuthorityStepDefinition.TimeLimit = 5.0f;
	NonReplicatedVerifyNonAuthorityStepDefinition.NativeTickEvent.BindLambda([this](float DeltaTime) {
		const FWorkerDefinition& LocalWorkerDefinition = GetLocalFlowController()->WorkerDefinition;
		if (LocalWorkerDefinition.Type == ESpatialFunctionalTestWorkerType::Server && LocalWorkerDefinition.Id == 1)
		{
			FinishStep();
		}
		else
		{
			Timer -= DeltaTime;
			if (Timer <= 0)
			{
				CheckNumActorsInLevel();
			}
		}
	});

	FSpatialFunctionalTestStepDefinition NonReplicatedDestroyStepDefinition(/*bIsNativeDefinition*/ true);
	NonReplicatedDestroyStepDefinition.StepName = TEXT("Non-replicated Dynamic Actor - Destroy");
	NonReplicatedDestroyStepDefinition.TimeLimit = 5.0f;
	NonReplicatedDestroyStepDefinition.NativeStartEvent.BindLambda([this]() {
		// Destroy to be able to re-run.
		DynamicNonReplicatedActor->Destroy();
		DynamicNonReplicatedActor = nullptr;
		FinishStep();
	});

	FSpatialFunctionalTestStepDefinition ReplicatedDestroyStepDefinition(/*bIsNativeDefinition*/ true);
	ReplicatedDestroyStepDefinition.StepName = TEXT("Replicated Dynamic Actor Spawned On Different Server - Destroy");
	ReplicatedDestroyStepDefinition.TimeLimit = 5.0f;
	ReplicatedDestroyStepDefinition.NativeStartEvent.BindLambda([this]() {
		// Destroy to be able to re-run.
		DynamicReplicatedActor->Destroy();
		CrossServerSetDynamicReplicatedActor(nullptr);
		FinishStep();
	});

	ResetTimer();

	if (HasAuthority())
	{
		NumHadAuthorityOverGameMode = 0;
		NumHadAuthorityOverGameState = 0;
	}

	// Replicated Level Actor. Server 1 should have Authority, again assuming that the Level is setup accordingly.
	{
		AddStep(
			TEXT("Replicated Level Actor - Server 1 Has Authority"), FWorkerDefinition::AllWorkers, nullptr, nullptr,
			[this](float DeltaTime) {
				Timer -= DeltaTime;
				if (Timer <= 0)
				{
					CheckDoesNotMigrate(LevelReplicatedActor, 1);
				}
			},
			5.0f);
	}

	// Replicated Level Actor on border. Server 4 should have Authority, again assuming that the Level is setup accordingly.
	{
		AddStep(
			TEXT("Replicated Level Actor On Border - Server 4 Has Authority"), FWorkerDefinition::AllWorkers, nullptr, nullptr,
			[this](float DeltaTime) {
				// Since this actor already was in level and we wait for timer in the previous step, we don't need to wait
				// in this one again.
				CheckDoesNotMigrate(LevelReplicatedActorOnBorder, 4);
			},
			5.0f);
	}

	// Non-replicated Level Actor. Each Server should have Authority over their instance, Clients don't.
	{
		AddStep(
			TEXT("Non-replicated Level Actor - Each Server has Authority, Client doesn't know"), FWorkerDefinition::AllWorkers, nullptr,
			nullptr,
			[this](float DeltaTime) {
				// Since this actor already was in level and we wait for timer in the previous step, we don't need to wait
				// in this one again.
				const FWorkerDefinition& LocalWorkerDefinition = GetLocalFlowController()->WorkerDefinition;
				if (LocalWorkerDefinition.Type == ESpatialFunctionalTestWorkerType::Server)
				{
					// Note: Non-replicated actors never get OnAuthorityGained() called.
					if (VerifyTestActor(LevelActor, ESpatialHasAuthority::ServerAuth, LocalWorkerDefinition.Id, LocalWorkerDefinition.Id, 0,
										0, 0, 0))
					{
						FinishStep();
					}
				}
				else
				{
					if (VerifyTestActor(LevelActor, ESpatialHasAuthority::ClientNonAuth, 0, 0, 0, 0, 0,0))
					{
						FinishStep(); // Clients don't have authority over non-replicated Level Actors.
					}
				}
			},
			5.0f);
	}

	// Replicated Dynamic Actor Spawned On Same Server. Server 1 should have Authority.
	{
		AddStep(TEXT("Replicated Dynamic Actor Spawned On Same Server - Spawn"), FWorkerDefinition::Server(1), nullptr, [this]() {
			ASpatialAuthorityTestReplicatedActor* Actor =
				GetWorld()->SpawnActor<ASpatialAuthorityTestReplicatedActor>(Server1Position, FRotator::ZeroRotator);
			CrossServerSetDynamicReplicatedActor(Actor);
			FinishStep();
		});

		AddStep(
			TEXT("Replicated Dynamic Actor Spawned On Same Server - Verify Server 1 Has Authority"), FWorkerDefinition::AllWorkers, nullptr,
			nullptr,
			[this](float DeltaTime) {
				Timer -= DeltaTime;
				if (Timer <= 0)
				{
					CheckDoesNotMigrate(DynamicReplicatedActor, 1);
				}
			},
			5.0f);

		AddStepFromDefinition(ReplicatedDestroyStepDefinition, FWorkerDefinition::Server(1));
	}

	// Replicated Dynamic Actor Spawned On Different Server. Server 1 should have Authority on BeginPlay, Server 2 on Tick
	{
		AddStep(TEXT("Replicated Dynamic Actor Spawned On Different Server - Spawn"), FWorkerDefinition::Server(1), nullptr, [this]() {
			ASpatialAuthorityTestReplicatedActor* Actor =
				GetWorld()->SpawnActor<ASpatialAuthorityTestReplicatedActor>(Server2Position, FRotator::ZeroRotator);
			CrossServerSetDynamicReplicatedActor(Actor);
			FinishStep();
		});

		AddStep(
			TEXT("Replicated Dynamic Actor Spawned On Different Server - Verify Server 1 Has Authority on BeginPlay and Server 2 "
				 "on Tick"),
			FWorkerDefinition::AllWorkers, nullptr, nullptr,
			[this](float DeltaTime) {
				Timer -= DeltaTime;
				if (Timer <= 0)
				{
					CheckMigration(1, 2);
				}
			},
			5.0f);

		AddStepFromDefinition(ReplicatedDestroyStepDefinition, FWorkerDefinition::Server(2));
	}

	// Non-replicated Dynamic Actor. Server 1 should have Authority.
	{
		AddStep(TEXT("Non-replicated Dynamic Actor - Spawn"), FWorkerDefinition::Server(1), nullptr, [this]() {
			// Spawning directly on Server 2, but since it's non-replicated it shouldn't migrate to Server 2.
			DynamicNonReplicatedActor = GetWorld()->SpawnActor<ASpatialAuthorityTestActor>(Server2Position, FRotator::ZeroRotator);
			FinishStep();
		});

		AddStepFromDefinition(NonReplicatedVerifyAuthorityStepDefinition, FWorkerDefinition::Server(1));

		AddStepFromDefinition(NonReplicatedVerifyNonAuthorityStepDefinition, FWorkerDefinition::AllWorkers);

		AddStepFromDefinition(NonReplicatedDestroyStepDefinition, FWorkerDefinition::Server(1));
	}

	// Replicated Dynamic Actor Spawned On Border from Server 1. Server 4 should get authority but server 1 will have authority on begin
	// play.
	{
		AddStep(TEXT("Replicated Dynamic Actor Spawned On Border From Server 1 - Spawn"), FWorkerDefinition::Server(1), nullptr, [this]() {
			ASpatialAuthorityTestReplicatedActor* Actor =
				GetWorld()->SpawnActor<ASpatialAuthorityTestReplicatedActor>(BorderPosition, FRotator::ZeroRotator);
			CrossServerSetDynamicReplicatedActor(Actor);
			FinishStep();
		});

		AddStep(
			TEXT("Replicated Dynamic Actor Spawned On Border From Server 1 - Verify Server 4 Has Authority"), FWorkerDefinition::AllWorkers,
			nullptr, nullptr,
			[this](float DeltaTime) {
				Timer -= DeltaTime;
				// Spawning directly on border, so it shouldn't migrate.
				if (Timer <= 0)
				{
					CheckMigration(1, 4);
				}
			},
			5.0f);

		AddStepFromDefinition(ReplicatedDestroyStepDefinition, FWorkerDefinition::Server(4));
	}

	// Replicated Dynamic Actor Spawned On Border from Server 2. Server 4 should get authority but server 2 will have authority on begin
	// play.
	{
		AddStep(TEXT("Replicated Dynamic Actor Spawned On Border From Server 2 - Spawn"), FWorkerDefinition::Server(2), nullptr, [this]() {
			ASpatialAuthorityTestReplicatedActor* Actor =
				GetWorld()->SpawnActor<ASpatialAuthorityTestReplicatedActor>(BorderPosition, FRotator::ZeroRotator);
			CrossServerSetDynamicReplicatedActor(Actor);
			FinishStep();
		});

		AddStep(
			TEXT("Replicated Dynamic Actor Spawned On Border From Server 2 - Verify Server 4 Has Authority"), FWorkerDefinition::AllWorkers,
			nullptr, nullptr,
			[this](float DeltaTime) {
				Timer -= DeltaTime;
				// Spawning directly on border, so it shouldn't migrate.
				if (Timer <= 0)
				{
					CheckMigration(2, 4);
				}
			},
			5.0f);

		AddStepFromDefinition(ReplicatedDestroyStepDefinition, FWorkerDefinition::Server(4));
	}

	// Replicated Dynamic Actor Spawned On Border from Server 3. Server 4 should get authority but server 3 will have authority on begin
	// play.
	{
		AddStep(TEXT("Replicated Dynamic Actor Spawned On Border From Server 3 - Spawn"), FWorkerDefinition::Server(3), nullptr, [this]() {
			ASpatialAuthorityTestReplicatedActor* Actor =
				GetWorld()->SpawnActor<ASpatialAuthorityTestReplicatedActor>(BorderPosition, FRotator::ZeroRotator);
			CrossServerSetDynamicReplicatedActor(Actor);
			FinishStep();
		});

		AddStep(
			TEXT("Replicated Dynamic Actor Spawned On Border From Server 3 - Verify Server 4 Has Authority"), FWorkerDefinition::AllWorkers,
			nullptr, nullptr,
			[this](float DeltaTime) {
				Timer -= DeltaTime;
				// Spawning directly on border, so it shouldn't migrate.
				if (Timer <= 0)
				{
					CheckMigration(3, 4);
				}
			},
			5.0f);

		AddStepFromDefinition(ReplicatedDestroyStepDefinition, FWorkerDefinition::Server(4));
	}

	// Replicated Dynamic Actor Spawned On Border from Server 4. Server 4 should keep authority
	// play.
	{
		AddStep(TEXT("Replicated Dynamic Actor Spawned On Border From Server 4 - Spawn"), FWorkerDefinition::Server(4), nullptr, [this]() {
			ASpatialAuthorityTestReplicatedActor* Actor =
				GetWorld()->SpawnActor<ASpatialAuthorityTestReplicatedActor>(BorderPosition, FRotator::ZeroRotator);
			CrossServerSetDynamicReplicatedActor(Actor);
			FinishStep();
		});

		AddStep(
			TEXT("Replicated Dynamic Actor Spawned On Border From Server 4 - Verify Server 4 Has Authority"), FWorkerDefinition::AllWorkers,
			nullptr, nullptr,
			[this](float DeltaTime) {
				Timer -= DeltaTime;
				// Spawning directly on border, so it shouldn't migrate.
				if (Timer <= 0)
				{
					CheckDoesNotMigrate(DynamicReplicatedActor, 4);
				}
			},
			5.0f);

		AddStepFromDefinition(ReplicatedDestroyStepDefinition, FWorkerDefinition::Server(4));
	}

	// Non-replicated Dynamic Actor On Border From Server 1. Server 1 should have Authority.
	{
		AddStep(TEXT("Non-replicated Dynamic Actor On Border From Server 1 - Spawn"), FWorkerDefinition::Server(1), nullptr, [this]() {
			// Spawning directly on border, but since it's non-replicated it shouldn't migrate.
			DynamicNonReplicatedActor = GetWorld()->SpawnActor<ASpatialAuthorityTestActor>(BorderPosition, FRotator::ZeroRotator);
			FinishStep();
		});

		AddStepFromDefinition(NonReplicatedVerifyAuthorityStepDefinition, FWorkerDefinition::Server(1));

		AddStepFromDefinition(NonReplicatedVerifyNonAuthorityStepDefinition, FWorkerDefinition::AllWorkers);

		AddStepFromDefinition(NonReplicatedDestroyStepDefinition, FWorkerDefinition::Server(1));
	}

	// Non-replicated Client Actor. Client 1 should have Authority.
	{
		AddStep(TEXT("Non-replicated Dynamic Actor Client - Spawn"), FWorkerDefinition::Client(1), nullptr, [this]() {
			// Spawning directly on Server 2, but since it's non-replicated it shouldn't migrate
			DynamicNonReplicatedActor = GetWorld()->SpawnActor<ASpatialAuthorityTestActor>(Server2Position, FRotator::ZeroRotator);
			FinishStep();
		});

		AddStep(
			TEXT("Non-replicated Dynamic Actor Client - Verify Authority on Client 1"), FWorkerDefinition::Client(1), nullptr, nullptr,
			[this](float DeltaTime) {
				// Not replicated so OnAuthorityGained() is not called.
				if (VerifyTestActor(DynamicNonReplicatedActor, ESpatialHasAuthority::ClientAuth, 1, 1, 0, 0, 0,0))
				{
					FinishStep();
				}
			},
			5.0f);

		AddStep(
			TEXT("Non-replicated Dynamic Actor Client - Verify Dynamic Actor doesn't exist on others"), FWorkerDefinition::AllWorkers,
			nullptr, nullptr,
			[this](float DeltaTime) {
				const FWorkerDefinition& LocalWorkerDefinition = GetLocalFlowController()->WorkerDefinition;
				if (LocalWorkerDefinition.Type == ESpatialFunctionalTestWorkerType::Client && LocalWorkerDefinition.Id == 1)
				{
					FinishStep();
				}
				else
				{
					Timer -= DeltaTime;
					if (Timer <= 0)
					{
						CheckNumActorsInLevel();
					}
				}
			},
			5.0f);

		AddStepFromDefinition(NonReplicatedDestroyStepDefinition, FWorkerDefinition::Client(1));
	}

	// GameMode.
	{
		AddStep(
			TEXT("GameMode - Determine Authority by every Server"), FWorkerDefinition::AllServers, nullptr, nullptr,
			[this](float DeltaTime) {
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
									&& AuthorityComponent->NumAuthorityGains == NumExpectedAuthorityGains
									&& AuthorityComponent->NumAuthorityLosses == 0;
				}
				else
				{
					bIsStateValid = AuthorityComponent->ReplicatedAuthWorkerIdOnBeginPlay != 0
									&& AuthorityComponent->AuthWorkerIdOnBeginPlay == 0 && AuthorityComponent->AuthWorkerIdOnTick == 0
									&& AuthorityComponent->NumAuthorityGains == 0 && AuthorityComponent->NumAuthorityLosses == 0;
				}

				if (bIsStateValid)
				{
					if (AuthorityComponent->AuthWorkerIdOnBeginPlay)
					{
						CrossServerNotifyHadAuthorityOverGameMode();
					}
					FinishStep();
				}
			},
			50.0f);
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
									&& AuthorityComponent->NumAuthorityGains == 1 && AuthorityComponent->NumAuthorityLosses == 0;
				}
				else
				{
					bIsStateValid = AuthorityComponent->ReplicatedAuthWorkerIdOnBeginPlay != 0
									&& AuthorityComponent->AuthWorkerIdOnBeginPlay == 0 && AuthorityComponent->AuthWorkerIdOnTick == 0
									&& AuthorityComponent->NumAuthorityGains == 0 && AuthorityComponent->NumAuthorityLosses == 0;
				}

				if (bIsStateValid)
				{
					if (AuthorityComponent->AuthWorkerIdOnBeginPlay > 0)
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
		AddStep(
			TEXT("Verify GameMode/State Unique Authority"), FWorkerDefinition::Server(1), nullptr, nullptr,
			[this](float DeltaTime) {
				if (NumHadAuthorityOverGameMode == 1 && NumHadAuthorityOverGameState == 1)
				{
					// Reset in case we run the Test multiple times in a row.
					NumHadAuthorityOverGameMode = 0;
					NumHadAuthorityOverGameState = 0;
					FinishStep();
				}
			},
			5.0f);
	}
}

void ASpatialAuthorityTest::CheckDoesNotMigrate(ASpatialAuthorityTestActor* Actor, int ServerId)
{
	const FWorkerDefinition& LocalWorkerDefinition = GetLocalFlowController()->WorkerDefinition;
	if (LocalWorkerDefinition.Type == ESpatialFunctionalTestWorkerType::Server)
	{
		// Allow it to continue working in Native / Single worker setups.
		if (GetNumberOfServerWorkers() > 1)
		{
			if (LocalWorkerDefinition.Type == ESpatialFunctionalTestWorkerType::Server && LocalWorkerDefinition.Id == ServerId)
			{
				if (VerifyTestActor(Actor, ESpatialHasAuthority::ServerAuth, ServerId, ServerId, 1, 0, 1, 0))
				{
					FinishStep();
				}
			}
			else if (LocalWorkerDefinition.Type == ESpatialFunctionalTestWorkerType::Server && Actor->bNetStartup)
			{
				// Startup actors receive OnActorReady on non-auth servers
				if (VerifyTestActor(Actor, ESpatialHasAuthority::ServerNonAuth, 0, 0, 0, 0, 0, 1))
				{
					FinishStep();
				}
			}
			else if (LocalWorkerDefinition.Type == ESpatialFunctionalTestWorkerType::Server)
			{
				// Dynamic actors do not receive OnActorReady on non-auth servers
				if (VerifyTestActor(Actor, ESpatialHasAuthority::ServerNonAuth, 0, 0, 0, 0, 0, 0))
				{
					FinishStep();
				}
			}
		}
		else // Support for Native / Single Worker.
		{
			if (VerifyTestActor(Actor, ESpatialHasAuthority::ServerAuth, 1, 1, 1, 0, 1,0))
			{
				FinishStep();
			}
		}
	}
	else // Clients
	{
		if (VerifyTestActor(Actor, ESpatialHasAuthority::ClientNonAuth, 0, 0, 0, 0, 0,0))
		{
			FinishStep();
		}
	}
}

void ASpatialAuthorityTest::CheckMigration(int StartServerId, int EndServerId)
{
	const FWorkerDefinition& LocalWorkerDefinition = GetLocalFlowController()->WorkerDefinition;
	if (LocalWorkerDefinition.Type == ESpatialFunctionalTestWorkerType::Server)
	{
		// Allow it to continue working in Native / Single worker setups.
		if (GetNumberOfServerWorkers() > 1)
		{
			if (LocalWorkerDefinition.Id == StartServerId)
			{
				// Note: An Actor always ticks on the spawning Worker before migrating.
				if (VerifyTestActor(DynamicReplicatedActor, ESpatialHasAuthority::ServerNonAuth, StartServerId, StartServerId, 1, 1, 1,0))
				{
					FinishStep();
				}
			}
			else if (LocalWorkerDefinition.Id == EndServerId)
			{
				if (VerifyTestActor(DynamicReplicatedActor, ESpatialHasAuthority::ServerAuth, 0, EndServerId, 1, 0, 0,0)
					&& DynamicReplicatedActor->AuthorityComponent->ReplicatedAuthWorkerIdOnBeginPlay == StartServerId)
				{
					FinishStep();
				}
			}
			else
			{
				if (VerifyTestActor(DynamicReplicatedActor, ESpatialHasAuthority::ServerNonAuth, 0, 0, 0, 0, 0,0))
				{
					FinishStep();
				}
			}
		}
		else // Support for Native / Single Worker.
		{
			if (VerifyTestActor(DynamicReplicatedActor, ESpatialHasAuthority::ServerAuth, 1, 1, 1, 0, 1,0))
			{
				FinishStep();
			}
		}
	}
	else // Clients.
	{
		if (VerifyTestActor(DynamicReplicatedActor, ESpatialHasAuthority::ClientNonAuth, 0, 0, 0, 0, 0,0))
		{
			FinishStep();
		}
	}
}
void ASpatialAuthorityTest::CheckNumActorsInLevel()
{
	int NumNonReplicatedActorsExpected = 1; // The one that is in the Map itself
	int NumNonReplicatedActorsInLevel = 0;
	for (TActorIterator<ASpatialAuthorityTestActor> It(GetWorld()); It; ++It)
	{
		if (!It->GetIsReplicated())
		{
			NumNonReplicatedActorsInLevel++;
		}
	}

	if (NumNonReplicatedActorsInLevel == NumNonReplicatedActorsExpected)
	{
		FinishStep();
	}
	else
	{
		FinishTest(EFunctionalTestResult::Failed, FString::Printf(TEXT("Was expecting only %d non replicated Actors, but found %d"),
																  NumNonReplicatedActorsExpected, NumNonReplicatedActorsInLevel));
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

bool ASpatialAuthorityTest::VerifyTestActor(ASpatialAuthorityTestActor* Actor, ESpatialHasAuthority ExpectedAuthority,
											int AuthorityOnBeginPlay, int AuthorityOnTick, int NumAuthorityGains, int NumAuthorityLosses,
											int NumActorReadyAuth, int NumActorReadyNonAuth)
{
	if (!IsValid(Actor) || !Actor->HasActorBegunPlay())
	{
		return false;
	}

	ESpatialHasAuthority ActualAuthority;
	USpatialStatics::SpatialSwitchHasAuthority(Actor, ActualAuthority);

	if (ActualAuthority != ExpectedAuthority)
	{
		return false;
	}

	return Actor->AuthorityComponent->AuthWorkerIdOnBeginPlay == AuthorityOnBeginPlay
		   && Actor->AuthorityComponent->AuthWorkerIdOnTick == AuthorityOnTick
		   && Actor->AuthorityComponent->NumAuthorityGains == NumAuthorityGains
		   && Actor->AuthorityComponent->NumAuthorityLosses == NumAuthorityLosses
		   && Actor->AuthorityComponent->NumActorReadyAuth == NumActorReadyAuth
		   && Actor->AuthorityComponent->NumActorReadyNonAuth == NumActorReadyNonAuth;
}
