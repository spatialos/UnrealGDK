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

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	void RemotePossessOnServer(APawn* InPawn);

	void RemovePossessionComponent();

	UFUNCTION(Server, Reliable)
	void RemotePossessOnClient(APawn* InPawn, bool bLockBefore);

	bool HasMigrated() const { return BeforePossessionWorkerId != AfterPossessionWorkerId; }

	void UnlockAllTokens();

	static void ResetCalledCounter();

	static int32 OnPossessCalled;

private:
	VirtualWorkerId GetCurrentWorkerId();

	UPROPERTY(Replicated)
	uint32 BeforePossessionWorkerId;

	UPROPERTY(Replicated)
	uint32 AfterPossessionWorkerId;

	TArray<ActorLockToken> Tokens;
};
