// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "SpatialFunctionalTest.h"

#include "CrossServerMultiPossessionTest.generated.h"

class ATestPossessionPawn;

UCLASS()
class SPATIALGDKFUNCTIONALTESTS_API ACrossServerMultiPossessionTest : public ASpatialFunctionalTest
{
	GENERATED_BODY()

public:
	ACrossServerMultiPossessionTest();

	virtual void PrepareTest() override;

private:
	ATestPossessionPawn* GetPawn();
	void CreateController(int Index, FVector Position);
	void CheckControllerHasAuthority(int Index);
	void RemotePossess(int Index);

	UFUNCTION()
	void OnPossess(APawn* Pawn, APlayerController* Controller);

	UFUNCTION()
	void OnUnPossess(APlayerController* Controller);

	UFUNCTION()
	void OnPossessFailed(ERemotePossessFailure FailureReason, APlayerController* Controller);

	float WaitTime;
	const static float MaxWaitTime;
};
