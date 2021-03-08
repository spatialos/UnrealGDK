// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "SpatialFunctionalTest.h"
#include "TestClasses/SpatialTestInitialOnlySpawnComponent.h"
#include "SpatialTestInitialOnlyForSpawnComponents.generated.h"

UCLASS()
class SPATIALGDKFUNCTIONALTESTS_API ASpatialTestInitialOnlyForSpawnComponents : public ASpatialFunctionalTest
{
	GENERATED_BODY()

public:
	ASpatialTestInitialOnlyForSpawnComponents();

	virtual void FinishTest(EFunctionalTestResult TestResult, const FString& Message) override;

	virtual void PrepareTest() override;

	TPair<AController*, APawn*> OriginalPawn;

	float PreviousMaximumDistanceThreshold;

	USpatialTestInitialOnlySpawnComponent* CreateAndAttachSpawnComponentToActor(AActor* Actor, FName Name);
};
