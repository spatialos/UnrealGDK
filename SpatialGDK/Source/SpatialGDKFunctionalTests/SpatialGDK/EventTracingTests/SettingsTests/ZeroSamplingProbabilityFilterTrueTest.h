// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "SpatialGDKFunctionalTests/SpatialGDK/EventTracingTests/EventTracingTest.h"

#include "ZeroSamplingProbabilityFilterTrueTest.generated.h"

UCLASS()
class SPATIALGDKFUNCTIONALTESTS_API AZeroSamplingProbabilityFilterTrueTest : public AEventTracingTest
{
	GENERATED_BODY()

public:
	AZeroSamplingProbabilityFilterTrueTest();

private:
	virtual void FinishEventTraceTest() override;
};
