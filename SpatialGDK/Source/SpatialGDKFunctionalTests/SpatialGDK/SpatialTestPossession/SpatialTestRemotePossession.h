// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "SpatialFunctionalTest.h"
#include "TestPossessionPlayerController.h"
#include "SpatialTestRemotePossession.generated.h"

class ATestPossessionPawn;

USTRUCT()
struct FControllerPawnPair
{
	GENERATED_BODY()

	UPROPERTY()
	ATestPossessionPlayerController* PlayerController;

	UPROPERTY()
	APawn* Pawn;
};

UCLASS()
class SPATIALGDKFUNCTIONALTESTS_API ASpatialTestRemotePossession : public ASpatialFunctionalTest
{
	GENERATED_BODY()
public:
	ASpatialTestRemotePossession();

	virtual void PrepareTest() override;

	ATestPossessionPawn* GetPawn();

	bool IsReadyForPossess();

	void AddWaitStep(const FWorkerDefinition& Worker);

protected:
	FVector LocationOfPawn;
	float WaitTime;
	const static float MaxWaitTime;

	void AddCleanupSteps();

	UFUNCTION(CrossServer, Reliable)
	void AddToOriginalPawns(ATestPossessionPlayerController* PlayerController, APawn* Pawn);

	// To save original Pawns and possess them back at the end
	UPROPERTY(Handover, Transient)
	TArray<FControllerPawnPair> OriginalPawns;
};
