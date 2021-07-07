// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "DormancyTest.h"
#include "InitiallyDormantMapActorTest.generated.h"

UCLASS()
class SPATIALGDKFUNCTIONALTESTS_API AInitiallyDormantMapActorTest : public ADormancyTest
{
	GENERATED_BODY()

public:
	AInitiallyDormantMapActorTest();

	virtual void PrepareTest() override;
};
