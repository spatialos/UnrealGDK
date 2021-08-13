#include "ScavengersHubCharacterCheatDetection.h"

#if 0

void FScavengersHubCharacterCheatDetection::Tick(float DeltaTime)
{
    Time.Tick(DeltaTime);
}

void FScavengersHubCharacterCheatDetection::ReceivedMovementUpdate(FScavengersHubMovement Movement, EMovementMode MovementMode)
{
    TimeLastHadUpdate = Time.GetCurrentTime();
    if (MovementMode != MOVE_Falling && MovementMode != MOVE_Flying)
    {
        TimeLastNotFallingOrFlying = Time.GetCurrentTime();
    }

    MovementHistory.Add(MovementUpdate{Time.GetCurrentTime(), Movement, MovementMode});
	if (MovementHistory.Num() > ULiveConfig::GetInt32(TEXT("morpheus"), TEXT("CharacterCheatBufferSize"), 8))
	{
		MovementHistory.RemoveAt(0);
	}
}

bool FScavengersHubCharacterCheatDetection::IsSuspicious()
{
    if (Time.GetCurrentTime() < NextCheckTime)
    {
        return CachedIsSuspicious;
    }

    CachedIsSuspicious = CalculateIsSuspicious();
    NextCheckTime = Time.GetCurrentTime() + ULiveConfig::GetInt32(TEXT("morpheus"), TEXT("CharacterCheatCheckInterval"), 2.0f);
    return CachedIsSuspicious;
}

bool FScavengersHubCharacterCheatDetection::CalculateIsSuspicious()
{
    if (ULiveConfig::GetBool(TEXT("morpheus"), TEXT("DisableCheatDetection"), false))
    {
        return false;
    }

    if (IsSuspiciousOverride)
    {
        return IsSuspiciousOverride.GetValue();
    }

    const float FlyingTimeLimit = ULiveConfig::GetFloat(TEXT("morpheus"), TEXT("CharacterFlyLimit"), 15.0f);
    const float TimeSinceLastUpdate = Time.GetCurrentTime() - TimeLastHadUpdate;
    const float TimeSinceLastOnGround = Time.GetCurrentTime() - TimeLastNotFallingOrFlying;

    if (TimeSinceLastOnGround > FlyingTimeLimit && TimeSinceLastUpdate < FlyingTimeLimit)
    {
        // This character has been flying for too long, while receiving updates.
        return true;
    }

    const bool HasMovedTooQuicklyOnGround =
        MedianRecentVelocity(MOVE_Walking) > ULiveConfig::GetFloat(TEXT("morpheus"), TEXT("CharacterWalkLimit"), 1500.0f);
    return HasMovedTooQuicklyOnGround;
}

float FScavengersHubCharacterCheatDetection::MedianRecentVelocity(TOptional<EMovementMode> MovementModeFilter)
{
    static TArray<float> VelocitySamples;
    VelocitySamples.Empty();

    for (auto i = 0; i < MovementHistory.Num() - 1; i++)
    {
        if (MovementModeFilter)
        {
            if (MovementHistory[i].MovementMode != MovementModeFilter.GetValue() ||
                MovementHistory[i + 1].MovementMode != MovementModeFilter.GetValue())
            {
                continue;
            }
        }
        const float DistanceDelta =
            FVector::Distance(MovementHistory[i].Movement.WorldLocation, MovementHistory[i + 1].Movement.WorldLocation);
        const float TimeDelta = MovementHistory[i + 1].ReceivedTime - MovementHistory[i].ReceivedTime;
        const float Velocity = DistanceDelta / TimeDelta;
        VelocitySamples.Add(Velocity);
    }

    if (VelocitySamples.Num() > 0)
    {
        VelocitySamples.Sort();
        return VelocitySamples[VelocitySamples.Num() / 2];
    }
    else
    {
        return 0;
    }
}

void FScavengersHubCharacterCheatDetection::SetIsSuspiciousOverride(TOptional<bool> Override)
{
    IsSuspiciousOverride = Override;
}

#endif
