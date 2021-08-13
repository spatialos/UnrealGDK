#pragma once

#if 0

#include "ScavengersHubGDK/Common/MorpheusActor/MorpheusActor.h"

#include <UObject/Object.h>

#include "MorpheusRenderTargetManager.generated.h"

UCLASS()
class SCAVENGERSHUBGAMEFRAMEWORK_API UMorpheusRenderTargetManager : public UObject
{
    GENERATED_BODY()
public:
    TMap<TWeakObjectPtr<UClass>, TMap<int64, TWeakObjectPtr<AMorpheusActor>>> MorpheusActorsByType;

    void AddMorpheusActor(AMorpheusActor* MorpheusActor);
    void RemoveMorpheusActor(AMorpheusActor* MorpheusActor);
    virtual void UpdateClientLodLevels();
    // This is NOT thread safe and is ran in parallel from AMorpheusActors.
    virtual float GetRenderPriorityInParallel(AMorpheusActor* MorpheusActor);

    void SetRenderTarget(AMorpheusActor* MorpheusActor, TOptional<int> LodLevelIndex);
    void ForEachEntityWithBaseClass(const TSubclassOf<AMorpheusActor>& BaseClass, const TFunction<void(AMorpheusActor*)>& Callback);

private:
    FVector CachedOriginPosition = FVector::ZeroVector;
    FVector CachedDirection = FVector::ForwardVector;
    float CachedFOVBias = 0.0f;
    float CachedFOVCosine = 0.0f;
};

#endif
