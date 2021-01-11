// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "SpatialComponentTestDummyComponent.h"

USpatialComponentTestDummyComponent::USpatialComponentTestDummyComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void USpatialComponentTestDummyComponent::OnAuthorityGained()
{
	NumAuthorityGains++;
}

void USpatialComponentTestDummyComponent::OnAuthorityLost()
{
	NumAuthorityLosses++;
}

void USpatialComponentTestDummyComponent::OnActorReady(bool bHasAuthority)
{
	if (bHasAuthority)
	{
		NumActorReadyAuth++;
	}
	else
	{
		NumActorReadyNonAuth++;
	}
}

void USpatialComponentTestDummyComponent::OnClientOwnershipGained()
{
	NumClientOwnershipGains++;
}

void USpatialComponentTestDummyComponent::OnClientOwnershipLost()
{
	NumClientOwnershipLosses++;
}
