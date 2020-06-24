// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "EngineClasses/SpatialMultiWorkerSettings.h"

#include "CoreMinimal.h"
#include "GameFramework/WorldSettings.h"
#include "SpatialGDKSettings.h"
#include "Utils/LayerInfo.h"

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
		return MultiWorkerSettings != nullptr && MultiWorkerSettings->bEnableMultiWorker;
	}

	const TMap<FName, FLayerInfo>& GetWorkerLayers() const
	{
		return MultiWorkerSettings->WorkerLayers;
	}

private:
	UPROPERTY()
	USpatialMultiWorkerSettings* MultiWorkerSettings;
};
