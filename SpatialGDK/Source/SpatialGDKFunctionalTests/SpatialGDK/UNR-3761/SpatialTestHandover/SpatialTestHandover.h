// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "SpatialFunctionalTest.h"
#include "SpatialTestHandover.generated.h"

class AHandoverCube;
class ULayeredLBStrategy;

UCLASS()
class SPATIALGDKFUNCTIONALTESTS_API ASpatialTestHandover : public ASpatialFunctionalTest
{
	GENERATED_BODY()

public:
	ASpatialTestHandover();

	virtual void PrepareTest() override;

private:
	AHandoverCube* HandoverCube;

	// The location where the HandoverCube will be spawned at.
	FVector SpawnLocation;

	// Index used to store what Location the HandoverCube should be moved to at each step in the test.
	int LocationIndex;

	// Array holding the Locations where the HandoverCube will be moved throughout the test.
	TArray<FVector> TestLocations;

	// The Load Balancing used by the test, needed to decide what Server should have authority over the HandoverCube.
	ULayeredLBStrategy* LoadBalancingStrategy;

	// Int holding the number of authority changes the HandoverCube had throughout the test.
	int AuthorityChanges;
};
