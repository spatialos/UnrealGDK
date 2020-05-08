// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "Serialization/JsonWriter.h"
#include "UObject/Object.h"

#include "SpatialRuntimeLoadBalancingStrategies.generated.h"

UCLASS(Abstract)
class SPATIALGDKEDITOR_API UAbstractRuntimeLoadBalancingStrategy : public UObject
{
	GENERATED_BODY()

public:
	virtual bool WriteToConfiguration(TSharedRef<TJsonWriter<>> Writer) const PURE_VIRTUAL(UAbstractRuntimeLoadBalancingStrategy::WriteToConfiguration,return false;);

	virtual int32 GetNumberOfWorkersForPIE() const PURE_VIRTUAL(UAbstractRuntimeLoadBalancingStrategy::GetNumberOfWorkersForPIE, return 0;);
};

UCLASS()
class SPATIALGDKEDITOR_API USingleWorkerRuntimeStrategy : public UAbstractRuntimeLoadBalancingStrategy
{
	GENERATED_BODY()

public:
	USingleWorkerRuntimeStrategy();

	bool WriteToConfiguration(TSharedRef<TJsonWriter<>> Writer) const override;

	int32 GetNumberOfWorkersForPIE() const override;
};

UCLASS(EditInlineNew)
class SPATIALGDKEDITOR_API UGridRuntimeLoadBalancingStrategy : public UAbstractRuntimeLoadBalancingStrategy
{
	GENERATED_BODY()

public:
	UGridRuntimeLoadBalancingStrategy();

	/** Number of columns in the rectangle grid load balancing config. */
	UPROPERTY(Category = "LoadBalancing", EditAnywhere, meta = (DisplayName = "Rectangle grid column count", ClampMin = "1", UIMin = "1"))
	int32 Columns;
	
	/** Number of rows in the rectangle grid load balancing config. */
	UPROPERTY(Category = "LoadBalancing", EditAnywhere, meta = (DisplayName = "Rectangle grid row count", ClampMin = "1", UIMin = "1"))
	int32 Rows;
	
	bool WriteToConfiguration(TSharedRef<TJsonWriter<>> Writer) const override;

	int32 GetNumberOfWorkersForPIE() const override;
};

UCLASS(EditInlineNew)
class SPATIALGDKEDITOR_API UEntityShardingRuntimeLoadBalancingStrategy : public UAbstractRuntimeLoadBalancingStrategy
{
	GENERATED_BODY()

public:
	UEntityShardingRuntimeLoadBalancingStrategy();

	/** Number of columns in the rectangle grid load balancing config. */
	UPROPERTY(Category = "LoadBalancing", EditAnywhere, meta = (DisplayName = "Number of workers", ClampMin = "1", UIMin = "1"))
	int32 NumWorkers;

	bool WriteToConfiguration(TSharedRef<TJsonWriter<>> Writer) const override;

	int32 GetNumberOfWorkersForPIE() const override;
};
