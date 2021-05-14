// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "Http.h"
#include "SpatialFunctionalTest.h"
#include "SpatialTestPlayerDisconnectTrigger.generated.h"

UCLASS()
class SPATIALGDKFUNCTIONALTESTS_API ASpatialTestPlayerDisconnectTrigger : public ASpatialFunctionalTest
{
	GENERATED_BODY()

	ASpatialTestPlayerDisconnectTrigger();

	virtual void PrepareTest() override;

	FHttpRequestPtr LocalShutdownRequest;
};
