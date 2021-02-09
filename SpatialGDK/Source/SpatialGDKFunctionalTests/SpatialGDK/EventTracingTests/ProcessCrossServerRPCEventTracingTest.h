// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "EventTracingTest.h"

#include "ProcessCrossServerRPCEventTracingTest.generated.h"

UCLASS()
class SPATIALGDKFUNCTIONALTESTS_API AProcessCrossServerRPCEventTracingTest : public AEventTracingTest
{
	GENERATED_BODY()

public:
	AProcessCrossServerRPCEventTracingTest();

private:
	virtual void FinishEventTraceTest() override;
};
