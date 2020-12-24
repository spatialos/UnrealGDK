// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "CuesGASTestActor.h"
#include "SpatialFunctionalTest.h"
#include "PredictedGameplayCuesTest.generated.h"

/**
 * TODO
 */
UCLASS()
class SPATIALGDKFUNCTIONALTESTS_API APredictedGameplayCuesTest : public ASpatialFunctionalTest
{
	GENERATED_BODY()

	APredictedGameplayCuesTest();

	virtual void PrepareTest() override;

	UPROPERTY(EditInstanceOnly, Category = "Test Settings")
	float DuplicateActivationCheckWaitTime;

private:
	ACuesGASTestActor* TargetActor;
	float StepTimer;

	// Returns whether the caller should finish the step
	// bool WaitForActivationConfirmation(float DeltaTime);
};
