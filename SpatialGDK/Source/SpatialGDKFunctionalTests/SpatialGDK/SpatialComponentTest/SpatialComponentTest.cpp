// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "SpatialComponentTest.h"
#include "Engine/World.h"
#include "Net/UnrealNetwork.h"
#include "SpatialComponentTestActor.h"
#include "SpatialComponentTestCallbackComponent.h"
#include "SpatialComponentTestDummyComponent.h"
#include "SpatialComponentTestReplicatedActor.h"
#include "SpatialFunctionalTestFlowController.h"
#include "SpatialGDK/Public/EngineClasses/SpatialNetDriver.h"
#include "SpatialGDK/Public/Utils/SpatialStatics.h"

/** This Test is meant to check that you can add / remove components on spatial component callbacks for the following cases:
 *		- replicated level actor
 *		- dynamic replicated actor (spawned on the same server)
 *		- dynamic replicated actor (spawned cross server)
 * Keep in mind that we're assuming a 2x1 Grid Load-Balancing Strategy, otherwise the ownership of
 * these actors may be something completely different (specially important for actors placed in the Level).
 */
ASpatialComponentTest::ASpatialComponentTest()
{
	Author = "Victoria Bloom";
	Description = TEXT("Test GDK component callbacks");

	Server1Position = FVector(-250.0f, -250.0f, 0.0f);
	Server2Position = FVector(250.0f, -250.0f, 0.0f);
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
			TEXT("Replicated Level Actor - Verify Server Components"), FWorkerDefinition::AllWorkers, nullptr, nullptr,
			[this](float DeltaTime) {
				Timer -= DeltaTime;
				if (Timer <= 0)
				{
					CheckComponents(LevelReplicatedActor, 1);
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
			TEXT("Replicated Dynamic Actor Spawned On Same Server - Verify components"), FWorkerDefinition::AllWorkers, nullptr,
			nullptr,
			[this](float DeltaTime) {
				Timer -= DeltaTime;
				if (Timer <= 0)
				{
					CheckComponents(DynamicReplicatedActor, 1, 0);
				}
			},
			5.0f);

		// Client 1 to get ownership of the dynamic actor - then check components
		AddStep(TEXT("Replicated Dynamic Actor Spawned On Same Server - Set owner to client 1"), FWorkerDefinition::Server(1), nullptr,
				[this]() {
					AController* PlayerController =
						Cast<AController>(GetFlowController(ESpatialFunctionalTestWorkerType::Client, 1)->GetOwner());
					DynamicReplicatedActor->SetOwner(PlayerController);
					FinishStep();
				});

		AddStep(
			TEXT("Replicated Dynamic Actor Spawned On Same Server - Verify components"), FWorkerDefinition::AllWorkers, nullptr,
			nullptr,
			[this](float DeltaTime) {
				Timer -= DeltaTime;
				if (Timer <= 0)
				{
					// FinishStep();
					CheckComponents(DynamicReplicatedActor, 1, 1, 0);
				}
			},
			5.0f);

		// Client 2 to get ownership of the dynamic actor - then check components
		AddStep(TEXT("Replicated Dynamic Actor Spawned On Same Server - Set owner to client 2"), FWorkerDefinition::Server(1), nullptr,
				[this]() {
					AController* PlayerController =
						Cast<AController>(GetFlowController(ESpatialFunctionalTestWorkerType::Client, 2)->GetOwner());
					DynamicReplicatedActor->SetOwner(PlayerController);
					FinishStep();
				});

		AddStep(
			TEXT("Replicated Dynamic Actor Spawned On Same Server - Verify components"), FWorkerDefinition::AllWorkers, nullptr,
			nullptr,
			[this](float DeltaTime) {
				Timer -= DeltaTime;
				if (Timer <= 0)
				{
					CheckComponents(DynamicReplicatedActor, 1, 2, 1);
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
			TEXT("Replicated Dynamic Actor Spawned On Different Server - Verify components "
				 "on Tick"),
			FWorkerDefinition::AllWorkers, nullptr, nullptr,
			[this](float DeltaTime) {
				Timer -= DeltaTime;
				if (Timer <= 0)
				{
					CheckComponentsCrossServer(DynamicReplicatedActor, 1, 2);
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

// Checks the number of components on the servers when an actor does not migrate
void ASpatialComponentTest::CheckComponents(ASpatialComponentTestActor* Actor, int ExpectedServerId, int ExpectedClient1ComponentCount,
											int ExpectedClient2ComponentCount)
{
	const FWorkerDefinition& LocalWorkerDefinition = GetLocalFlowController()->WorkerDefinition;
	if (LocalWorkerDefinition.Type == ESpatialFunctionalTestWorkerType::Server)
	{
		// Allow it to continue working in Native / Single worker setups.
		if (GetNumberOfServerWorkers() > 1)
		{
			if (LocalWorkerDefinition.Id == ExpectedServerId)
			{
				// Server auth - OnAuthorityGained component and OnActorReady component
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
	else
	{
		if (LocalWorkerDefinition.Id == 1)
		{
			// Client 1
			if (VerifyTestActorComponents(Actor, ExpectedClient1ComponentCount))
			{
				FinishStep();
			}
		}
		else if (LocalWorkerDefinition.Id == 2)
		{
			// Client 2
			if (VerifyTestActorComponents(Actor, ExpectedClient2ComponentCount))
			{
				FinishStep();
			}
		}
	}
}

// Checks the number of components when an actor migrates
void ASpatialComponentTest::CheckComponentsCrossServer(ASpatialComponentTestActor* Actor, int StartServerId, int EndServerId)
{
	const FWorkerDefinition& LocalWorkerDefinition = GetLocalFlowController()->WorkerDefinition;
	if (LocalWorkerDefinition.Type == ESpatialFunctionalTestWorkerType::Server)
	{
		// Allow it to continue working in Native / Single worker setups.
		if (GetNumberOfServerWorkers() > 1)
		{
			if (LocalWorkerDefinition.Id == StartServerId)
			{
				// Spawning server - OnActorReady component, OnAuthorityGained component and OnAuthorityLost component
				if (VerifyTestActorComponents(Actor, 3))
				{
					FinishStep();
				}
			}
			else if (LocalWorkerDefinition.Id == EndServerId)
			{
				// Migrated server - OnAuthorityGained component
				if (VerifyTestActorComponents(Actor, 1))
				{
					FinishStep();
				}
			}
		}
		else // Support for Native / Single Worker.
		{
			// Spawning server - OnActorReady component and OnAuthorityGained component
			if (VerifyTestActorComponents(Actor, 2))
			{
				FinishStep();
			}
		}
	}
	else
	{
		// Clients
		if (VerifyTestActorComponents(Actor, 0))
		{
			FinishStep();
		}
	}
}

bool ASpatialComponentTest::VerifyTestActorComponents(ASpatialComponentTestActor* Actor, int ExpectedTestComponentCount)
{
	TArray<UActorComponent*> FoundComponents = Actor->GetComponentsByClass(USpatialComponentTestDummyComponent::StaticClass());
	int FoundTestComponentCount = FoundComponents.Num();
	return FoundTestComponentCount == ExpectedTestComponentCount;
}

void ASpatialComponentTest::CrossServerSetDynamicReplicatedActor_Implementation(ASpatialComponentTestReplicatedActor* Actor)
{
	DynamicReplicatedActor = Actor;
}
