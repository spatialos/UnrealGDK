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
	UPROPERTY(EditAnywhere, Category = "Multi-Worker")
	TSubclassOf<USpatialMultiWorkerSettings> MultiWorkerSettingsClass;
	
	const TArray<FLayerInfo>& GetWorkerLayers() const
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
};
