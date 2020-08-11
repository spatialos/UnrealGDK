// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "SpatialAuthorityTest.h"
#include "SpatialAuthorityTestActor.h"
#include "SpatialAuthorityTestReplicatedActor.h"
#include "SpatialFunctionalTestFlowController.h"
#include "Engine/World.h"

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

	// Replicated Level Actor
	{
		AddStep(TEXT("Replicated Level Actor - Server 1 Has Authority"), FWorkerDefinition::AllWorkers,	nullptr, nullptr,
			[this](ASpatialFunctionalTest* Test, float DeltaTime) {
				if (LevelReplicatedActor->AuthorityOnBeginPlay == 1 && LevelReplicatedActor->AuthorityOnTick == 1)
				{
					FinishStep();
				}
			}, 5.0f);
	}

	// Non-replicated Level Actor
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
						FinishStep(); // Clients don't have authority over Level non replicated
					}
				}
			}, 5.0f);
	}

	// Replicated Dynamic Actor Spawned On Same Server
	{
		AddStep(TEXT("Replicated Dynamic Actor Spawned On Same Server - Spawn"), FWorkerDefinition::Server(1), nullptr,
			[](ASpatialFunctionalTest* Test)
			{
				Test->GetWorld()->SpawnActor<ASpatialAuthorityTestReplicatedActor>(ASpatialAuthorityTestReplicatedActor::StaticClass(), Test->GetActorTransform());
			});
	}

	// Non-replicated Dynamic Actor
	{

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

	//DOREPLIFETIME(ARPCInInterfaceTest, TestActor);
}
