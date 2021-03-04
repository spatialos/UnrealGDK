// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "SpatialEventTracingTest.h"

#include "UserProcessRPCEventTracingTest.generated.h"

UCLASS()
class SPATIALGDKFUNCTIONALTESTS_API AUserProcessRPCEventTracingTest : public ASpatialEventTracingTest
{
	GENERATED_BODY()

public:
	AUserProcessRPCEventTracingTest();

private:
	virtual void FinishEventTraceTest() override;
};
