// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "SpatialFunctionalTest.h"
#include "SpatialTestWorldComposition.generated.h"

UCLASS()
class SPATIALGDKFUNCTIONALTESTS_API ASpatialTestWorldComposition : public ASpatialFunctionalTest
{
	GENERATED_BODY()

public:
	ASpatialTestWorldComposition();

	virtual void BeginPlay() override;
};
