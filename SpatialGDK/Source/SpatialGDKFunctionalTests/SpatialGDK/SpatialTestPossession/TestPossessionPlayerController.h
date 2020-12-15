// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "TestPossessionPlayerController.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(LogTestPossessionPlayerController, Log, All);

UCLASS()
class ATestPossessionPlayerController : public APlayerController
{
	GENERATED_BODY()
private:
	virtual void OnPossess(APawn* InPawn) override;

	virtual void OnUnPossess() override;

	virtual void OnPossessFailed(ERemotePossessFailure FailureReason) override;
public:
	ATestPossessionPlayerController();

	UFUNCTION(Server, Reliable)
	void RemotePossess(APawn* InPawn);

	static void ResetCalledCounter();

	static int32 OnPossessCalled;

	static int32 OnPossessFailedCalled;
};
