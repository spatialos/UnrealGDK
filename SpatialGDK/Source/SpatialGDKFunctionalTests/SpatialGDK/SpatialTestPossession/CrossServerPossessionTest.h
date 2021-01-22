// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "SpatialTestRemotePossession.h"
#include "CrossServerPossessionTest.generated.h"

UCLASS()
class SPATIALGDKFUNCTIONALTESTS_API ACrossServerPossessionTest : public ASpatialTestRemotePossession
{
	GENERATED_BODY()

public:
	ACrossServerPossessionTest();

	virtual void PrepareTest() override;
	virtual void CreateControllerAndPawn() override;
};
