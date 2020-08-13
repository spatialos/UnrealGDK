// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "SpatialFunctionalTestFlowController.h"
#include "SpatialAuthorityTestFlowController.generated.h"

UCLASS()
class SPATIALGDKFUNCTIONALTESTS_API ASpatialAuthorityTestFlowController : public ASpatialFunctionalTestFlowController
{
	GENERATED_BODY()
	
public:	
	UFUNCTION(Server, Reliable)
	void ServerSetGameStateAuthority(int Authority);
};
