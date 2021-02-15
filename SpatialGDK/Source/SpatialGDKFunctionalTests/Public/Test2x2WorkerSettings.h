// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "LoadBalancing/SpatialMultiWorkerSettings.h"
#include "Test2x2WorkerSettings.generated.h"

/**
 * Uses the Test2x2GridStrategy, otherwise has default settings.
 */
UCLASS()
class SPATIALGDKFUNCTIONALTESTS_API UTest2x2WorkerSettings : public USpatialMultiWorkerSettings
{
	GENERATED_BODY()

public:
	UTest2x2WorkerSettings();
};
