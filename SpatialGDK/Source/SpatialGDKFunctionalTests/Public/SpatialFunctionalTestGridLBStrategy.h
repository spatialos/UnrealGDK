// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "LoadBalancing/GridBasedLBStrategy.h"
#include "SpatialFunctionalTestLBDelegationInterface.h"
#include "SpatialFunctionalTestGridLBStrategy.generated.h"

/**
 * A 2 by 2 (rows by columns) load balancing strategy for testing zoning features.
 * You should use this Grid LBS instead of the UGridBasedLBStrategy because it allows you to
 * do runtime delegations of Actors to specific Server Workers.
 */
UCLASS()
class SPATIALGDKFUNCTIONALTESTS_API USpatialFunctionalTestGridLBStrategy : public UGridBasedLBStrategy, public ISpatialFunctionalTestLBDelegationInterface
{
	GENERATED_BODY()

public:
	USpatialFunctionalTestGridLBStrategy();

	virtual bool ShouldHaveAuthority(const AActor& Actor) const override;
	virtual VirtualWorkerId WhoShouldHaveAuthority(const AActor& Actor) const override;
};
