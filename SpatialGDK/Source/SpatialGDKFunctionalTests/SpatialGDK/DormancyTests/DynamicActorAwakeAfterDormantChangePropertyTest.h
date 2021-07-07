// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "DormancyTest.h"
#include "DynamicActorAwakeAfterDormantChangePropertyTest.generated.h"

// TODO: Failing due to UNR-5790. Add to test gyms when ticket is solved.

UCLASS()
class SPATIALGDKFUNCTIONALTESTS_API ADynamicActorAwakeAfterDormantChangePropertyTest : public ADormancyTest
{
	GENERATED_BODY()

public:
	ADynamicActorAwakeAfterDormantChangePropertyTest();

	virtual void PrepareTest() override;

	FTimerHandle DelayTimerHandle;
};
