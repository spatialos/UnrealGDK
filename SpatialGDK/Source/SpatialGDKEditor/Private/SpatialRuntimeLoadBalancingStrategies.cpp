// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "SpatialRuntimeLoadBalancingStrategies.h"

USingleWorkerRuntimeStrategy::USingleWorkerRuntimeStrategy() = default;

int32 USingleWorkerRuntimeStrategy::GetNumberOfWorkersForPIE() const
{
	return 1;
}

UGridRuntimeLoadBalancingStrategy::UGridRuntimeLoadBalancingStrategy()
	: Columns(1)
	, Rows(1)
{

}

int32 UGridRuntimeLoadBalancingStrategy::GetNumberOfWorkersForPIE() const
{
	return Rows * Columns;
}

UEntityShardingRuntimeLoadBalancingStrategy::UEntityShardingRuntimeLoadBalancingStrategy()
	: NumWorkers(1)
{

}

int32 UEntityShardingRuntimeLoadBalancingStrategy::GetNumberOfWorkersForPIE() const
{
	return NumWorkers;
}
