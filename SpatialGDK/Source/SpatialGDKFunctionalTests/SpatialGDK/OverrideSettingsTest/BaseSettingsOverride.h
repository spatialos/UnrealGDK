// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "SpatialFunctionalTest.h"
#include "BaseSettingsOverride.generated.h"

UCLASS()
class SPATIALGDKFUNCTIONALTESTS_API ABaseSettingsOverride : public ASpatialFunctionalTest
{
	GENERATED_BODY()

public:
	ABaseSettingsOverride();

	virtual void PrepareTest() override;
};
