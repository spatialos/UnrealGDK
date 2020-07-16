// Copyright (c) Improbable Worlds Ltd, All Rights Reserved


#include "SpatialFunctionalTestGridLBStrategy.h"
#include "GameFramework/Actor.h"
#include "SpatialFunctionalTestWorkerDelegationComponent.h"

USpatialFunctionalTestGridLBStrategy::USpatialFunctionalTestGridLBStrategy()
	: Super()
{
	Rows = 2;
	Cols = 2;
}

bool USpatialFunctionalTestGridLBStrategy::ShouldHaveAuthority(const AActor& Actor) const
{
	USpatialFunctionalTestWorkerDelegationComponent* DelegationComponent = Actor.FindComponentByClass<USpatialFunctionalTestWorkerDelegationComponent>();

	if (DelegationComponent != nullptr)
	{
		return GetLocalVirtualWorkerId() == DelegationComponent->WorkerId;
	}
	return Super::ShouldHaveAuthority(Actor);
}

VirtualWorkerId USpatialFunctionalTestGridLBStrategy::WhoShouldHaveAuthority(const AActor& Actor) const
{
	USpatialFunctionalTestWorkerDelegationComponent* DelegationComponent = Actor.FindComponentByClass<USpatialFunctionalTestWorkerDelegationComponent>();

	if (DelegationComponent != nullptr)
	{
		return DelegationComponent->WorkerId;
	}
	return Super::WhoShouldHaveAuthority(Actor);
}
