// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "DormancyTest.h"
#include "InitiallyDormantMapActorChangePropertyAndFlushTest.generated.h"

UCLASS()
class SPATIALGDKFUNCTIONALTESTS_API AInitiallyDormantMapActorChangePropertyAndFlushTest : public ADormancyTest
{
	GENERATED_BODY()

public:
	AInitiallyDormantMapActorChangePropertyAndFlushTest();

	virtual void PrepareTest() override;
};
