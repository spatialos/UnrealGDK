// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "SpatialFunctionalTest.h"
#include "RegisterAutoDestroyActorsTest.generated.h"

/**
 * These Tests are meant to test the functionality of RegisterAutoDestroyActor in Test environments.
 * Keep in mind that for it to run correctly you need to run both part 1 and 2, and in that order,
 * since the auto destruction happens at the end of the test, so you need the next test to check
 * that it is working. This test should work both with and without load-balancing, as long as
 * the servers have global interest area (limitation at this time).
 */
UCLASS()
class SPATIALGDKFUNCTIONALTESTS_API ARegisterAutoDestroyActorsTestPart1 : public ASpatialFunctionalTest
{
	GENERATED_BODY()

	ARegisterAutoDestroyActorsTestPart1();

	virtual void BeginPlay() override;
};

UCLASS()
class SPATIALGDKFUNCTIONALTESTS_API ARegisterAutoDestroyActorsTestPart2 : public ASpatialFunctionalTest
{
	GENERATED_BODY()

	ARegisterAutoDestroyActorsTestPart2();

	virtual void BeginPlay() override;
};
