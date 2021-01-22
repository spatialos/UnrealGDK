// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "SpatialFunctionalTest.h"
#include "SpatialTestRemotePossession.generated.h"

class ATestPossessionPawn;
class ATestPossessionController;

UCLASS()
class SPATIALGDKFUNCTIONALTESTS_API ASpatialTestRemotePossession : public ASpatialFunctionalTest
{
	GENERATED_BODY()
public:
	ASpatialTestRemotePossession();

	virtual void PrepareTest() override;

	virtual void CreateControllerAndPawn() {}

	ATestPossessionPawn* GetPawn();
	ATestPossessionController* GetController();

	void CreateController(FVector Location);
	void CreatePawn(FVector Location);

	bool IsReadyForPossess();

	void AddWaitStep(const FWorkerDefinition& Worker);

	void AddCleanStep();

protected:
	float WaitTime;
	const static float MaxWaitTime;
};
