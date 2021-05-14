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
	Server2Position = FVector(-250.0f, 250.0f, 0.0f);
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

	// Replicated Level Actor. Server 1 should have Authority, again assuming that the Level is setup accordingly.
	{
		AddStep(
			TEXT("Replicated Level Actor - Verify Server Components"), FWorkerDefinition::AllWorkers, nullptr, nullptr,
			[this](float DeltaTime) {
				CheckComponents(LevelReplicatedActor, 1, 0, 0);
				FinishStep();
			},
			5.0f);
	}

	// Replicated Dynamic Actor Spawned On Same Server. Server 1 should have Authority.
	{
		AddStep(TEXT("Replicated Dynamic Actor Spawned On Same Server - Spawn"), FWorkerDefinition::Server(1), nullptr, [this]() {
			ASpatialComponentTestReplicatedActor* Actor =
				GetWorld()->SpawnActor<ASpatialComponentTestReplicatedActor>(Server1Position, FRotator::ZeroRotator);
			CrossServerSetDynamicReplicatedActor(Actor);
			FinishStep();
		});

		AddStep(
			TEXT("Replicated Dynamic Actor Spawned On Same Server - Verify components"), FWorkerDefinition::AllWorkers,
			[this]() -> bool {
				return IsValid(DynamicReplicatedActor);
			},
			nullptr,
			[this](float DeltaTime) {
				CheckComponents(DynamicReplicatedActor, 1, 0, 0);
				FinishStep();
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
			TEXT("Replicated Dynamic Actor Spawned On Same Server - Verify components"), FWorkerDefinition::AllWorkers, nullptr, nullptr,
			[this](float DeltaTime) {
				// Client 1 OnClientOwnershipGained component and Client 2 no events expected
				CheckComponents(DynamicReplicatedActor, 1, 1, 0);
				FinishStep();
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
			TEXT("Replicated Dynamic Actor Spawned On Same Server - Verify components"), FWorkerDefinition::AllWorkers, nullptr, nullptr,
			[this](float DeltaTime) {
				// Client 1 OnClientOwnershipGained component and OnClientOwnershipLost component and Client 2 OnClientOwnershipGained
				// component
				CheckComponents(DynamicReplicatedActor, 1, 2, 1);
				FinishStep();
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
				CheckComponentsCrossServer(DynamicReplicatedActor, 1, 2);
				FinishStep();
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

// Checks the number of components on the servers and clients when an actor does not migrate
void ASpatialComponentTest::CheckComponents(ASpatialComponentTestActor* Actor, int ExpectedServerId, int ExpectedClient1ComponentCount,
											int ExpectedClient2ComponentCount)
{
	if (!RequireTrue(IsValid(Actor), TEXT("Actor is valid")))
	{
		return;
	}

	if (!RequireTrue(Actor->HasActorBegunPlay(), TEXT("Actor has begun play")))
	{
		return;
	}

	const FWorkerDefinition& LocalWorkerDefinition = GetLocalFlowController()->WorkerDefinition;
	if (LocalWorkerDefinition.Type == ESpatialFunctionalTestWorkerType::Server)
	{
		// Allow it to continue working in Native / Single worker setups.
		if (GetNumberOfServerWorkers() > 1)
		{
			if (LocalWorkerDefinition.Id == ExpectedServerId)
			{
				VerifyTestActorComponents(Actor, 2, TEXT("Server auth - OnAuthorityGained component and OnActorReady component"));
			}
			else if (Actor->bNetStartup)
			{
				VerifyTestActorComponents(Actor, 1, TEXT("Non-auth servers - Level actors receive OnActorReady only OnAuthorityGained"));
			}
			else
			{
				VerifyTestActorComponents(Actor, 0, TEXT("Non-auth servers - Dynamic actors do not receive OnActorReady"));
			}
		}
		else // Support for Native / Single Worker.
		{
			VerifyTestActorComponents(Actor, 2, TEXT("Native / Single Worker - OnActorReady component and OnAuthorityGained component"));
		}
	}
	else // Clients
	{
		if (LocalWorkerDefinition.Id == 1)
		{
			VerifyTestActorComponents(Actor, ExpectedClient1ComponentCount, TEXT("Client 1"));
		}
		else if (LocalWorkerDefinition.Id == 2)
		{
			VerifyTestActorComponents(Actor, ExpectedClient2ComponentCount, TEXT("Client 2"));
		}
	}
}

// Checks the number of components on the servers and clients when an actor migrates
void ASpatialComponentTest::CheckComponentsCrossServer(ASpatialComponentTestActor* Actor, int StartServerId, int EndServerId)
{
	if (!RequireTrue(IsValid(Actor), TEXT("Actor is valid")))
	{
		return;
	}

	if (!RequireTrue(Actor->HasActorBegunPlay(), TEXT("Actor has begun play")))
	{
		return;
	}

	const FWorkerDefinition& LocalWorkerDefinition = GetLocalFlowController()->WorkerDefinition;
	if (LocalWorkerDefinition.Type == ESpatialFunctionalTestWorkerType::Server)
	{
		// Allow it to continue working in Native / Single worker setups.
		if (GetNumberOfServerWorkers() > 1)
		{
			if (LocalWorkerDefinition.Id == StartServerId)
			{
				VerifyTestActorComponents(
					Actor, 3, TEXT("Spawning server - OnActorReady component, OnAuthorityGained component and OnAuthorityLost component"));
			}
			else if (LocalWorkerDefinition.Id == EndServerId)
			{
				VerifyTestActorComponents(Actor, 1, TEXT("Migrated server - OnAuthorityGained component"));
			}
		}
		else // Support for Native / Single Worker.
		{
			VerifyTestActorComponents(Actor, 2, TEXT("Native / Single Worker - OnActorReady component and OnAuthorityGained component"));
		}
	}
	else // Clients
	{
		VerifyTestActorComponents(Actor, 0, TEXT("Clients"));
	}
}

bool ASpatialComponentTest::VerifyTestActorComponents(ASpatialComponentTestActor* Actor, int ExpectedTestComponentCount,
													  const FString& Message)
{
	return RequireEqual_Int(GetComponentsCount(Actor), ExpectedTestComponentCount,
							FString::Printf(TEXT("(%s) Component count correct"), *Message));
}

int32 ASpatialComponentTest::GetComponentsCount(ASpatialComponentTestActor* Actor)
{
	TArray<UActorComponent*> FoundComponents;
	Actor->GetComponents(USpatialComponentTestDummyComponent::StaticClass(), FoundComponents);
	return FoundComponents.Num();
}

void ASpatialComponentTest::CrossServerSetDynamicReplicatedActor_Implementation(ASpatialComponentTestReplicatedActor* Actor)
{
	DynamicReplicatedActor = Actor;
}
