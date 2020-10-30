// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "EventTracingTest.h"

#include "ComponentUpdateEventTracingTest.generated.h"

UCLASS()
class SPATIALGDKFUNCTIONALTESTS_API AComponentUpdateEventTracingTest : public AEventTracingTest
{
	GENERATED_BODY()

public:
	AComponentUpdateEventTracingTest();

private:
	virtual void FinishEventTraceTest() override;
};
