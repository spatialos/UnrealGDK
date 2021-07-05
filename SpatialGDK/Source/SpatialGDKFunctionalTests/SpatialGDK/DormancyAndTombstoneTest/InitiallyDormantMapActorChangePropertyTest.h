// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "DormancyTest.h"
#include "InitiallyDormantMapActorChangePropertyTest.generated.h"

UCLASS()
class SPATIALGDKFUNCTIONALTESTS_API AInitiallyDormantMapActorChangePropertyTest : public ADormancyTest
{
	GENERATED_BODY()

public:
	AInitiallyDormantMapActorChangePropertyTest();

	virtual void PrepareTest() override;
};
