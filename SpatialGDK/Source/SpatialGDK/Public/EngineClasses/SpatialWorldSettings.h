// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "EngineClasses/SpatialMultiWorkerSettings.h"
#include "Utils/LayerInfo.h"

#include "Containers/Map.h"
#include "GameFramework/WorldSettings.h"
#include "SpatialGDKSettings.h"
#include "Utils/LayerInfo.h"

#include "Templates/SubclassOf.h"

#include "SpatialWorldSettings.generated.h"

class UAbstractLBStrategy;
class UAbstractLockingPolicy;

UCLASS()
class SPATIALGDK_API ASpatialWorldSettings : public AWorldSettings
{
	GENERATED_BODY()

public:
	ASpatialWorldSettings()
	{
		if (*MultiWorkerSettingsClass != nullptr)
		{
			MultiWorkerSettings = NewObject<USpatialMultiWorkerSettings>(this, MultiWorkerSettingsClass);
		}
	};

	UPROPERTY(EditAnywhere, Category = "Multi-Worker")
	TSubclassOf<USpatialMultiWorkerSettings> MultiWorkerSettingsClass;

	bool IsMultiWorkerEnabled() const
	{
		return *MultiWorkerSettingsClass != nullptr && MultiWorkerSettingsClass->GetDefaultObject<USpatialMultiWorkerSettings>()->bEnableMultiWorker;
	}

	const TSubclassOf<UAbstractLBStrategy>& GetDefaultLoadBalancingStrategyClass() const
	{
		check(IsMultiWorkerEnabled());
		return MultiWorkerSettings->DefaultLayerLoadBalanceStrategy;
	}

	const TSubclassOf<UAbstractLockingPolicy>& GetDefaultLockingPolicyClass() const
	{
		check(IsMultiWorkerEnabled());
		return MultiWorkerSettings->DefaultLayerLockingPolicy;
	}

	TMap<FName, FLayerInfo>& GetWorkerLayers()
	{
		check(IsMultiWorkerEnabled());
		return MultiWorkerSettings->WorkerLayers;
	}

	const TMap<FName, FLayerInfo>& GetWorkerLayers() const
	{
		check(IsMultiWorkerEnabled());
		return MultiWorkerSettings->WorkerLayers;
	}

	// USED FOR TEST ONLY
	void SetLoadBalancingStrategyClass(TSubclassOf<UAbstractLBStrategy> LoadBalancingStrategyClass) const
	{
		check(IsMultiWorkerEnabled());
		MultiWorkerSettings->DefaultLayerLoadBalanceStrategy = LoadBalancingStrategyClass;
	}

	// USED FOR TEST ONLY
	void SetEnableMultiWorker(bool bEnableMultiWorker) const
	{
		MultiWorkerSettings->bEnableMultiWorker = bEnableMultiWorker;
	}

protected:
	UPROPERTY()
	USpatialMultiWorkerSettings* MultiWorkerSettings;
};
