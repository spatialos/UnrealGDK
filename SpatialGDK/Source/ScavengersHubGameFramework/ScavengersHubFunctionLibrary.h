#pragma once

#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "Kismet/BlueprintFunctionLibrary.h"

#if 0

#include "ScavengersHubGDK/Common/MorpheusActor/MorpheusActor.h"
#include "ScavengersHubGDK/Common/MorpheusActor/MorpheusActorRenderTargetComponent.h"

#include "ScavengersHubFunctionLibrary.generated.h"

UCLASS()
class SCAVENGERSHUBGAMEFRAMEWORK_API UScavengersHubBlueprintFunctionLibrary : public UBlueprintFunctionLibrary
{
    GENERATED_BODY()

public:
    UFUNCTION(BlueprintCallable)
    static AMorpheusActor* GetMorpheusActor(AActor* Actor);

    UFUNCTION(BlueprintCallable)
    static AMorpheusActor* SpawnMorpheusActorWithClientAuthority(
        UObject* WorldContext, UClass* Class, AMorpheusClientConnection* ClientConnection, FTransform Transform);

    // Deprecated, here for Blueprints.
    UFUNCTION(BlueprintCallable)
    static AMorpheusActor* GetEntityObject(AActor* Actor);
    UFUNCTION(BlueprintCallable)
    static AMorpheusActor* SpawnEntityObjectWithClientAuthority(
        UObject* WorldContext, UClass* Class, AMorpheusClientConnection* ClientConnection, FTransform Transform);
};

class SCAVENGERSHUBGAMEFRAMEWORK_API ScavengersHubFunctionLibrary
{
public:
    template <typename T>
    static T* GetMorpheusActor(AActor* Actor)
    {
        if (Actor)
        {
            if (auto* RenderTargetOwnerComponent = Actor->FindComponentByClass<UMorpheusRenderTargetOwnerComponent>())
            {
                return Cast<T>(RenderTargetOwnerComponent->MorpheusActor);
            }
        }
        return nullptr;
    }

    static void PlaceActorInStandaloneMode(AActor* Actor)
    {
        SCAVENGERS_HUB_TRY_CHECK(Actor);

        // We set the net driver name to an unknown net driver to place the actor in standalone mode.
        Actor->SetNetDriverName(FName(TEXT("DummyNetDriver")));
    }

    // Deprecated
    template <typename T>
    static T* GetEntityObject(AActor* Actor)
    {
        return GetMorpheusActor<T>(Actor);
    }

    template <typename T>
    static T* SpawnMorpheusActorWithClientAuthority(
        UWorld* World, UClass* Class, AMorpheusClientConnection* ClientConnection, FTransform Transform = {})
    {
        return Cast<T>(UScavengersHubBlueprintFunctionLibrary::SpawnMorpheusActorWithClientAuthority(
            World, Class, ClientConnection, Transform));
    }

    // Deprecated
    template <typename T>
    static T* SpawnEntityObjectWithClientAuthority(
        UWorld* World, UClass* Class, AMorpheusClientConnection* ClientConnection, FTransform Transform = {})
    {
        return SpawnMorpheusActorWithClientAuthority<T>(World, Class, ClientConnection, Transform);
    }
};

#endif
