// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "SpatialFunctionalTest.h"
#include "SpatialTestInitialOnlyForInterestActor.generated.h"

UCLASS()
class SPATIALGDKFUNCTIONALTESTS_API ASpatialTestInitialOnlyForInterestActor : public ASpatialFunctionalTest
{
	GENERATED_BODY()

public:
	ASpatialTestInitialOnlyForInterestActor();

	virtual void FinishTest(EFunctionalTestResult TestResult, const FString& Message) override;

	virtual void PrepareTest() override;

	TPair<AController*, APawn*> OriginalPawn;

	float PreviousMaximumDistanceThreshold;
};
