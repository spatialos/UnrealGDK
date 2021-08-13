#pragma once
#include "Containers/CircularBuffer.h"
#include "RenderCore.h"
#include "ScavengersHubMovement.h"

#if 0

#include "ScavengersHubCharacterCheatDetection.generated.h"

USTRUCT()
struct FScavengersHubCharacterCheatDetection
{
    GENERATED_BODY()

public:
    virtual ~FScavengersHubCharacterCheatDetection() = default;

    virtual void Tick(float DeltaTime);
    virtual void ReceivedMovementUpdate(FScavengersHubMovement Movement, EMovementMode MovementMode);
    virtual bool IsSuspicious();
    float MedianRecentVelocity(TOptional<EMovementMode> MovementModeFilter);
    void SetIsSuspiciousOverride(TOptional<bool> Override);

protected:
    virtual bool CalculateIsSuspicious();

    FTimer Time;
    struct MovementUpdate
    {
        float ReceivedTime;
        FScavengersHubMovement Movement;
        EMovementMode MovementMode;
    };
    TArray<MovementUpdate> MovementHistory;
    TOptional<bool> IsSuspiciousOverride;

private:
    float NextCheckTime = 0;
    bool CachedIsSuspicious = false;
    float TimeLastNotFallingOrFlying = 0;
    float TimeLastHadUpdate = 0;
};

#endif
