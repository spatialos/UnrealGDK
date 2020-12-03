// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "SpatialFunctionalTest.h"
#include "ZCleanUpFunctionalTests.generated.h"

class AReplicatedTestActor;

UCLASS()
class SPATIALGDKFUNCTIONALTESTS_API AZCleanUpFunctionalTests : public ASpatialFunctionalTest
{
	GENERATED_BODY()
public:
	AZCleanUpFunctionalTests();

	virtual void PrepareTest() override;
};
