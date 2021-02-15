// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "LoadBalancing/SpatialMultiWorkerSettings.h"
#include "SpatialTest1x2GridSmallInterestWorkerSettings.generated.h"

/**
 * Uses the SpatialTest1x2GridSmallInterestStrategy, otherwise has default settings.
 */
UCLASS()
class SPATIALGDKFUNCTIONALTESTS_API USpatialTest1x2GridSmallInterestWorkerSettings : public USpatialMultiWorkerSettings
{
	GENERATED_BODY()

public:
	USpatialTest1x2GridSmallInterestWorkerSettings();
};
