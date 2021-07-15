// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "SpatialFunctionalTest.h"
#include "SpatialTestPlayerDisconnect.generated.h"

UCLASS()
class SPATIALGDKFUNCTIONALTESTS_API ASpatialTestPlayerDisconnect : public ASpatialFunctionalTest
{
	GENERATED_BODY()

	ASpatialTestPlayerDisconnect();

	virtual void PrepareTest() override;
};
