// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "SpatialFunctionalTest.h"

#include "CrossServerPossessionTest.generated.h"

UCLASS()
class SPATIALGDKFUNCTIONALTESTS_API ACrossServerPossessionTest : public ASpatialFunctionalTest
{
	GENERATED_BODY()

public:
	ACrossServerPossessionTest();

	virtual void PrepareTest() override;

private:
	float WaitTime;
	const static float MaxWaitTime;
};
