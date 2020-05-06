// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "SpatialRuntimeLoadBalancingStrategies.h"

USingleWorkerRuntimeStrategy::USingleWorkerRuntimeStrategy() = default;

bool USingleWorkerRuntimeStrategy::WriteToConfiguration(TSharedRef<TJsonWriter<>> Writer) const
{
	Writer->WriteObjectStart("rectangle_grid");
	Writer->WriteValue(TEXT("cols"), 1);
	Writer->WriteValue(TEXT("rows"), 1);
	Writer->WriteObjectEnd();

	return true;
}

int32 USingleWorkerRuntimeStrategy::GetNumberOfWorkersForPIE() const
{
	return 1;
}

UGridRuntimeLoadBalancingStrategy::UGridRuntimeLoadBalancingStrategy()
	: Columns(1)
	, Rows(1)
{

}

bool UGridRuntimeLoadBalancingStrategy::WriteToConfiguration(TSharedRef<TJsonWriter<>> Writer) const
{
	Writer->WriteObjectStart("rectangle_grid");
	Writer->WriteValue(TEXT("cols"), Columns);
	Writer->WriteValue(TEXT("rows"), Rows);
	Writer->WriteObjectEnd();

	return true;
}

int32 UGridRuntimeLoadBalancingStrategy::GetNumberOfWorkersForPIE() const
{
	return Rows * Columns;
}

UEntityShardingRuntimeLoadBalancingStrategy::UEntityShardingRuntimeLoadBalancingStrategy()
	: NumWorkers(1)
{

}

bool UEntityShardingRuntimeLoadBalancingStrategy::WriteToConfiguration(TSharedRef<TJsonWriter<>> Writer) const
{
	Writer->WriteObjectStart("entity_id_sharding");
	Writer->WriteValue(TEXT("numWorkers"), NumWorkers);
	Writer->WriteObjectEnd();

	return true;
}

int32 UEntityShardingRuntimeLoadBalancingStrategy::GetNumberOfWorkersForPIE() const
{
	return NumWorkers;
}
