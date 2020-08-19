// Copyright (c) Improbable Worlds Ltd, All Rights Reserved


#include "SpatialAuthorityTestGameState.h"
#include "SpatialAuthorityTestActorComponent.h"

ASpatialAuthorityTestGameState::ASpatialAuthorityTestGameState()
	: Super()
{
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.TickInterval = 0.0f; // Ensure fast Tick

	AuthorityComponent = CreateDefaultSubobject<USpatialAuthorityTestActorComponent>(FName("AuthorityComponent"));

	RootComponent = AuthorityComponent;
}
