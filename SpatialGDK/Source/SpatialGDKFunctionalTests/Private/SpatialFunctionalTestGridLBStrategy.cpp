// Copyright (c) Improbable Worlds Ltd, All Rights Reserved


#include "SpatialFunctionalTestGridLBStrategy.h"
#include "GameFramework/Actor.h"

USpatialFunctionalTestGridLBStrategy::USpatialFunctionalTestGridLBStrategy()
{
	Rows = 2;
	Cols = 2;
	InterestBorder = 500.0f;
}

bool USpatialFunctionalTestGridLBStrategy::ShouldHaveAuthority(const AActor& Actor) const
{
	auto* Delegation = Delegations.Find(Actor.GetUniqueID());
	if (Delegation != nullptr)
	{
		return GetLocalVirtualWorkerId() == Delegation->WorkerId;
	}
	return Super::ShouldHaveAuthority(Actor);
}

VirtualWorkerId USpatialFunctionalTestGridLBStrategy::WhoShouldHaveAuthority(const AActor& Actor) const
{
	auto* Delegation = Delegations.Find(Actor.GetUniqueID());
	if (Delegation != nullptr)
	{
		return Delegation->WorkerId;
	}
	return Super::WhoShouldHaveAuthority(Actor);
}
