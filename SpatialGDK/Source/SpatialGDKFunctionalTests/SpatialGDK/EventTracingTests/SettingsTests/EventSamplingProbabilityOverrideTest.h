// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "SpatialGDKFunctionalTests/SpatialGDK/EventTracingTests/EventTracingTest.h"

#include "EventSamplingProbabilityOverrideTest.generated.h"

UCLASS()
class SPATIALGDKFUNCTIONALTESTS_API AEventSamplingProbabilityOverrideTest : public AEventTracingTest
{
	GENERATED_BODY()

public:
	AEventSamplingProbabilityOverrideTest();

private:
	virtual void FinishEventTraceTest() override;
};
