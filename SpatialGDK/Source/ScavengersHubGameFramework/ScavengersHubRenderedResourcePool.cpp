#include "ScavengersHubRenderedResourcePool.h"

#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"
#include "ScavengersHubPooledActor.h"

#if 0

AActor* AScavengersHubRenderedResourcePool::GetNewActor(
    AMorpheusActor* MorpheusActor, UClass* ActorClass, FVector Location, FRotator Rotation, bool& RequiresFinishSpawning)
{
    SCAVENGERS_HUB_TRY_CHECK_RETURN_VALUE(MorpheusActor != nullptr, nullptr);
    SCAVENGERS_HUB_TRY_CHECK_RETURN_VALUE(ActorClass != nullptr, nullptr);

    RequiresFinishSpawning = false;

    // Don't use the pool for client authoritative characters.
    const bool ShouldUsePool = !MorpheusActor->HasClientAuthority;
    if (ShouldUsePool)
    {
        FEntityActorClassPair PoolKey(MorpheusActor->GetClass(), ActorClass);

        if (TArray<TWeakObjectPtr<AActor>>* ClassPool = ActorPool.Find(PoolKey))
        {
            while (ClassPool->Num() > 0)
            {
                TWeakObjectPtr<AActor> FoundActor = ClassPool->Pop();
                if (FoundActor.IsValid())
                {
                    if (FoundActor->GetClass()->ImplementsInterface(UScavengersHubPooledActor::StaticClass()))
                    {
                        IScavengersHubPooledActor::Execute_OnBeginPlayFromPool(FoundActor.Get());
                    }

                    TArray<UActorComponent*> Components =
                        FoundActor->GetComponentsByInterface(UScavengersHubPooledActor::StaticClass());
                    for (UActorComponent* Component : Components)
                    {
                        IScavengersHubPooledActor::Execute_OnBeginPlayFromPool(Component);
                    }

                    FoundActor->SetActorLocationAndRotation(Location, Rotation);

                    return FoundActor.Get();
                }
            }
        }
    }

    // Spawn a new actor.
    FActorSpawnParameters SpawnParams;
    SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
    SpawnParams.bDeferConstruction = true;
    RequiresFinishSpawning = true;
    return GetWorld()->SpawnActor(ActorClass, &Location, &Rotation, SpawnParams);
}

void AScavengersHubRenderedResourcePool::FreeActor(AMorpheusActor* MorpheusActor, AActor* Actor)
{
    SCAVENGERS_HUB_TRY_CHECK(MorpheusActor != nullptr);
    SCAVENGERS_HUB_TRY_CHECK(Actor != nullptr);

    if (!EnableActorPooling.IsSet())
    {
        EnableActorPooling = ULiveConfig::GetBool(TEXT("morpheus"), TEXT("EnableActorPooling"), true);
    }

    bool bShouldPool = false;

    if (EnableActorPooling && MorpheusActor->ShouldReturnActorToPool())
    {
        if (Actor->GetClass()->ImplementsInterface(UScavengersHubPooledActor::StaticClass()))
        {
            IScavengersHubPooledActor::Execute_OnEndPlayToPool(Actor);
            bShouldPool = true;
        }

        TArray<UActorComponent*> Components = Actor->GetComponentsByInterface(UScavengersHubPooledActor::StaticClass());
        if (Components.Num() > 0)
        {
            for (UActorComponent* Component : Components)
            {
                IScavengersHubPooledActor::Execute_OnEndPlayToPool(Component);
            }
            bShouldPool = true;
        }
    }

    if (bShouldPool)
    {
        FEntityActorClassPair PoolKey(MorpheusActor->GetClass(), Actor->GetClass());
        ActorPool.FindOrAdd(PoolKey).Push(Actor);
    }
    else
    {
        Actor->Destroy(true);
    }
}

AScavengersHubIsmActor* AScavengersHubRenderedResourcePool::GetIsmForRenderTarget(const FScavengersHubRenderTarget& RenderTarget)
{
    if (auto* Ism = InstancedStaticMeshes.Find(RenderTarget.StaticMesh))
    {
        return Ism->Get();
    }
    else
    {
        UClass* ActorClass = IsmActorClass.LoadSynchronous();
        if (!ActorClass)
        {
            // Fall back to the default class if the custom one hasn't been specified or fails to load.
            ActorClass = AScavengersHubIsmActor::StaticClass();
        }

        auto* IsmActor = GetWorld()->SpawnActorDeferred<AScavengersHubIsmActor>(ActorClass, FTransform::Identity);
        IsmActor->bUseCustomInstancedStaticMeshComponent = RenderTarget.StaticMeshRenderStrategy == ParallelISM;
        UGameplayStatics::FinishSpawningActor(IsmActor, FTransform::Identity);
        IsmActor->SetStaticMesh(RenderTarget.StaticMesh);
        return InstancedStaticMeshes.Add(RenderTarget.StaticMesh, TStrongObjectPtr<AScavengersHubIsmActor>{IsmActor}).Get();
    }
}

#endif
