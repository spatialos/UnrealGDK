#include "ScavengersHubCharacterObject.h"

#if 0

#include "Components/CapsuleComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "ScavengersHubFunctionLibrary.h"
#include "ScavengersHubGDK/MorpheusClientConnection.h"
#include "ScavengersHubSimulatedController.h"
#include "TimerManager.h"

AScavengersHubCharacterObject::AScavengersHubCharacterObject()
{
    MorpheusActorMovementComponent->MovementAuthority = ClientAuthoritative;
    AutoPossessPlayerOnClientAuthority = EAutoReceiveInput::Player0;
    DisableBlueprintTick = true;

    // Default characters to the background as they are normally high scale.
    MinimumNetworkLevel = EMorpheusNetworkLevel::Background;

    SetReplicatingMovement(true);
}

void AScavengersHubCharacterObject::ApplyPackedMovementData(FPackedMovementData MovementData)
{
    if (GetCharacter())
    {
        GetCharacter()->GetCharacterMovement()->SetMovementMode(MovementData.GetMovementMode());

        if (GetCharacter()->bIsCrouched != MovementData.GetIsCrouched())
        {
            GetCharacter()->bIsCrouched = MovementData.GetIsCrouched();
            GetCharacter()->OnRep_IsCrouched();
        }
    }
}

void AScavengersHubCharacterObject::NetworkLevelChanged()
{
    Super::NetworkLevelChanged();

    if (NetworkLevel == EMorpheusNetworkLevel::Background)
    {
        // We have entered the background. Clear our packed movement.
        PackedMovementData = {};
    }
}

void AScavengersHubCharacterObject::SetDisableMeshComponentTick(bool Disable)
{
    DisableMeshComponentTick = Disable;
    if (GetCharacter() && GetCharacter()->GetMesh())
    {
        GetCharacter()->GetMesh()->SetComponentTickEnabled(!DisableMeshComponentTick);
    }
}

void AScavengersHubCharacterObject::AboutToFlushBackgroundProperties()
{
    if (MorpheusActorMovementComponent->HasMovementAuthority())
    {
        // This must be done before the Super:: call, to ensure we replicate the movement base (if needed) BEFORE we replicate any
        // movement updates.
        UpdateMovementBase();
    }

    Super::AboutToFlushBackgroundProperties();

    if (IsOnClient())
    {
        if (GetCharacter())
        {
            PackedMovementData.SetIsCrouched(GetCharacter()->bIsCrouched);
            PackedMovementData.SetMovementMode(GetCharacter()->GetCharacterMovement()->MovementMode);
        }

        if (UpdateClientOriginMovement)
        {
            Cast<AMorpheusClientConnection>(ActiveConnection)->ClientOriginMovement =
                MorpheusActorMovementComponent->GetCurrentMovement();
        }
    }
}

void AScavengersHubCharacterObject::UpdateMovementBase()
{
    if (GetCharacter() != nullptr)
    {
        if (GetCharacter()->GetMovementComponent() != nullptr)
        {
            if (UCharacterMovementComponent* UEMovementComp =
                    Cast<UCharacterMovementComponent>(GetCharacter()->GetMovementComponent()))
            {
                if (UPrimitiveComponent* UEMovementBase = UEMovementComp->GetMovementBase())
                {
                    MovementBase = Cast<AMorpheusActor>(UEMovementBase->GetOwner());
                    if (MovementBase == nullptr)
                    {
                        MovementBase = ScavengersHubFunctionLibrary::GetMorpheusActor<AMorpheusActor>(UEMovementBase->GetOwner());
                    }
                    if (MovementBase == nullptr)
                    {
                        CachedMovementBasePrimitive = nullptr;
                    }
                    else
                    {
                        CachedMovementBasePrimitive = UEMovementBase;
                    }
                    return;
                }
            }
        }
    }
    MovementBase = nullptr;
    CachedMovementBasePrimitive = nullptr;
}

void AScavengersHubCharacterObject::PostSpawnRenderTargetActor()
{
    Super::PostSpawnRenderTargetActor();

    // Apply the current packed movement for the new character.
    ApplyPackedMovementData(PackedMovementData);

    if (DisableMeshComponentTick && GetCharacter()->GetMesh())
    {
        GetCharacter()->GetMesh()->SetComponentTickEnabled(false);
    }

    GetCharacter()->GetCharacterMovement()->NetworkNoSmoothUpdateDistance =
        ULiveConfig::GetFloat(TEXT("morpheus"), TEXT("CharacterNetworkNoSmoothUpdateDistance"), 3000);
    GetCharacter()->GetCharacterMovement()->NetworkMaxSmoothUpdateDistance =
        ULiveConfig::GetFloat(TEXT("morpheus"), TEXT("CharacterNetworkMaxSmoothUpdateDistance"), 3000);

    if (HasClientAuthority)
    {
        // Maybe auto possess character.
        if (AutoPossessPlayerOnClientAuthority != EAutoReceiveInput::Disabled)
        {
            const int32 PlayerIndex = static_cast<int32>(AutoPossessPlayerOnClientAuthority.GetValue()) - 1;
            APlayerController* Controller = UGameplayStatics::GetPlayerController(this, PlayerIndex);
            if (Controller)
            {
                Controller->SetRole(ROLE_Authority);
                Controller->Possess(GetCharacter());
                Controller->SetControlRotation(MorpheusActorMovementComponent->GetCurrentMovement().Rotation);
            }
        }

        if (Cast<AMorpheusClientConnection>(ActiveConnection)->IsSimulatedPlayer)
        {
            // Spawn a SimulatedPlayer controller for this Character.
            AController* SimulatedController =
                GetCharacter()->GetWorld()->SpawnActor<AController>(SimulatedPlayerControllerClass.Get());
            SimulatedController->SetRole(ROLE_Authority);
            SimulatedController->Possess(GetCharacter());
        }
    }
}

FCustomMovementData AScavengersHubCharacterObject::GetCurrentReplicatedCustomMovementData()
{
    return {PackedMovementData.PackedValue};
}

void AScavengersHubCharacterObject::ApplyCustomMovementData(FCustomMovementData CustomMovementData)
{
    // Only apply the packed movement data if we are at least in the midground network level.
    if (NetworkLevel >= EMorpheusNetworkLevel::Midground)
    {
        ApplyPackedMovementData({CustomMovementData.CustomMovementData});
    }
}

ACharacter* AScavengersHubCharacterObject::GetCharacter() const
{
    return Cast<ACharacter>(MorpheusActorRenderTargetComponent->CurrentActor);
}

void AScavengersHubCharacterObject::GetMorpheusReplicatedPropertyParameters(TArray<FScavengersHubPropertyParam>& OutParams)
{
    Super::GetMorpheusReplicatedPropertyParameters(OutParams);

    DOREP_MORPHEUS_AUTHORITY_OVERRIDE(
        AScavengersHubCharacterObject, PackedMovementData, MorpheusActorMovementComponent->MovementAuthority);
}

#endif
