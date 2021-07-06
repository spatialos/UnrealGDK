// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "DormancyTest.h"
#include "DynamicActorAwakeChangePropertyTest.generated.h"

UCLASS()
class SPATIALGDKFUNCTIONALTESTS_API ADynamicActorAwakeChangePropertyTest : public ADormancyTest
{
	GENERATED_BODY()

public:
	ADynamicActorAwakeChangePropertyTest();

	virtual void PrepareTest() override;
};
