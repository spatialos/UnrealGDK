// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "EventTracingTest.h"

#include "UserReceivePropertyEventTracingTest.generated.h"

UCLASS()
class SPATIALGDKFUNCTIONALTESTS_API AUserReceivePropertyEventTracingTest : public AEventTracingTest
{
	GENERATED_BODY()

public:
	AUserReceivePropertyEventTracingTest();

private:
	FName UserReceivePropertyEventName = "user.receive_property";
	FName UserReceiveComponentPropertyEventName = "user.receive_component_property";

	virtual void FinishEventTraceTest() override;
};
