// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "SpatialFunctionalTest.h"
#include "CrossServerPlayerControllerPossessPawn.generated.h"

class ATestPossessionPawn;

UCLASS()
class SPATIALGDKFUNCTIONALTESTS_API ACrossServerPlayerControllerPossessPawn : public ASpatialFunctionalTest
{
	GENERATED_BODY()

public:
	ACrossServerPlayerControllerPossessPawn();

	virtual void PrepareTest() override;

	ATestPossessionPawn* GetPawn();

	void AddWaitStep(const FWorkerDefinition& Worker);

protected:
	float WaitTime;
	const static float MaxWaitTime;
};
