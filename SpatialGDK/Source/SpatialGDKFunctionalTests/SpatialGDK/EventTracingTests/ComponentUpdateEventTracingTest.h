// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "EventTracingCrossServerTest.h"

#include "ComponentUpdateEventTracingTest.generated.h"

UCLASS()
class SPATIALGDKFUNCTIONALTESTS_API AComponentUpdateEventTracingTest : public AEventTracingCrossServerTest
{
	GENERATED_BODY()

public:
	AComponentUpdateEventTracingTest();

private:
	virtual void FinishEventTraceTest() override;
};
