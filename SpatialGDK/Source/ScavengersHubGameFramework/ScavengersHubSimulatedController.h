// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "Misc/TVariant.h"
#include "RenderCore.h"

#if 0

#include "ScavengersHubSimulatedController.generated.h"

/**
 *
 */
UCLASS()
class SCAVENGERSHUBGAMEFRAMEWORK_API AScavengersHubSimulatedController : public APlayerController
{
    GENERATED_BODY()

protected:
    virtual void BeginPlay() override;

public:
    AScavengersHubSimulatedController();

    virtual void Tick(float DeltaTime) override;

private:
    struct StandingState
    {
        float StartTime;
    };

    struct WalkingState
    {
        FVector Target;
        float StartTime;
    };

    using State = TVariant<StandingState, WalkingState>;

    State CurrentState;
    FTimer Time;

    FRotator TargetRotation;

    TOptional<FVector> ChooseNextTarget(FVector PlayerPosition);
};

#endif
