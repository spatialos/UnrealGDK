// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "SpatialFunctionalTest.h"
#include "SpatialTestRPCTimeout.generated.h"

class ATestMovementCharacter;

UCLASS()
class ASpatialTestRPCTimeout : public ASpatialFunctionalTest
{
	GENERATED_BODY()

public:
	ASpatialTestRPCTimeout();

	virtual void PrepareTest() override;

	virtual void CreateCustomContentForMap() override;

private:
	float Step1Timer = 0.f;
};
