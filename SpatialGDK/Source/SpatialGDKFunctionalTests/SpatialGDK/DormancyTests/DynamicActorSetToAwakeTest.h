// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "DormancyTest.h"
#include "DynamicActorSetToAwakeTest.generated.h"

UCLASS()
class SPATIALGDKFUNCTIONALTESTS_API ADynamicActorSetToAwakeTest : public ADormancyTest
{
	GENERATED_BODY()

public:
	ADynamicActorSetToAwakeTest();

	virtual void PrepareTest() override;
};
