// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "DormancyTest.h"
#include "DynamicActorDormantAllChangePropertyTest.generated.h"

UCLASS()
class SPATIALGDKFUNCTIONALTESTS_API ADynamicActorDormantAllChangePropertyTest : public ADormancyTest
{
	GENERATED_BODY()

public:
	ADynamicActorDormantAllChangePropertyTest();

	virtual void PrepareTest() override;

private:

	AActor* TestActor;
};
