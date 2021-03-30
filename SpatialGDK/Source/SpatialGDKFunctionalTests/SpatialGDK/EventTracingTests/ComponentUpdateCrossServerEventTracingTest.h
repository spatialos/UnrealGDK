// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "EventTracingCrossServerTest.h"

#include "ComponentUpdateCrossServerEventTracingTest.generated.h"

UCLASS()
class SPATIALGDKFUNCTIONALTESTS_API AComponentUpdateCrossServerEventTracingTest : public AEventTracingCrossServerTest
{
	GENERATED_BODY()

public:
	AComponentUpdateCrossServerEventTracingTest();

private:
	virtual void FinishEventTraceTest() override;
};
