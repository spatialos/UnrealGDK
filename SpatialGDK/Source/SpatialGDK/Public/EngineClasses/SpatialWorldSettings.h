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
	//ASpatialWorldSettings(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
	//{
	//	UE_LOG(LogTemp, Log, TEXT("ASpatialWorldSettings constructor"));
	//	if (*MultiWorkerSettingsClass != nullptr)
	//	{
	//		MultiWorkerSettings = NewObject<USpatialMultiWorkerSettings>(this, MultiWorkerSettingsClass);
	//	}
	//};

	UPROPERTY(EditAnywhere, Category = "Multi-Worker")
	TSubclassOf<USpatialMultiWorkerSettings> MultiWorkerSettingsClass;

	// const TSubclassOf<UAbstractLBStrategy>& GetDefaultLayerLoadBalancingStrategyClass() const
	// {
	// 	check(IsMultiWorkerEnabled());
	// 	return GetDefault<USpatialMultiWorkerSettings>(MultiWorkerSettingsClass)->DefaultLayerLoadBalanceStrategy;
	// }
	//
	// const TSubclassOf<UAbstractLockingPolicy>& GetDefaultLayerLockingPolicyClass() const
	// {
	// 	check(IsMultiWorkerEnabled());
	// 	return GetDefault<USpatialMultiWorkerSettings>(MultiWorkerSettingsClass)->DefaultLayerLockingPolicy;
	// }

	//TMap<FName, FLayerInfo>& GetWorkerLayers()
	//{
	//	check(IsMultiWorkerEnabled());
	//	return MultiWorkerSettings->WorkerLayers;
	//}

	const TMap<FName, FLayerInfo>& GetWorkerLayers() const
	{
		check(IsMultiWorkerEnabled());
		return GetDefault<USpatialMultiWorkerSettings>(MultiWorkerSettingsClass)->WorkerLayers;
	}

	bool IsMultiWorkerEnabled() const
	{
		if (*MultiWorkerSettingsClass == nullptr)
		{
			return false;
		}

		const USpatialGDKSettings* SpatialGDKSettings = GetDefault<USpatialGDKSettings>();
		if (SpatialGDKSettings->bOverrideMultiWorker.IsSet())
		{
			return SpatialGDKSettings->bOverrideMultiWorker.GetValue();
		}

		return true;
	}

	//// USED FOR TEST ONLY
	//void SetLoadBalancingStrategyClass(TSubclassOf<UAbstractLBStrategy> LoadBalancingStrategyClass) const
	//{
	//	check(IsMultiWorkerEnabled());
	//	MultiWorkerSettings->DefaultLayerLoadBalanceStrategy = LoadBalancingStrategyClass;
	//}

	//// USED FOR TEST ONLY
	//void SetEnableMultiWorker(bool bEnableMultiWorker) const
	//{
	//	MultiWorkerSettings->bEnableMultiWorker = bEnableMultiWorker;
	//}

//protected:
//	UPROPERTY()
//	USpatialMultiWorkerSettings* MultiWorkerSettings;
};
