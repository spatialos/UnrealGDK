// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "SpatialTestRemotePossession.h"
#include "CrossServerMultiPossessionTest.generated.h"

UCLASS()
class SPATIALGDKFUNCTIONALTESTS_API ACrossServerMultiPossessionTest : public ASpatialTestRemotePossession
{
	GENERATED_BODY()

public:
	ACrossServerMultiPossessionTest();

	virtual void PrepareTest() override;
};
