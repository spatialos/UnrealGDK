// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "SpatialTestRPCTimeoutPlayerController.generated.h"

/**
 *
 */
UCLASS()
class ASpatialTestRPCTimeoutPlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	ASpatialTestRPCTimeoutPlayerController();

	TSoftObjectPtr<UMaterial> SoftMaterialPtr;

	bool IsSuccessfullyResolved();

private:
	UFUNCTION(Client, Reliable)
	void OnSetMaterial(UMaterial* PlayerMaterial);

	UFUNCTION(Client, Reliable)
	void CheckMaterialLoaded();

	virtual void OnPossess(APawn* InPawn) override;

	void CheckValidCharacter();

	void SetMaterialAfterDelay();

	bool bSuccessfullyResolved = false;

	UPROPERTY()
	UMaterial* FailedMaterialAsset;

	FTimerHandle HasValidCharacterTimer;

	FTimerHandle MaterialSetDelay;
};
