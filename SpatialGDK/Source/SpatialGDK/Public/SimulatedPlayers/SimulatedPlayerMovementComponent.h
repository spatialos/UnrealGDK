// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "SimulatedPlayerMovementComponent.generated.h"

/**
 * 
 */
UCLASS()
class SPATIALGDK_API USimulatedPlayerMovementComponent : public UCharacterMovementComponent
{
	GENERATED_BODY()
	
public:
	// Override to redirect these movement requests to the controlled player
	virtual void RequestDirectMove(const FVector& MoveVelocity, bool bForceMaxSpeed) override;
	virtual void RequestPathMove(const FVector& MoveInput) override;	
};
