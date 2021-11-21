// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "SpatialFunctionalTest.h"
#include "TestMaps/GeneratedTestMap.h"
#include "SpatialTestRPCTimeout.generated.h"

class ATestMovementCharacter;

UCLASS()
class ASpatialTestRPCTimeout : public ASpatialFunctionalTest
{
	GENERATED_BODY()

public:
	ASpatialTestRPCTimeout();

	virtual void PrepareTest() override;

private:
	float Step1Timer = 0.f;
};

/**
 * This map is for use with SpatialTestRPCTimeout
 */
	UCLASS()
	class SPATIALGDKFUNCTIONALTESTS_API USpatialRPCTimeoutMap : public UGeneratedTestMap
{
	GENERATED_BODY()

public:
	USpatialRPCTimeoutMap();

protected:
	virtual void CreateCustomContentForMap() override;
};
