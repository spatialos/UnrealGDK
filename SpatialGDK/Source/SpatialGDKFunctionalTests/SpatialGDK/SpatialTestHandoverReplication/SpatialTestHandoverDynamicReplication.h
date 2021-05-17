// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "CoreMinimal.h"

#include "SpatialFunctionalTest.h"
#include "SpatialTestHandoverDynamicReplication.generated.h"

class ADynamicReplicationHandoverCube;

UCLASS()
class SPATIALGDKFUNCTIONALTESTS_API ASpatialTestHandoverDynamicReplication : public ASpatialFunctionalTest
{
	GENERATED_BODY()
public:
	ASpatialTestHandoverDynamicReplication();

	virtual void PrepareTest() override;

private:
	ADynamicReplicationHandoverCube* HandoverCube;

	// The Load Balancing used by the test, needed to decide what Server should have authority over the TestActor.
	ULayeredLBStrategy* LoadBalancingStrategy;

	void RequireHandoverCubeAuthorityAndPosition(int WorkerShouldHaveAuthority, const FVector& ExpectedPosition);

	bool MoveHandoverCube(const FVector& Position);

	// Positions that belong to specific server according to 2x2 Grid LBS.
	FVector Server1Position;
	FVector Server2Position;
	FVector Server3Position;
	FVector Server4Position;
};
