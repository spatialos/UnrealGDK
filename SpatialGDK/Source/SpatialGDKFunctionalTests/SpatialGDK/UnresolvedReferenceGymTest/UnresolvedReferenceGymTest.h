// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "SpatialFunctionalTest.h"
#include "UnresolvedReferenceGymTestActor.h"
#include "UnresolvedReferenceGymTest.generated.h"

UCLASS()
class SPATIALGDKFUNCTIONALTESTS_API AUnresolvedReferenceGymTest : public ASpatialFunctionalTest
{
	GENERATED_BODY()

public:
	AUnresolvedReferenceGymTest();

	virtual void PrepareTest() override;
};
