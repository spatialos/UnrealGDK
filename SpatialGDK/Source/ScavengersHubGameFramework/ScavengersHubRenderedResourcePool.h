#pragma once

#if 0
#include "ScavengersHubIsmActor.h"
#include "ScavengersHubPooledActor.h"
#include "UObject/StrongObjectPtr.h"

#include "ScavengersHubRenderedResourcePool.generated.h"


UCLASS(config = Game)
class AScavengersHubRenderedResourcePool : public AActor
{
    GENERATED_BODY()
public:
    AActor* GetNewActor(
        AMorpheusActor* MorpheusActor, UClass* ActorClass, FVector Location, FRotator Rotation, bool& RequiresFinishSpawning);
    void FreeActor(AMorpheusActor* MorpheusActor, AActor* Actor);
    AScavengersHubIsmActor* GetIsmForRenderTarget(const FScavengersHubRenderTarget& RenderTarget);

    UPROPERTY(config)
    TSoftClassPtr<AScavengersHubIsmActor> IsmActorClass;

private:
    TMap<UStaticMesh*, TStrongObjectPtr<AScavengersHubIsmActor>> InstancedStaticMeshes;
    TMap<FEntityActorClassPair, TArray<TWeakObjectPtr<AActor>>> ActorPool;
    TOptional<bool> EnableActorPooling;
};

#endif
