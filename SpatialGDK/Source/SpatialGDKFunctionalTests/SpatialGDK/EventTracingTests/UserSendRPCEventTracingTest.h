// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "SpatialEventTracingTest.h"

#include "UserSendRPCEventTracingTest.generated.h"

UCLASS()
class SPATIALGDKFUNCTIONALTESTS_API AUserSendRPCEventTracingTest : public ASpatialEventTracingTest
{
	GENERATED_BODY()

public:
	AUserSendRPCEventTracingTest();

private:
	virtual void FinishEventTraceTest() override;
};
