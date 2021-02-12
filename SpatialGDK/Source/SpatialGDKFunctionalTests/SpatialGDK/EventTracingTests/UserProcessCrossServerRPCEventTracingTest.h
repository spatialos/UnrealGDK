// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "EventTracingCrossServerTest.h"

#include "UserProcessCrossServerRPCEventTracingTest.generated.h"

UCLASS()
class SPATIALGDKFUNCTIONALTESTS_API AUserProcessCrossServerRPCEventTracingTest : public AEventTracingCrossServerTest
{
	GENERATED_BODY()

public:
	AUserProcessCrossServerRPCEventTracingTest();

private:
	virtual void FinishEventTraceTest() override;
};
