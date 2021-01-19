// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "SpatialTestRemotePossession.h"
#include "NoneCrossServerPossessionTest.generated.h"

UCLASS()
class SPATIALGDKFUNCTIONALTESTS_API ANoneCrossServerPossessionTest : public ASpatialTestRemotePossession
{
	GENERATED_BODY()

public:
	ANoneCrossServerPossessionTest();

	virtual void PrepareTest() override;
};
