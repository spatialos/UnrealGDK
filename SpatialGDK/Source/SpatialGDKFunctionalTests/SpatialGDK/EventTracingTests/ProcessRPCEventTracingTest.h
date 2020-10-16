// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "EventTracingTest.h"

#include "ProcessRPCEventTracingTest.generated.h"

UCLASS()
class SPATIALGDKFUNCTIONALTESTS_API AProcessRPCEventTracingTest : public AEventTracingTest
{
	GENERATED_BODY()

public:
	AProcessRPCEventTracingTest();

private:

	FName ProcessRPCEventName = "unreal_gdk.process_rpc";

	virtual void FinishEventTraceTest() override;
};
