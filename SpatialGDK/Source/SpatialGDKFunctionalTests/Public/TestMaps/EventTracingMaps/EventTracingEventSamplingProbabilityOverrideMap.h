// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "TestMaps/GeneratedTestMap.h"
#include "EventTracingEventSamplingProbabilityOverrideMap.generated.h"

UCLASS()
class SPATIALGDKFUNCTIONALTESTS_API UEventTracingEventSamplingProbabilityOverrideMap : public UGeneratedTestMap
{
	GENERATED_BODY()

public:
	UEventTracingEventSamplingProbabilityOverrideMap();

protected:
	virtual void CreateCustomContentForMap() override;
};
