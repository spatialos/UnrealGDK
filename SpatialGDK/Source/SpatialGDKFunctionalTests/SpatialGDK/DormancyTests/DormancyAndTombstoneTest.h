// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "DormancyTest.h"
#include "DormancyAndTombstoneTest.generated.h"

UCLASS()
class SPATIALGDKFUNCTIONALTESTS_API ADormancyAndTombstoneTest : public ADormancyTest
{
	GENERATED_BODY()

public:
	ADormancyAndTombstoneTest();

	virtual void PrepareTest() override;
};
