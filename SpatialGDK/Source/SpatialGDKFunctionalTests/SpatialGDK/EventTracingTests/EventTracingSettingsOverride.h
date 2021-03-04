// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "SpatialEventTracingTest.h"
#include "EventTracingSettingsOverride.generated.h"

UCLASS()
class SPATIALGDKFUNCTIONALTESTS_API AEventTracingSettingsOverride : public ASpatialEventTracingTest
{
	GENERATED_BODY()

public:
	AEventTracingSettingsOverride();

	virtual void PrepareTest() override;

private:
	virtual void FinishEventTraceTest() override;
};
