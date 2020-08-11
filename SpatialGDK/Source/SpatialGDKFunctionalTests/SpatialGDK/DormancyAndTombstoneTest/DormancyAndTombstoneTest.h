// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "SpatialFunctionalTest.h"
#include "DormancyAndTombstoneTest.generated.h"

UCLASS()
class SPATIALGDKFUNCTIONALTESTS_API ADormancyAndTombstoneTest : public ASpatialFunctionalTest
{
	GENERATED_BODY()

public:
	ADormancyAndTombstoneTest();

	virtual void BeginPlay() override;
};
