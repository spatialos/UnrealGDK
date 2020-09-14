// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "EventTracingTest.h"

#include "MergeComponentFieldEventTracingTest.generated.h"

UCLASS()
class SPATIALGDKFUNCTIONALTESTS_API AMergeComponentFieldEventTracingTest : public AEventTracingTest
{
	GENERATED_BODY()

public:
	AMergeComponentFieldEventTracingTest();

private:
	virtual void FinishEventTraceTest() override;
};
