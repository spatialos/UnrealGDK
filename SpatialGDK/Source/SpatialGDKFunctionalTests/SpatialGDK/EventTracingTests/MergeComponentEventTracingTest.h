// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "EventTracingTest.h"

#include "MergeComponentEventTracingTest.generated.h"

UCLASS()
class SPATIALGDKFUNCTIONALTESTS_API AMergeComponentEventTracingTest : public AEventTracingTest
{
	GENERATED_BODY()

public:
	AMergeComponentEventTracingTest();

private:
	virtual void FinishEventTraceTest() override;
};
