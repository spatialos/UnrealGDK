// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "SpatialCommonTypes.h"
#include "TestPossessionController.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(LogTestPossessionController, Log, All);

UCLASS()
class ATestPossessionController : public AController
{
	GENERATED_BODY()
private:
	virtual void OnPossess(APawn* InPawn) override;

	virtual void OnUnPossess() override;

public:
	ATestPossessionController();

	UFUNCTION(CrossServer, Reliable)
	void RemotePossess(APawn* InPawn);

	void RemotePossessOnServer(APawn* InPawn, bool bLockBefore = false);

	void ReleaseLock();

	bool IsMigration() const { return BeforePossessionWorkerId != AfterPossessionWorkerId; }

	static void ResetCalledCounter();

	static int32 OnPossessCalled;

private:
	VirtualWorkerId GetCurrentWorkerId();

	VirtualWorkerId BeforePossessionWorkerId;
	VirtualWorkerId AfterPossessionWorkerId;

	ActorLockToken LockToken;
};
