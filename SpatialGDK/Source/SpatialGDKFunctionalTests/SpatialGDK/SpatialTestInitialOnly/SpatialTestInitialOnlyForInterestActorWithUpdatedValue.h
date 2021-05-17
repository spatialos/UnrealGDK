// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "SpatialFunctionalTest.h"
#include "SpatialTestInitialOnlyForInterestActorWithUpdatedValue.generated.h"

UCLASS()
class SPATIALGDKFUNCTIONALTESTS_API ASpatialTestInitialOnlyForInterestActorWithUpdatedValue : public ASpatialFunctionalTest
{
	GENERATED_BODY()

public:
	ASpatialTestInitialOnlyForInterestActorWithUpdatedValue();

	virtual void PrepareTest() override;

	TPair<AController*, APawn*> OriginalPawn;
};
