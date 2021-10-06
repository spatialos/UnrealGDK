// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "SpatialFunctionalTest.h"
#include "SpatialAuthoritySettingsOverride.generated.h"

UCLASS()
class SPATIALGDKFUNCTIONALTESTS_API ASpatialAuthoritySettingsOverride : public ASpatialFunctionalTest
{
	GENERATED_BODY()

public:
	ASpatialAuthoritySettingsOverride();

	virtual void PrepareTest() override;
};
