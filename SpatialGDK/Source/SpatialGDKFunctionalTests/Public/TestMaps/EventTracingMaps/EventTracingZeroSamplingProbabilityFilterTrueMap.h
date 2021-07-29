// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "TestMaps/GeneratedTestMap.h"
#include "EventTracingZeroSamplingProbabilityFilterTrueMap.generated.h"

UCLASS()
class SPATIALGDKFUNCTIONALTESTS_API UEventTracingZeroSamplingProbabilityFilterTrueMap : public UGeneratedTestMap
{
	GENERATED_BODY()

public:
	UEventTracingZeroSamplingProbabilityFilterTrueMap();

protected:
	virtual void CreateCustomContentForMap() override;
};
