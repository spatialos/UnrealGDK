// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "EventTracingTest.h"

#include "UserProcessCrossServerRPCEventTracingTest.generated.h"

UCLASS()
class SPATIALGDKFUNCTIONALTESTS_API AUserProcessCrossServerRPCEventTracingTest : public AEventTracingTest
{
	GENERATED_BODY()

public:
	AUserProcessCrossServerRPCEventTracingTest();

private:
	virtual void FinishEventTraceTest() override;
};
