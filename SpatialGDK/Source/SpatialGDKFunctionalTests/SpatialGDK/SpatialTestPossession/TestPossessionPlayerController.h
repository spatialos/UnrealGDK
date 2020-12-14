// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "TestPossessionPlayerController.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnPossess, APawn*, Pawn, APlayerController*, Controller);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnPossessFailed, ERemotePossessFailure, FailureReason, APlayerController*, Controller);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnUnPossess, APlayerController*, Controller);

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

	UFUNCTION(Client, Reliable)
	void RemotePossess(APawn* InPawn);

	FOnPossess OnPossessEvent;

	FOnUnPossess OnUnPossessEvent;

	FOnPossessFailed OnPossessFailedEvent;
};
