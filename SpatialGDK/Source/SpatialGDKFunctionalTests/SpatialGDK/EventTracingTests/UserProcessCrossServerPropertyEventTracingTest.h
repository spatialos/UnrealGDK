// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "EventTracingCrossServerTest.h"

#include "UserProcessCrossServerPropertyEventTracingTest.generated.h"

UCLASS()
class SPATIALGDKFUNCTIONALTESTS_API AUserProcessCrossServerPropertyEventTracingTest : public AEventTracingCrossServerTest
{
	GENERATED_BODY()

public:
	AUserProcessCrossServerPropertyEventTracingTest();

private:
	virtual void FinishEventTraceTest() override;
};
