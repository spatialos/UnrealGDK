// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "LoadBalancing/LayeredLBStrategy.h"

#include "CoreMinimal.h"
#include "GameFramework/WorldSettings.h"
#include "SpatialGDKSettings.h"
#include "Utils/LayerInfo.h"
#include "Utils/SpatialDebuggerEditor.h"
#include "EngineUtils.h"


#include "SpatialWorldSettings.generated.h"

class UAbstractLBStrategy;
class UAbstractLockingPolicy;

UCLASS()
class SPATIALGDK_API ASpatialWorldSettings : public AWorldSettings
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, Config, Category = "Multi-Worker", meta = (EditCondition = "bEnableMultiWorker"))
	TSubclassOf<UAbstractLBStrategy> DefaultLayerLoadBalanceStrategy;

	UPROPERTY(EditAnywhere, Config, Category = "Multi-Worker", meta = (EditCondition = "bEnableMultiWorker"))
	TSubclassOf<UAbstractLockingPolicy> DefaultLayerLockingPolicy;

	/**
	  * Any classes not specified on another layer will be handled by the default layer, but this also gives a way
	  * to force classes to be on the default layer.
	  */
	UPROPERTY(EditAnywhere, Category = "Multi-Worker", meta = (EditCondition = "bEnableMultiWorker"))
	TSet<TSoftClassPtr<AActor>> ExplicitDefaultActorClasses;

	/** Layer configuration. */
	UPROPERTY(EditAnywhere, Config, Category = "Multi-Worker", meta = (EditCondition = "bEnableMultiWorker"))
	TMap<FName, FLayerInfo> WorkerLayers;

	bool IsMultiWorkerEnabled() const
	{
		const USpatialGDKSettings* SpatialGDKSettings = GetDefault<USpatialGDKSettings>();
		if (SpatialGDKSettings->bOverrideMultiWorker.IsSet())
		{
			return SpatialGDKSettings->bOverrideMultiWorker.GetValue();
		}
		return bEnableMultiWorker;
	}

	void SetMultiWorkerEnabled(bool IsEnabled)
	{
		bEnableMultiWorker = IsEnabled;
	}

	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override
	{
		if (PropertyChangedEvent.Property != nullptr)
		{
			const FName PropertyName(PropertyChangedEvent.Property->GetFName());
			if (PropertyName == GET_MEMBER_NAME_CHECKED(ASpatialWorldSettings, DefaultLayerLoadBalanceStrategy) ||
				PropertyName == GET_MEMBER_NAME_CHECKED(ASpatialWorldSettings, bEnableMultiWorker))
			{
				// If the load balancing strategy has changed, refresh the worker boundaries in the editor
				UWorld* World = GetWorld();
				for (TActorIterator<ASpatialDebuggerEditor> It(World); It; ++It)
				{
					ASpatialDebuggerEditor* FoundActor = *It;
					FoundActor->RefreshWorkerRegions();
				}
 			}
		}
		Super::PostEditChangeProperty(PropertyChangedEvent);
	}

private:
	/** Enable running different server worker types to split the simulation. */
	UPROPERTY(EditAnywhere, Config, Category = "Multi-Worker")
	bool bEnableMultiWorker;
};
