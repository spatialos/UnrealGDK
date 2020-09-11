// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "EventTracingTest.h"

#include "CommandRequestEventTracingTest.generated.h"

UCLASS()
class SPATIALGDKFUNCTIONALTESTS_API ACommandRequestEventTracingTest : public AEventTracingTest
{
	GENERATED_BODY()

public:
	ACommandRequestEventTracingTest();

private:

	FName CommandRequestEventName = "unreal_gdk.command_request";

	virtual void FinishEventTraceTest() override;
};
