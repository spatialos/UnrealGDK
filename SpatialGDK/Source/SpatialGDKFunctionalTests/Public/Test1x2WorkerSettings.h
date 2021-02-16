// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "LoadBalancing/SpatialMultiWorkerSettings.h"
#include "Test1x2WorkerSettings.generated.h"

/**
 * Uses the Test1x2GridStrategy, otherwise has default settings.
 */
UCLASS()
class SPATIALGDKFUNCTIONALTESTS_API UTest1x2WorkerSettings : public USpatialMultiWorkerSettings
{
	GENERATED_BODY()

public:
	UTest1x2WorkerSettings();
};
