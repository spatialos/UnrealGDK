// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "LoadBalancing/AbstractLBStrategy.h"

#include "EngineClasses/SpatialNetDriver.h"
#include "EngineClasses/SpatialVirtualWorkerTranslator.h"

UAbstractLBStrategy::UAbstractLBStrategy()
	: Super()
{
}

void UAbstractLBStrategy::Init(const USpatialNetDriver* InNetDriver, const SpatialVirtualWorkerTranslator* InVirtualWorkerTranslator)
{
	VirtualWorkerTranslator = InVirtualWorkerTranslator;
}

VirtualWorkerId UAbstractLBStrategy::GetLocalVirtualWorkerId() const
{
	return VirtualWorkerTranslator->GetLocalVirtualWorkerId();
}
