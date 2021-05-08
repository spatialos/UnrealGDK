// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "LoadBalancing/AbstractLBStrategy.h"

#include "LoadBalancing/LoadBalancingCalculator.h"

#include "EngineClasses/SpatialNetDriver.h"

UAbstractLBStrategy::UAbstractLBStrategy()
	: Super()
	, LocalVirtualWorkerId(SpatialConstants::INVALID_VIRTUAL_WORKER_ID)
{
}

void UAbstractLBStrategy::SetLocalVirtualWorkerId(VirtualWorkerId InLocalVirtualWorkerId)
{
	LocalVirtualWorkerId = InLocalVirtualWorkerId;
}

TUniquePtr<FLoadBalancingCalculator> UAbstractLBStrategy::CreateLoadBalancingCalculator() const
{
	return {};
}
