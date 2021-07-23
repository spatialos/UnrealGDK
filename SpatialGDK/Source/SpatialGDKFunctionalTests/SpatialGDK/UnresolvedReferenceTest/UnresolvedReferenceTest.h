// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "SpatialFunctionalTest.h"
#include "UnresolvedReferenceTest.generated.h"

UCLASS()
class SPATIALGDKFUNCTIONALTESTS_API AUnresolvedReferenceTest : public ASpatialFunctionalTest
{
	GENERATED_BODY()

public:
	AUnresolvedReferenceTest();

	virtual void PrepareTest() override;
};
