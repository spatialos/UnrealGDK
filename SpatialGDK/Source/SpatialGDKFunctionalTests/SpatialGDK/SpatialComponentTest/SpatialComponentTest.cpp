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

/** This Test is meant to check that you can add / remove actor components in the following component callbacks:
 *		- OnAuthorityGained
 *		- OnAuthorityLost
 *		- OnActorReady
 *		- OnClientOwnershipGained
 *		- OnClientOwnershipLost
 * For the following cases:
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

	// TODO - replicated level actor
	// -- check for 0 components before
	// -- check for X components after
}

void ASpatialComponentTest::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ASpatialComponentTest, DynamicReplicatedActor);
}

void ASpatialComponentTest::CrossServerSetDynamicReplicatedActor_Implementation(ASpatialComponentTestReplicatedActor* Actor)
{
	DynamicReplicatedActor = Actor;
}
