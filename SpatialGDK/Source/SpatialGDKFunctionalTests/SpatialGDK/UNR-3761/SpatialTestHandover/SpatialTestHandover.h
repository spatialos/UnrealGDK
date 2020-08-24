// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "SpatialFunctionalTest.h"
#include "SpatialTestHandover.generated.h"

class AHandoverCube;

UCLASS()
class SPATIALGDKFUNCTIONALTESTS_API ASpatialTestHandover : public ASpatialFunctionalTest
{
	GENERATED_BODY()

public:
	ASpatialTestHandover();

	virtual void BeginPlay() override;

private:
	AHandoverCube* HandoverCube;

	// The location where the HandoverCube will be spawned at.
	FVector SpawnLocation;

	// Index used to store what Location the HandoverCube should be moved to at each step in the test.
	int LocationIndex;

	// Array holding the Locations where the HandoverCube will be moved throughout the test.
	TArray<FVector> TestLocations;

	// Index used to store what Server should be authoritative over the HandoverCube at each step in the test.
	int AuthorityCheckIndex;

	// Array holding what Servers should be authoritative over the HandoverCube throughout the test.
	TArray<int> ExpectedAuthoritativeServer;
};
