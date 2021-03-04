// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "SpatialEventTracingTest.h"

#include "PropertyUpdateEventTracingTest.generated.h"

UCLASS()
class SPATIALGDKFUNCTIONALTESTS_API APropertyUpdateEventTracingTest : public ASpatialEventTracingTest
{
	GENERATED_BODY()

public:
	APropertyUpdateEventTracingTest();

private:
	virtual void FinishEventTraceTest() override;
};
