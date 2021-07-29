// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "SpatialGDKSettings.h"
#include "ZeroSamplingProbabilityFilterTrueSettings.generated.h"

UCLASS()
class UZeroSamplingProbabilityFilterTrueSettings : public UEventTracingSamplingSettings
{
	GENERATED_BODY()

public:
	UZeroSamplingProbabilityFilterTrueSettings()
	{
		SamplingProbability = 0.0;
		GDKEventPreFilter = "true";
		GDKEventPostFilter = "true";
		RuntimeEventPreFilter = "true";
		RuntimeEventPostFilter = "true";
	}
};
