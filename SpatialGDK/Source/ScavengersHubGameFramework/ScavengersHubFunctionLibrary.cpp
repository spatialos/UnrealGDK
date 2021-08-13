#include "ScavengersHubFunctionLibrary.h"

#if 0
#include "ScavengersHubGDK/Common/MorpheusActor/MorpheusActor.h"
#include "ScavengersHubGDK/MorpheusClientConnection.h"

AMorpheusActor* UScavengersHubBlueprintFunctionLibrary::GetMorpheusActor(AActor* Actor)
{
    return ScavengersHubFunctionLibrary::GetMorpheusActor<AMorpheusActor>(Actor);
}

AMorpheusActor* UScavengersHubBlueprintFunctionLibrary::GetEntityObject(AActor* Actor)
{
    return GetMorpheusActor(Actor);
}

AMorpheusActor* UScavengersHubBlueprintFunctionLibrary::SpawnEntityObjectWithClientAuthority(
    UObject* WorldContext, UClass* Class, AMorpheusClientConnection* ClientConnection, FTransform Transform)
{
    return SpawnMorpheusActorWithClientAuthority(WorldContext, Class, ClientConnection, Transform);
}

AMorpheusActor* UScavengersHubBlueprintFunctionLibrary::SpawnMorpheusActorWithClientAuthority(
    UObject* WorldContext, UClass* Class, AMorpheusClientConnection* ClientConnection, FTransform Transform)
{
    if (WorldContext == nullptr || Class == nullptr)
    {
        return nullptr;
    }

    AMorpheusActor* MorpheusActor = WorldContext->GetWorld()->SpawnActorDeferred<AMorpheusActor>(Class, Transform);
    if (MorpheusActor == nullptr)
    {
        return nullptr;
    }
    MorpheusActor->SetOwner(ClientConnection);
    MorpheusActor->FinishSpawning(Transform);
    return MorpheusActor;
}

#endif
