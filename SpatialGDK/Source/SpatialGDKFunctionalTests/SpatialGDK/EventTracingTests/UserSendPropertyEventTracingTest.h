// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "EventTracingTest.h"

#include "UserSendPropertyEventTracingTest.generated.h"

UCLASS()
class SPATIALGDKFUNCTIONALTESTS_API AUserSendPropertyEventTracingTest : public AEventTracingTest
{
	GENERATED_BODY()

public:
	AUserSendPropertyEventTracingTest();

private:
	FName UserSendPropertyEventName = "user.send_property";
	FName UserSendComponentPropertyEventName = "user.send_component_property";
	FName SendPropertyUpdatesEventName = "unreal_gdk.send_property_updates";

	virtual void FinishEventTraceTest() override;
};
