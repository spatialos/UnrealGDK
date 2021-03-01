// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "SpatialCommonTypes.h"
#include "TestPossessionPlayerController.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(LogTestPossessionPlayerController, Log, All);

UCLASS()
class ATestPossessionPlayerController : public APlayerController
{
	GENERATED_BODY()
private:
	virtual void OnPossess(APawn* InPawn) override;

	virtual void OnUnPossess() override;

public:
	ATestPossessionPlayerController();

	void RemotePossessOnServer(APawn* InPawn);

	UFUNCTION(Server, Reliable)
	void RemotePossessOnClient(APawn* InPawn, bool bLockBefore);

	bool HasMigrated() const { return BeforePossessionWorkerId != AfterPossessionWorkerId; }

	static void ResetCalledCounter();

	static int32 OnPossessCalled;

private:
	VirtualWorkerId GetCurrentWorkerId();

	VirtualWorkerId BeforePossessionWorkerId;
	VirtualWorkerId AfterPossessionWorkerId;
};
