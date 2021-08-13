#include "MorpheusRenderTargetManager.h"

#if 0

#include "DrawDebugHelpers.h"
#include "GameFramework/PlayerController.h"

#include <EngineUtils.h>

#include <algorithm>

void UMorpheusRenderTargetManager::AddMorpheusActor(AMorpheusActor* MorpheusActor)
{
    SCAVENGERS_HUB_TRY_CHECK(MorpheusActor);

    MorpheusActor->Private_RenderTargetManager = this;
    MorpheusActorsByType.FindOrAdd(MorpheusActor->GetClass()).Add(MorpheusActor->EntityId, MorpheusActor);
}

void UMorpheusRenderTargetManager::RemoveMorpheusActor(AMorpheusActor* MorpheusActor)
{
    SCAVENGERS_HUB_TRY_CHECK(MorpheusActor);

    auto& MorpheusActors = MorpheusActorsByType.FindOrAdd(MorpheusActor->GetClass());
    MorpheusActors.Remove(MorpheusActor->EntityId);
    if (MorpheusActors.Num() == 0)
    {
        MorpheusActorsByType.Remove(MorpheusActor->GetClass());
    }
}

void UMorpheusRenderTargetManager::UpdateClientLodLevels()
{
    CachedOriginPosition = FVector::ZeroVector;
    CachedDirection = FVector::ForwardVector;
    CachedFOVCosine = 0.0f;

    // How much to favor entities in the view frustrum (lower is more)
    CachedFOVBias = ULiveConfig::GetFloat(TEXT("morpheus"), TEXT("RenderTargetManagerFOVBias"), 1.0);

    // Added degrees from Camera FOV to stop edge effects
    auto FOVSurplus = ULiveConfig::GetFloat(TEXT("morpheus"), TEXT("RenderTargetManagerFOVSurplus"), 20.0);

    if (GetWorld()->GetFirstPlayerController() && GetWorld()->GetFirstPlayerController()->PlayerCameraManager)
    {
        auto* CameraManager = GetWorld()->GetFirstPlayerController()->PlayerCameraManager;
        CachedOriginPosition = CameraManager->GetCameraLocation();
        CachedDirection = CameraManager->GetCameraRotation().Vector();
        auto FOVAngle = CameraManager->GetFOVAngle() + FOVSurplus;
        CachedFOVCosine = FMath::Cos(FMath::DegreesToRadians(FOVAngle * 0.5f));
    }

    TArray<TPair<float, AMorpheusActor*>> PriorityBuffer;
    for (auto& EntityClassKvp : MorpheusActorsByType)
    {
        auto& MorpheusActors = EntityClassKvp.Value;

        if (MorpheusActors.Num() == 0)
        {
            continue;
        }

        PriorityBuffer.Reset(MorpheusActors.Num());
        for (auto& Kvp : MorpheusActors)
        {
            if (AMorpheusActor* MorpheusActor = Kvp.Value.Get())
            {
                PriorityBuffer.Add(TPair<float, AMorpheusActor*>{MorpheusActor->Private_CachedRenderPriority, MorpheusActor});
            }
        }

        if (PriorityBuffer.Num() == 0)
        {
            continue;
        }

        const auto& LodLevels = PriorityBuffer[0].Value->MorpheusActorRenderTargetComponent->GetClientLodLevels();
        auto NextLodLevelIndex = 0;
        while (PriorityBuffer.Num() > 0 && NextLodLevelIndex < LodLevels.Num())
        {
            const auto LodLevelIndex = NextLodLevelIndex++;
            const auto& LodLevel = LodLevels[LodLevelIndex];

            const auto InterestedIndex = FMath::Min(static_cast<int>(LodLevel.NumEntities), PriorityBuffer.Num());
            std::nth_element(PriorityBuffer.GetData(), PriorityBuffer.GetData() + InterestedIndex,
                PriorityBuffer.GetData() + PriorityBuffer.Num());

            for (auto i = 0; i < InterestedIndex; i++)
            {
                SetRenderTarget(PriorityBuffer[i].Value, LodLevelIndex);
            }
            PriorityBuffer.RemoveAt(0, InterestedIndex);
        }

        // Fill the remaining entities with no LOD level.
        for (auto i = 0; i < PriorityBuffer.Num(); i++)
        {
            SetRenderTarget(PriorityBuffer[i].Value, {});
        }
    }
}

float UMorpheusRenderTargetManager::GetRenderPriorityInParallel(AMorpheusActor* MorpheusActor)
{
    auto EntityPosition = MorpheusActor->MorpheusActorMovementComponent->GetCurrentMovement().WorldLocation;
    auto DistanceFromAuthoritativePlayerSquared = FVector::DistSquared(EntityPosition, CachedOriginPosition);
    auto OriginToEntity = EntityPosition - CachedOriginPosition;
    OriginToEntity.Normalize();
    auto EntityDirection = FVector::DotProduct(OriginToEntity, CachedDirection);

    if (EntityDirection > CachedFOVCosine)
    {
        // Favour towards entities in field of view
        DistanceFromAuthoritativePlayerSquared *= CachedFOVBias;
    }

    if (MorpheusActor->MorpheusActorRenderTargetComponent->GetClientLodLevelIndex())
    {
        // Favour entities which are already lower LOD levels to reduce churn.
        DistanceFromAuthoritativePlayerSquared *=
            FMath::Pow(1.05f, MorpheusActor->MorpheusActorRenderTargetComponent->GetClientLodLevelIndex().GetValue());
    }

    return DistanceFromAuthoritativePlayerSquared;
}

void UMorpheusRenderTargetManager::SetRenderTarget(AMorpheusActor* MorpheusActor, TOptional<int> LodLevelIndex)
{
    if (LodLevelIndex != MorpheusActor->MorpheusActorRenderTargetComponent->GetClientLodLevelIndex())
    {
        MorpheusActor->MorpheusActorRenderTargetComponent->SetClientLodLevelIndex(LodLevelIndex);
    }
}

void UMorpheusRenderTargetManager::ForEachEntityWithBaseClass(
    const TSubclassOf<AMorpheusActor>& BaseClass, const TFunction<void(AMorpheusActor*)>& Callback)
{
    for (const auto& Entry : MorpheusActorsByType)
    {
        if (Entry.Key.IsValid() && Entry.Key->IsChildOf(BaseClass))
        {
            for (const auto& Kvp : Entry.Value)
            {
                if (Kvp.Value.IsValid())
                {
                    Callback(Kvp.Value.Get());
                }
            }
        }
    }
}

#endif
