// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "SpatialGDKFunctionalTests/SpatialGDK/EventTracingTests/EventTracingTest.h"

#include "CustomGDKFilterTest.generated.h"

UCLASS()
class SPATIALGDKFUNCTIONALTESTS_API ACustomGDKFilterTest : public AEventTracingTest
{
	GENERATED_BODY()

public:
	ACustomGDKFilterTest();

private:
	virtual void FinishEventTraceTest() override;
};
