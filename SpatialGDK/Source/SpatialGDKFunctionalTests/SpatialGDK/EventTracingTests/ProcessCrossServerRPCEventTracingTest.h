// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "EventTracingCrossServerTest.h"

#include "ProcessCrossServerRPCEventTracingTest.generated.h"

UCLASS()
class SPATIALGDKFUNCTIONALTESTS_API AProcessCrossServerRPCEventTracingTest : public AEventTracingCrossServerTest
{
	GENERATED_BODY()

public:
	AProcessCrossServerRPCEventTracingTest();

private:
	virtual void FinishEventTraceTest() override;
};
