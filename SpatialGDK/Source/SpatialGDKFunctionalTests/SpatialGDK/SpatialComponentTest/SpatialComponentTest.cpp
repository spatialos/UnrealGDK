// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "SpatialComponentTest.h"
#include "Engine/World.h"
#include "Net/UnrealNetwork.h"
#include "SpatialComponentTestActor.h"
#include "SpatialComponentTestActorComponent.h"
#include "SpatialComponentTestReplicatedActor.h"
#include "SpatialFunctionalTestFlowController.h"
#include "SpatialGDK/Public/EngineClasses/SpatialNetDriver.h"
#include "SpatialGDK/Public/Utils/SpatialStatics.h"

/** This Test is meant to check that you can add / remove actor components for the following cases:
 *		- replicated level actor
 *		- dynamic replicated actor (one time with spatial authority and another without)
 *		- non-replicated actor spawned on client
 * Keep in mind that we're assuming a 2x2 Grid Load-Balancing Strategy, otherwise the ownership of
 * these actors may be something completely different (specially important for actors placed in the Level).
 * You have some flexibility to change the Server Position properties to test in different Load-Balancing Strategies.
 */
ASpatialComponentTest::ASpatialComponentTest()
{
	Author = "Victoria Bloom";
	Description = TEXT("Test GDK component callbacks");

	Server1Position = FVector(-250.0f, -250.0f, 0.0f);
	Server2Position = FVector(250.0f, -250.0f, 0.0f);
	Server3Position = FVector(-250.0f, 250.0f, 0.0f);
	Server4Position = FVector(250.0f, 250.0f, 0.0f);
	BorderPosition = FVector(0.0f, 0.0f, 0.0f);
}

void ASpatialComponentTest::PrepareTest()
{
	Super::PrepareTest();

	// Reusable functional test steps

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

	// Replicated Level Actor. Server 1 should have Authority, again assuming that the Level is setup accordingly.
	{
		AddStep(
			TEXT("Replicated Level Actor - Server 1 Has Authority"), FWorkerDefinition::AllWorkers, nullptr, nullptr,
			[this](float DeltaTime) {
				Timer -= DeltaTime;
				if (Timer <= 0)
				{
					CheckComponentsNonMigration(LevelReplicatedActor, 1);
				}
			},
			5.0f);
	}

	// Replicated Dynamic Actor Spawned On Same Server.Server 1 should have Authority.
	{
		AddStep(TEXT("Replicated Dynamic Actor Spawned On Same Server - Spawn"), FWorkerDefinition::Server(1), nullptr, [this]() {
			ASpatialComponentTestReplicatedActor* Actor =
				GetWorld()->SpawnActor<ASpatialComponentTestReplicatedActor>(Server1Position, FRotator::ZeroRotator);
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
					CheckComponentsNonMigration(DynamicReplicatedActor, 1);
				}
			},
			5.0f);

		AddStepFromDefinition(ReplicatedDestroyStepDefinition, FWorkerDefinition::Server(1));
	}

	// Replicated Dynamic Actor Spawned On Different Server. Server 1 should have Authority on BeginPlay, Server 2 on Tick
	{
		AddStep(TEXT("Replicated Dynamic Actor Spawned On Different Server - Spawn"), FWorkerDefinition::Server(1), nullptr, [this]() {
			ASpatialComponentTestReplicatedActor* Actor =
				GetWorld()->SpawnActor<ASpatialComponentTestReplicatedActor>(Server2Position, FRotator::ZeroRotator);
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
					CheckComponentsMigration(DynamicReplicatedActor, 1, 2);
				}
			},
			5.0f);

		AddStepFromDefinition(ReplicatedDestroyStepDefinition, FWorkerDefinition::Server(2));
	}
}

void ASpatialComponentTest::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ASpatialComponentTest, DynamicReplicatedActor);
}

// Checks the number of components when an actor does not migrate
void ASpatialComponentTest::CheckComponentsNonMigration(ASpatialComponentTestActor* Actor, int ExpectedServerId)
{
	const FWorkerDefinition& LocalWorkerDefinition = GetLocalFlowController()->WorkerDefinition;
	if (LocalWorkerDefinition.Type == ESpatialFunctionalTestWorkerType::Server)
	{
		// Allow it to continue working in Native / Single worker setups.
		if (GetNumberOfServerWorkers() > 1)
		{
			if (LocalWorkerDefinition.Id == ExpectedServerId)
			{
				// Server auth
				if (VerifyTestActorComponents(Actor, 2))
				{
					FinishStep();
				}
			}
			else if (Actor->bNetStartup)
			{
				// Level actors receive OnActorReady on non-auth servers
				if (VerifyTestActorComponents(Actor, 1))
				{
					FinishStep();
				}
			}
			else
			{
				// Dynamic actors do not receive OnActorReady on non-auth servers
				if (VerifyTestActorComponents(Actor, 0))
				{
					FinishStep();
				}
			}
		}
		else // Support for Native / Single Worker.
		{
			if (VerifyTestActorComponents(Actor, 2))
			{
				FinishStep();
			}
		}
	}
	else // Clients
	{
		// Client non-auth
		if (VerifyTestActorComponents(Actor, 0))
		{
			FinishStep();
		}
	}
}

// Checks the number of components when an actor migrates
void ASpatialComponentTest::CheckComponentsMigration(ASpatialComponentTestActor* Actor, int StartServerId, int EndServerId)
{
	const FWorkerDefinition& LocalWorkerDefinition = GetLocalFlowController()->WorkerDefinition;
	if (LocalWorkerDefinition.Type == ESpatialFunctionalTestWorkerType::Server)
	{
		// Allow it to continue working in Native / Single worker setups.
		if (GetNumberOfServerWorkers() > 1)
		{
			if (LocalWorkerDefinition.Id == StartServerId)
			{
				if (VerifyTestActorComponents(Actor, 3))
				{
					FinishStep();
				}
			}
			else if (LocalWorkerDefinition.Id == EndServerId)
			{
				if (VerifyTestActorComponents(Actor, 1))
				{
					FinishStep();
				}
			}
			else
			{
				if (VerifyTestActorComponents(Actor, 0))
				{
					FinishStep();
				}
			}
		}
		else // Support for Native / Single Worker.
		{
			if (VerifyTestActorComponents(Actor, 2))
			{
				FinishStep();
			}
		}
	}
	else // Clients.
	{
		if (VerifyTestActorComponents(Actor, 0))
		{
			FinishStep();
		}
	}
}

bool ASpatialComponentTest::VerifyTestActorComponents(ASpatialComponentTestActor* Actor, int ExpectedTestComponentCount)
{
	TArray<UActorComponent*> FoundComponents = Actor->GetComponentsByClass(UStaticMeshComponent::StaticClass());
	int FoundTestComponentCount = FoundComponents.Num();
	return FoundTestComponentCount == ExpectedTestComponentCount;
}

void ASpatialComponentTest::CrossServerSetDynamicReplicatedActor_Implementation(ASpatialComponentTestReplicatedActor* Actor)
{
	DynamicReplicatedActor = Actor;
}
