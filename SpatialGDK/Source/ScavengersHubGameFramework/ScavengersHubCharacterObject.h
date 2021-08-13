#pragma once

#if 0

#include "BackwardsCompatibility.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "ScavengersHubCharacterCheatDetection.h"
#include "ScavengersHubGDK/Common/MorpheusActor/MorpheusActor.h"
#include "ScavengersHubSimulatedController.h"
#include "Tickable.h"

#include "ScavengersHubCharacterObject.generated.h"

USTRUCT()
struct FPackedMovementData
{
    GENERATED_BODY()

    UPROPERTY()
    uint64 PackedValue;

    FPackedMovementData()
    {
        PackedValue = 0;
        SetMovementMode(MOVE_None);
    }

    FPackedMovementData(uint64 Value)
    {
        PackedValue = Value;
    }

    TEnumAsByte<EMovementMode> GetMovementMode() const
    {
        return TEnumAsByte<EMovementMode>(static_cast<uint8>(PackedValue));
    }

    void SetMovementMode(TEnumAsByte<EMovementMode> Mode)
    {
        PackedValue &= ~0b11111111;
        PackedValue |= static_cast<uint8>(Mode);
    }

    bool GetIsCrouched() const
    {
        return GetBit(8 + kCrouchBuiltInSlotIndex);
    }

    void SetIsCrouched(bool Value)
    {
        SetBit(8 + kCrouchBuiltInSlotIndex, Value);
    }

    bool GetCustomMovementSlot(int Index) const
    {
        return GetBit(8 + kNumBuiltInSlots + Index);
    }

    void SetCustomMovementSlot(int Index, bool Value)
    {
        SetBit(8 + kNumBuiltInSlots + Index, Value);
    }

private:
    static const int kCrouchBuiltInSlotIndex = 0;
    static const int kNumBuiltInSlots = 1;

    bool GetBit(int BitIndex) const
    {
        return (PackedValue & (static_cast<uint64>(1) << static_cast<uint64>(BitIndex))) != 0;
    }

    void SetBit(int BitIndex, bool Value)
    {
        // Clear the bit.
        PackedValue &= ~(static_cast<uint64>(1) << static_cast<uint64>(BitIndex));
        if (Value)
        {
            // Set the bit.
            PackedValue |= static_cast<uint64>(1) << static_cast<uint64>(BitIndex);
        }
    }
};

UCLASS(Abstract)
class SCAVENGERSHUBGAMEFRAMEWORK_API AScavengersHubCharacterObject : public AEntityObject
{
    GENERATED_BODY()

public:
    AScavengersHubCharacterObject();

    UPROPERTY(EditAnywhere, Category = Pawn)
    TEnumAsByte<EAutoReceiveInput::Type> AutoPossessPlayerOnClientAuthority;

    // The authority of this is overriden in GetMorpheusReplicatedPropertyParameters.
    UPROPERTY(Replicated, meta = (Midground))
    FPackedMovementData PackedMovementData;

    UPROPERTY(EditAnywhere, Category = Pawn)
    TSubclassOf<APlayerController> SimulatedPlayerControllerClass = AScavengersHubSimulatedController::StaticClass();

    virtual void ApplyPackedMovementData(FPackedMovementData MovementData);

    virtual void NetworkLevelChanged() override;
    virtual void SetDisableMeshComponentTick(bool Disable);
    virtual void AboutToFlushBackgroundProperties() override;
    virtual void UpdateMovementBase();
    virtual void PostSpawnRenderTargetActor() override;
    virtual FCustomMovementData GetCurrentReplicatedCustomMovementData() override;
    virtual void ApplyCustomMovementData(FCustomMovementData CustomMovementData) final override;
    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override
    {
        Super::GetLifetimeReplicatedProps(OutLifetimeProps);
    }

    UFUNCTION(BlueprintCallable)
    ACharacter* GetCharacter() const;
    virtual void GetMorpheusReplicatedPropertyParameters(TArray<FScavengersHubPropertyParam>& OutParams) override;

    bool UpdateClientOriginMovement = true;

private:
    bool DisableMeshComponentTick = false;
};

#endif
