// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "SpatialFunctionalTest.h"
#include "SpatialComponentSettingsOverride.generated.h"

UCLASS()
class SPATIALGDKFUNCTIONALTESTS_API ASpatialComponentSettingsOverride : public ASpatialFunctionalTest
{
	GENERATED_BODY()

public:
	ASpatialComponentSettingsOverride();

	virtual void PrepareTest() override;
};
