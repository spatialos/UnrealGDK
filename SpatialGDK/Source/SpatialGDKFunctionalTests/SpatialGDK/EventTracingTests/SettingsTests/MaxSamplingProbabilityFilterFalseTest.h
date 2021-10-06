// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "SpatialGDKFunctionalTests/SpatialGDK/EventTracingTests/EventTracingTest.h"

#include "MaxSamplingProbabilityFilterFalseTest.generated.h"

UCLASS()
class SPATIALGDKFUNCTIONALTESTS_API AMaxSamplingProbabilityFilterFalseTest : public AEventTracingTest
{
	GENERATED_BODY()

public:
	AMaxSamplingProbabilityFilterFalseTest();

private:
	virtual void FinishEventTraceTest() override;
};
