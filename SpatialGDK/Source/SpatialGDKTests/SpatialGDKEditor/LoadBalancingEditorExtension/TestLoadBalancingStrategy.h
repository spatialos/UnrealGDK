// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "LoadBalancing/AbstractLBStrategy.h"

#include "TestLoadBalancingStrategy.generated.h"

class SpatialVirtualWorkerTranslator;
class UAbstractSpatialMultiWorkerSettings;

UCLASS()
class UDummyLoadBalancingStrategy : public UAbstractLBStrategy
{
	GENERATED_BODY()

public:
	UDummyLoadBalancingStrategy() = default;


	/* UAbstractLBStrategy Interface */
	void Init(const UAbstractSpatialMultiWorkerSettings* MultiWorkerSettings) override
	{
	}

	TSet<VirtualWorkerId> GetVirtualWorkerIds() const override
	{
		return TSet<VirtualWorkerId>();
	}

	bool ShouldHaveAuthority(const AActor& Actor) const override
	{
		return false;
	}

	VirtualWorkerId WhoShouldHaveAuthority(const AActor& Actor) const override
	{
		return 0;
	}

	SpatialGDK::QueryConstraint GetWorkerInterestQueryConstraint() const override
	{
		return SpatialGDK::QueryConstraint();
	}

	bool RequiresHandoverData() const override
	{
		return false;
	}

	FVector GetWorkerEntityPosition() const override
	{
		return FVector(ForceInitToZero);
	}
	/* End UAbstractLBStrategy Interface */

	uint32 NumberOfWorkers = 1;

};

UCLASS()
class UDerivedDummyLoadBalancingStrategy : public UDummyLoadBalancingStrategy
{
	GENERATED_BODY()
};
