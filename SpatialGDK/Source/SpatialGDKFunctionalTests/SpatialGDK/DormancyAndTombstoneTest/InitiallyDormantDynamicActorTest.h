// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "DormancyTest.h"
#include "InitiallyDormantDynamicActorTest.generated.h"

UCLASS()
class SPATIALGDKFUNCTIONALTESTS_API AInitiallyDormantDynamicActorTest : public ADormancyTest
{
	GENERATED_BODY()

public:
	AInitiallyDormantDynamicActorTest();

	virtual void PrepareTest() override;
};
