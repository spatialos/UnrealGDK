// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "SpatialFunctionalTest.h"
#include "SpatialTestNetReferenceSettingsOverride.generated.h"

UCLASS()
class SPATIALGDKFUNCTIONALTESTS_API ASpatialTestNetReferenceSettingsOverride : public ASpatialFunctionalTest
{
	GENERATED_BODY()

public:
	ASpatialTestNetReferenceSettingsOverride();

	virtual void PrepareTest() override;
};
