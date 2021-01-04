// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "SpatialFunctionalTest.h"
#include "SpatialTestCondInitialOnly.generated.h"

class ATestCondInitialOnlyPawn;

UCLASS()
class SPATIALGDKFUNCTIONALTESTS_API ASpatialTestCondInitialOnly : public ASpatialFunctionalTest
{
	GENERATED_BODY()
public:
	ASpatialTestCondInitialOnly();

	virtual void PrepareTest() override;

private:
	ATestCondInitialOnlyPawn* GetPawn();
	void Wait();

	float WaitTime;
};
