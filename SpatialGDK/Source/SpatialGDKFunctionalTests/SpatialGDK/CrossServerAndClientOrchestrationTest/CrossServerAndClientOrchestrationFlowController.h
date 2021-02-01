// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "SpatialFunctionalTestFlowController.h"
#include "CrossServerAndClientOrchestrationFlowController.generated.h"

/**
 *
 */
UCLASS()
class SPATIALGDKFUNCTIONALTESTS_API ACrossServerAndClientOrchestrationFlowController : public ASpatialFunctionalTestFlowController
{
	GENERATED_BODY()

public:
	UFUNCTION(Server, Reliable)
	void ServerClientReadValueAck();
};
