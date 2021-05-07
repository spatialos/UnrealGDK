// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "PlayerDisconnectController.generated.h"

/**
 * Used for testing that players are cleaned up correctly when they return to the main menu.
 */
UCLASS()
class SPATIALGDKFUNCTIONALTESTS_API APlayerDisconnectController : public APlayerController
{
	GENERATED_BODY()

public:
	virtual void SetupInputComponent() override;

	UFUNCTION()
	void MPressed();
};
