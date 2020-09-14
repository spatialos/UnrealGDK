// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "Http.h"
#include "SpatialFunctionalTest.h"
#include "TestPrepareShutdownListener.h"
#include "SpatialTestShutdownPreparationTrigger.generated.h"

UCLASS()
class SPATIALGDKFUNCTIONALTESTS_API ASpatialTestShutdownPreparationTrigger : public ASpatialFunctionalTest
{
	GENERATED_BODY()

	ASpatialTestShutdownPreparationTrigger();

	virtual void BeginPlay() override;

	UPROPERTY(EditInstanceOnly, Category = "Test Settings")
	TSubclassOf<ATestPrepareShutdownListener> PrepareShutdownListenerClass;

	UPROPERTY(EditInstanceOnly, Category = "Test Settings")
	float EventWaitTime;

	float StepTimer;
	ATestPrepareShutdownListener* LocalListener;
	FHttpRequestPtr LocalShutdownRequest;
};
