// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "Http.h"
#include "SpatialFunctionalTest.h"
#include "SpatialTestPlayerDisconnectReturnToMainMenu.generated.h"

UCLASS()
class SPATIALGDKFUNCTIONALTESTS_API ASpatialTestPlayerDisconnectReturnToMainMenu : public ASpatialFunctionalTest
{
	GENERATED_BODY()

	ASpatialTestPlayerDisconnectReturnToMainMenu();

	virtual void PrepareTest() override;

	UPROPERTY(EditInstanceOnly, Category = "Test Settings")
	float TriggerEventWaitTime;

	float StepTimer;
	FHttpRequestPtr LocalShutdownRequest;
};
