// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "SpatialFunctionalTest.h"
#include "SpatialTestRemotePossession.h"
#include "CrossServerPossessionLockTest.generated.h"

class ATestPossessionPawn;

UCLASS()
class SPATIALGDKFUNCTIONALTESTS_API ACrossServerPossessionLockTest : public ASpatialTestRemotePossession
{
	GENERATED_BODY()
public:
	ACrossServerPossessionLockTest();

	virtual void PrepareTest() override;
};
