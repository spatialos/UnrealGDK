// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "SpatialComponentTestActorComponent.h"
#include "EngineClasses/SpatialNetDriver.h"
#include "LoadBalancing/AbstractLBStrategy.h"
#include "Net/UnrealNetwork.h"
#include "SpatialFunctionalTest.h"
#include "SpatialFunctionalTestFlowController.h"

USpatialComponentTestActorComponent::USpatialComponentTestActorComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.TickInterval = 0.0f;

	SetIsReplicatedByDefault(true);
}

void USpatialComponentTestActorComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
}

void USpatialComponentTestActorComponent::OnAuthorityGained()
{
	// TODO: Add 2 components

	// TODO: Remove 1 component
}

void USpatialComponentTestActorComponent::OnAuthorityLost()
{
	// TODO: Add 2 components

	// TODO: Remove 1 component
}

void USpatialComponentTestActorComponent::OnActorReady(bool bHasAuthority)
{
	if (bHasAuthority)
	{
		// TODO: Add 2 components

		// TODO: Remove 1 component
	}
	else
	{
		// Do nothing?
	}
}
