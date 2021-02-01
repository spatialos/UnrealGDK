// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "EventTracingTest.h"

#include "UserSendRPCEventTracingTest.generated.h"

UCLASS()
class SPATIALGDKFUNCTIONALTESTS_API AUserSendRPCEventTracingTest : public AEventTracingTest
{
	GENERATED_BODY()

public:
	AUserSendRPCEventTracingTest();

private:
	virtual void FinishEventTraceTest() override;
};
