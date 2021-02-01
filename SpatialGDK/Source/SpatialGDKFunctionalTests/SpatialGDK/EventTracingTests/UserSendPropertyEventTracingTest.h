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
	virtual void FinishEventTraceTest() override;
};
