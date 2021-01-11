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

	// Replicated Level Actor. Server 1 should have Authority, again assuming that the Level is setup accordingly.
	{
		AddStep(
			TEXT("Replicated Level Actor - Verify Server Components"), FWorkerDefinition::AllServers, nullptr, nullptr,
			[this](float DeltaTime) {
				CheckComponentsOnServer(LevelReplicatedActor, 1);
			},
			5.0f);
		AddStep(
			TEXT("Replicated Level Actor - Verify Client Components"), FWorkerDefinition::AllClients, nullptr, nullptr,
			[this](float DeltaTime) {
				RequireTrue(VerifyClientComponents(LevelReplicatedActor), "No client components expected");
				RequireTrue(false, "No client components expected");
				FinishStep();
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
			TEXT("Replicated Dynamic Actor Spawned On Same Server - Verify Server Components"), FWorkerDefinition::AllServers, nullptr,
			nullptr,
			[this](float DeltaTime) {
				CheckComponentsOnServer(DynamicReplicatedActor, 1);
			},
			5.0f);
		AddStep(
			TEXT("Replicated Dynamic Actor Spawned On Same Server - Verify Client Components"), FWorkerDefinition::AllClients, nullptr,
			nullptr,
			[this](float DeltaTime) {
				RequireTrue(VerifyClientComponents(DynamicReplicatedActor), "No client components expected");
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
			TEXT("Replicated Dynamic Actor Spawned On Same Server - Verify Server Components"), FWorkerDefinition::AllServers, nullptr,
			nullptr,
			[this](float DeltaTime) {
				CheckComponentsOnServer(DynamicReplicatedActor, 1);
			},
			5.0f);

		AddStep(
			TEXT("Replicated Dynamic Actor Spawned On Same Server - Verify Client Components"), FWorkerDefinition::AllClients, nullptr,
			nullptr,
			[this](float DeltaTime) {
				const FWorkerDefinition& LocalWorkerDefinition = GetLocalFlowController()->WorkerDefinition;
				if (LocalWorkerDefinition.Id == 1)
				{
					RequireTrue(VerifyClientComponents(DynamicReplicatedActor, 1, 1, 0), "Client 1 - OnClientOwnership gained component");
					FinishStep();
				}
				else if (LocalWorkerDefinition.Id == 2)
				{
					RequireTrue(VerifyClientComponents(DynamicReplicatedActor), "Client 2 - No components");
					FinishStep();
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
			TEXT("Replicated Dynamic Actor Spawned On Same Server - Verify Server Components"), FWorkerDefinition::AllServers, nullptr,
			nullptr,
			[this](float DeltaTime) {
				CheckComponentsOnServer(DynamicReplicatedActor, 1);
			},
			5.0f);

		AddStep(
			TEXT("Replicated Dynamic Actor Spawned On Same Server - Verify Client Components"), FWorkerDefinition::AllClients, nullptr,
			nullptr,
			[this](float DeltaTime) {
				const FWorkerDefinition& LocalWorkerDefinition = GetLocalFlowController()->WorkerDefinition;
				if (LocalWorkerDefinition.Id == 1)
				{
					RequireTrue(VerifyClientComponents(DynamicReplicatedActor, 2, 1, 1),
								"Client 1 - OnClientOwnershipGained component and OnClientOwnership lost component");
					FinishStep();
				}
				else if (LocalWorkerDefinition.Id == 2)
				{
					RequireTrue(VerifyClientComponents(DynamicReplicatedActor, 1, 1, 0), "Client 2 - OnClientOwnershipGained component");
					FinishStep();
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
				CheckComponentsCrossServer(DynamicReplicatedActor, 1, 2);
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

// Checks the number of dynamically added components and their callbacks on the servers and clients when an actor does not migrate
void ASpatialComponentTest::CheckComponentsOnServer(ASpatialComponentTestActor* Actor, int ExpectedServerId)
{
	const FWorkerDefinition& LocalWorkerDefinition = GetLocalFlowController()->WorkerDefinition;
	if (LocalWorkerDefinition.Type == ESpatialFunctionalTestWorkerType::Server)
	{
		// Allow it to continue working in Native / Single worker setups.
		if (GetNumberOfServerWorkers() > 1)
		{
			if (LocalWorkerDefinition.Id == ExpectedServerId)
			{
				RequireTrue(VerifyServerComponents(Actor, 2, 1, 0, 1, 0),
							"Server auth - OnAuthorityGained component and OnActorReady component");
				FinishStep();
			}
			else if (Actor->bNetStartup)
			{
				RequireTrue(VerifyServerComponents(Actor, 1, 0, 0, 0, 1), "Non-auth servers - Level actors receive OnActorReady only");
				FinishStep();
			}
			else
			{
				RequireTrue(VerifyServerComponents(Actor),
							"Non-auth servers - Dynamic actors do not receive OnActorReady or OnAuthorityGained ");
				FinishStep();
			}
		}
		else // Support for Native / Single Worker.
		{
			RequireTrue(VerifyServerComponents(Actor, 2, 1, 0, 1, 0),
						"Native / Single Worker - OnActorReady component and OnAuthorityGained component");
			FinishStep();
		}
	}
}

// Checks the number of dynamically added components and their callbacks on the servers and clients when an actor migrates
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
				RequireTrue(VerifyServerComponents(Actor, 3),
							"Spawning server - OnActorReady component, OnAuthorityGained component and OnAuthorityLost component");
				FinishStep();
			}
			else if (LocalWorkerDefinition.Id == EndServerId)
			{
				RequireTrue(VerifyServerComponents(Actor, 1), "Migrated server - OnAuthorityGained component");
				FinishStep();
			}
		}
		else // Support for Native / Single Worker.
		{
			RequireTrue(VerifyServerComponents(Actor, 2),
						"Native / Single Worker - OnActorReady component and OnAuthorityGained component");
			FinishStep();
		}
	}
	else // Clients
	{
		RequireTrue(VerifyClientComponents(Actor, 0), "No client components expected");
		FinishStep();
	}
}

bool ASpatialComponentTest::VerifyServerComponents(ASpatialComponentTestActor* Actor, int ExpectedComponentCount, int NumAuthorityGains,
												   int NumAuthorityLosses, int NumActorReadyAuth, int NumActorReadyNonAuth)
{
	TArray<UActorComponent*> FoundComponents = GetDynamicComponents(Actor);
	int FoundComponentCount = FoundComponents.Num();

	// Check each dynamically added test component has the expected number of spatial callbacks
	for (UActorComponent* FoundComponent : FoundComponents)
	{
		USpatialComponentTestDummyComponent* DynamicComponent = Cast<USpatialComponentTestDummyComponent>(FoundComponent);
		if (FoundComponentCount != ExpectedComponentCount && DynamicComponent->NumAuthorityGains != NumAuthorityGains
			&& DynamicComponent->NumAuthorityLosses != NumAuthorityLosses && DynamicComponent->NumActorReadyAuth != NumActorReadyAuth
			&& DynamicComponent->NumActorReadyNonAuth != NumActorReadyNonAuth)
		{
			return false;
		}
	}

	return true;
}

bool ASpatialComponentTest::VerifyClientComponents(ASpatialComponentTestActor* Actor, int ExpectedComponentCount,
												   int NumClientOwnershipGained, int NumClientOwnershipLost)
{
	TArray<UActorComponent*> FoundComponents = GetDynamicComponents(Actor);
	int FoundComponentCount = FoundComponents.Num();

	// Check each dynamically added test component has the expected number of spatial callbacks
	for (UActorComponent* FoundComponent : FoundComponents)
	{
		USpatialComponentTestDummyComponent* DynamicComponent = Cast<USpatialComponentTestDummyComponent>(FoundComponent);
		if (FoundComponentCount != ExpectedComponentCount && DynamicComponent->NumClientOwnershipGains != NumClientOwnershipGained
			&& DynamicComponent->NumClientOwnershipLosses != NumClientOwnershipLost)
		{
			return false;
		}
	}

	return true;
}

TArray<UActorComponent*> ASpatialComponentTest::GetDynamicComponents(ASpatialComponentTestActor* Actor)
{
	if (!IsValid(Actor) || !Actor->HasActorBegunPlay())
	{
		TArray<UActorComponent*> Components;
		return Components;
	}

	return Actor->GetComponentsByClass(USpatialComponentTestDummyComponent::StaticClass());
}

void ASpatialComponentTest::CrossServerSetDynamicReplicatedActor_Implementation(ASpatialComponentTestReplicatedActor* Actor)
{
	DynamicReplicatedActor = Actor;
}
