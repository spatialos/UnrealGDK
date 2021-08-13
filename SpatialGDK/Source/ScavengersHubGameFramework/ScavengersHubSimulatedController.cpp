// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "ScavengersHubSimulatedController.h"

#include "GameFramework/Character.h"
#include "Kismet/KismetMathLibrary.h"

#if 0

AScavengersHubSimulatedController::AScavengersHubSimulatedController()
{
    PrimaryActorTick.bCanEverTick = true;
}

void AScavengersHubSimulatedController::BeginPlay()
{
    Super::BeginPlay();

    CurrentState.Emplace<StandingState>(StandingState{Time.GetCurrentTime()});
}

void AScavengersHubSimulatedController::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    Time.Tick(DeltaTime);

    if (!GetCharacter())
    {
        return;
    }

    SetControlRotation(TargetRotation);
    GetCharacter()->SetActorRotation(TargetRotation);

    if (auto* State = CurrentState.TryGet<StandingState>())
    {
        if (Time.GetCurrentTime() >= State->StartTime + 5.0f)
        {
            auto Target = ChooseNextTarget(GetCharacter()->GetActorLocation());
            if (Target)
            {
                CurrentState.Emplace<WalkingState>(WalkingState{Target.GetValue(), Time.GetCurrentTime()});
            }
        }
    }

    if (auto* State = CurrentState.TryGet<WalkingState>())
    {
        auto Delta2D = State->Target - GetCharacter()->GetActorLocation();
        Delta2D.Z = 0;
        Delta2D.Normalize();
        GetCharacter()->AddMovementInput(Delta2D, 0.8);

        if (Time.GetCurrentTime() >= State->StartTime + 10.0f)
        {
            CurrentState.Emplace<StandingState>(StandingState{Time.GetCurrentTime()});
        }
    }
}

TOptional<FVector> AScavengersHubSimulatedController::ChooseNextTarget(FVector PlayerPosition)
{
    const float FakePlayerTargetRange = ULiveConfig::GetFloat(TEXT("morpheus"), TEXT("FakePlayerTargetRange"), 3000.0f);
    // Circle of this radius has area 1000m^2
    const float FakePlayerRange = ULiveConfig::GetFloat(TEXT("morpheus"), TEXT("FakePlayerRange"), 54100.0f);

    auto Angle = FMath::RandRange(0.0f, PI * 2.0f);
    FVector Delta(FakePlayerTargetRange * FMath::Cos(Angle), FakePlayerTargetRange * FMath::Sin(Angle), 0.0);
    auto Target = PlayerPosition + Delta;

    // Ensure target is within play space. If outside of space, walk closer to the space.
    auto IsAcceptableRange =
        Target.SizeSquared() < FakePlayerRange * FakePlayerRange || Target.SizeSquared() < PlayerPosition.SizeSquared();

    if (IsAcceptableRange)
    {
        TargetRotation = UKismetMathLibrary::FindLookAtRotation(FVector::ZeroVector, Delta);
        return Target;
    }
    else
    {
        return {};
    }
}

#endif
