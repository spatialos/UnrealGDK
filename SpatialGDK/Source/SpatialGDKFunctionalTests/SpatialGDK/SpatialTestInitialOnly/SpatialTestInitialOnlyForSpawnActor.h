// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "SpatialFunctionalTest.h"
#include "SpatialTestInitialOnlyForSpawnActor.generated.h"

UCLASS()
class SPATIALGDKFUNCTIONALTESTS_API ASpatialTestInitialOnlyForSpawnActor : public ASpatialFunctionalTest
{
	GENERATED_BODY()

public:
	ASpatialTestInitialOnlyForSpawnActor();

	virtual void FinishTest(EFunctionalTestResult TestResult, const FString& Message) override;

	virtual void PrepareTest() override;

	TPair<AController*, APawn*> OriginalPawn;

	float PreviousMaximumDistanceThreshold;
};
