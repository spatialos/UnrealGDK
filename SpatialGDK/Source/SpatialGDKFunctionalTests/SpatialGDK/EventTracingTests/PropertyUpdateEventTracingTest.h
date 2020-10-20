// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "EventTracingTest.h"

#include "PropertyUpdateEventTracingTest.generated.h"

UCLASS()
class SPATIALGDKFUNCTIONALTESTS_API APropertyUpdateEventTracingTest : public AEventTracingTest
{
	GENERATED_BODY()

public:
	APropertyUpdateEventTracingTest();

private:
	FName PropertyUpdateEventName = "unreal_gdk.property_update";

	virtual void FinishEventTraceTest() override;
};
