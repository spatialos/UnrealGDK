// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "LoadBalancing/SpatialMultiWorkerSettings.h"
#include "SpatialGDKSettings.h"
#include "Utils/LayerInfo.h"
#include "Utils/SpatialStatics.h"

#include "GameFramework/WorldSettings.h"
#include "Templates/SubclassOf.h"

#include "SpatialWorldSettings.generated.h"

UCLASS()
class SPATIALGDK_API ASpatialWorldSettings : public AWorldSettings
{
	GENERATED_BODY()

private:
	/** Enable running different server worker types to split the simulation. */
	UPROPERTY(EditAnywhere, Config, Category = "Multi-Worker")
	bool bEnableMultiWorker;

public:
	UPROPERTY(EditAnywhere, Category = "Multi-Worker", meta = (EditCondition = "bEnableMultiWorker"))
	TSubclassOf<USpatialMultiWorkerSettings> MultiWorkerSettingsClass;

	// This function is used to expose the private bool property to SpatialStatics.
	// You should call USpatialStatics::IsMultiWorkerEnabled to properly check whether multi-worker is enabled.
	bool IsMultiWorkerEnabledInWorldSettings() const { return bEnableMultiWorker && *MultiWorkerSettingsClass != nullptr; }
};
