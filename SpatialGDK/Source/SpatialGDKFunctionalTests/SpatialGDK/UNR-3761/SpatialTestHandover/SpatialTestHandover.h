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

	// The Load Balancing used by the test, needed to decide what Server should have authority over the HandoverCube.
	ULayeredLBStrategy* LoadBalancingStrategy;

	void RequireHandoverCubeAuthorityAndPosition(int WorkerShouldHaveAuthority, FVector ExpectedPosition);

	bool MoveHandoverCube(FVector Position);

	// Position in Server1, where HandoverCube will be spawned
	FVector Server1Position;
	FVector Server2Position;
	FVector Server3Position;
	FVector Server4Position;
};
