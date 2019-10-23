// Copyright (c) Improbable Worlds Ltd, All Rights Reserved


#include "AbstractLBStrategy.h"
#include "SpatialNetDriver.h"
#include "Interop/Connection/SpatialWorkerConnection.h"

UAbstractLBStrategy::UAbstractLBStrategy()
	: Super()
	, LocalVirtualWorkerId(0)
{
}

void UAbstractLBStrategy::SetLocalVirtualWorkerId(uint32 InLocalVirtualWorkerId)
{
	LocalVirtualWorkerId = InLocalVirtualWorkerId;
}

void UAbstractLBStrategy::Init(const class USpatialNetDriver* InNetDriver)
{
	NetDriver = InNetDriver;
}
