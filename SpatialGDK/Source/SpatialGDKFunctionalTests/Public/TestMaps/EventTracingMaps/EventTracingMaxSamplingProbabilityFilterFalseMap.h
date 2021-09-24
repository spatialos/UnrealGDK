// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "TestMaps/GeneratedTestMap.h"
#include "EventTracingMaxSamplingProbabilityFilterFalseMap.generated.h"

UCLASS()
class SPATIALGDKFUNCTIONALTESTS_API UEventTracingMaxSamplingProbabilityFilterFalseMap : public UGeneratedTestMap
{
	GENERATED_BODY()

public:
	UEventTracingMaxSamplingProbabilityFilterFalseMap();

protected:
	virtual void CreateCustomContentForMap() override;
};
