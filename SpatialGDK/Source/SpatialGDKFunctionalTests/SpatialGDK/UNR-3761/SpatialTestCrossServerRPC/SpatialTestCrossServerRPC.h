// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "SpatialFunctionalTest.h"
#include "SpatialTestCrossServerRPC.generated.h"

UCLASS()
class SPATIALGDKFUNCTIONALTESTS_API ASpatialTestCrossServerRPC : public ASpatialFunctionalTest
{
	GENERATED_BODY()

public:
	ASpatialTestCrossServerRPC();

	virtual void BeginPlay() override;
};
