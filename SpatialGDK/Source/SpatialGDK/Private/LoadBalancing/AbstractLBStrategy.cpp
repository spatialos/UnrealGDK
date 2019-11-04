// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "LoadBalancing/AbstractLBStrategy.h"

#include "EngineClasses/SpatialNetDriver.h"

UAbstractLBStrategy::UAbstractLBStrategy()
	: Super()
	, LocalVirtualWorkerId(0)
{
}

void UAbstractLBStrategy::SetLocalVirtualWorkerId(uint32 InLocalVirtualWorkerId)
{
	LocalVirtualWorkerId = InLocalVirtualWorkerId;
}

void UAbstractLBStrategy::Init(const USpatialNetDriver* InNetDriver)
{
}
