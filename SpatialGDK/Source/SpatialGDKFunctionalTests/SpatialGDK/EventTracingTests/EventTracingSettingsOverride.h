// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "EventTracingTest.h"
#include "EventTracingSettingsOverride.generated.h"

UCLASS()
class SPATIALGDKFUNCTIONALTESTS_API AEventTracingSettingsOverride : public AEventTracingTest
{
	GENERATED_BODY()

public:
	AEventTracingSettingsOverride();

	virtual void PrepareTest() override;

private:
	virtual void FinishEventTraceTest() override;
};
