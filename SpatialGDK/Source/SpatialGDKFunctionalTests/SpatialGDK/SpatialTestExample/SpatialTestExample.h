
// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "SpatialFunctionalTest.h"
#include "SpatialTestExample.generated.h"

class ATestExampleActor;

UCLASS()
class SPATIALGDKFUNCTIONALTESTS_API ASpatialTestExample : public ASpatialFunctionalTest
{
	GENERATED_BODY()
public:
	ASpatialTestExample();

	virtual void PrepareTest()  override;

private:
	ATestExampleActor* TestActor;
};
