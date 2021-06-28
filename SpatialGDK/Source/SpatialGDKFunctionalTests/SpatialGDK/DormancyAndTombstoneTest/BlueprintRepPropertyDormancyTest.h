// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "SpatialFunctionalTest.h"
#include "BlueprintRepPropertyDormancyTest.generated.h"

UCLASS()
class SPATIALGDKFUNCTIONALTESTS_API ABlueprintRepPropertyDormancyTest : public ASpatialFunctionalTest
{
	GENERATED_BODY()

public:
	ABlueprintRepPropertyDormancyTest();

	virtual void PrepareTest() override;
};
