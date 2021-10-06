// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "SpatialAuthorityTestGameMode.h"
#include "SpatialAuthorityTestActorComponent.h"
#include "SpatialAuthorityTestGameState.h"

ASpatialAuthorityTestGameMode::ASpatialAuthorityTestGameMode()
	: Super()
{
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.TickInterval = 0.0f; // Ensure fast Tick

	GameStateClass = ASpatialAuthorityTestGameState::StaticClass();

	AuthorityComponent = CreateDefaultSubobject<USpatialAuthorityTestActorComponent>(FName("AuthorityComponent"));

	RootComponent = AuthorityComponent;
}
