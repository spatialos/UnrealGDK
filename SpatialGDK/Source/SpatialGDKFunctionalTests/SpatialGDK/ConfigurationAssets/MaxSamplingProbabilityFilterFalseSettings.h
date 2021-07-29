// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "SpatialGDKSettings.h"
#include "MaxSamplingProbabilityFilterFalseSettings.generated.h"

UCLASS()
class UMaxSamplingProbabilityFilterFalseSettings : public UEventTracingSamplingSettings
{
	GENERATED_BODY()

public:
	UMaxSamplingProbabilityFilterFalseSettings()
	{
		SamplingProbability = 1.0;
		GDKEventPreFilter = "false";
		GDKEventPostFilter = "false";
		RuntimeEventPreFilter = "false";
		RuntimeEventPostFilter = "false";
	}
};
