// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "SpatialTestRemotePossession.h"
#include "TestPossessionPlayerController.h"
#include "CrossServerPossessionLockTest.generated.h"


UCLASS()
class SPATIALGDKFUNCTIONALTESTS_API ACrossServerPossessionLockTest : public ASpatialTestRemotePossession
{
	GENERATED_BODY()
	
public:
	ACrossServerPossessionLockTest();

	virtual void PrepareTest() override;
};
