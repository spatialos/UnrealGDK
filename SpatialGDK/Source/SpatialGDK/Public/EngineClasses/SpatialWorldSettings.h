// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/WorldSettings.h"
#include "LoadBalancing/LayeredLBStrategy.h"

#include "SpatialWorldSettings.generated.h"

class UAbstractLBStrategy;
class UAbstractLockingPolicy;

UCLASS()
class SPATIALGDK_API ASpatialWorldSettings : public AWorldSettings
{
	GENERATED_BODY()

public:
	/** Default layer LB Strategy */
	UPROPERTY(EditAnywhere, Config, Category = "Load Balancing")
	TSubclassOf<UAbstractLBStrategy> DefaultLoadBalanceStrategy;

	/** Default layer locaking policy. */
	UPROPERTY(EditAnywhere, Config, Category = "Load Balancing")
	TSubclassOf<UAbstractLockingPolicy> DefaultLockingPolicy;

	/** Layer loadbalancing  configuration. */
	UPROPERTY(EditAnywhere, Config, Category = "Multi Worker")
	TMap<FName, FLBLayerInfo> WorkerLBLayers;
};
