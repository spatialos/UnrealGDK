// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "SpatialFunctionalTest.h"
#include "UnresolvedReferenceTest.generated.h"

UCLASS()
class SPATIALGDKFUNCTIONALTESTS_API AUnresolvedReferenceTest : public ASpatialFunctionalTest
{
	GENERATED_BODY()

	// 3 actors are enough to check if the references are resolved
	const int NumActors = 3;

public:
	AUnresolvedReferenceTest();

	void PrepareTest() final;
};
