// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "CoreMinimal.h"

#include "DynamicReplicationHandoverCube.h"
#include "SpatialFunctionalTest.h"
#include "SpatialTestHandoverReplication.generated.h"

class ADynamicReplicationHandoverCube;

UCLASS()
class SPATIALGDKFUNCTIONALTESTS_API ASpatialTestHandoverReplication : public ASpatialFunctionalTest
{
	GENERATED_BODY()
public:
	ASpatialTestHandoverReplication();

	virtual void PrepareTest() override;

private:
	ADynamicReplicationHandoverCube* HandoverCube;

	// The Load Balancing used by the test, needed to decide what Server should have authority over the TestActor.
	ULayeredLBStrategy* LoadBalancingStrategy;

	void RequireHandoverCubeAuthorityAndPosition(int WorkerShouldHaveAuthority, FVector ExpectedPosition);

	bool MoveHandoverCube(FVector Position);

	// Positions that belong to specific server according to 2x2 Grid LBS.
	FVector Server1Position;
	FVector Server2Position;
	FVector Server3Position;
	FVector Server4Position;
};
