#include "ScavengersHubIteratorUtility.h"

#if 0

void AScavengersHubIteratorUtility::ActorChangedNetworkLevel(AMorpheusActor* MorpheusActor)
{
    if (MorpheusActor == nullptr)
    {
        return;
    }

    const EMorpheusNetworkLevel::Type Level = MorpheusActor->NetworkLevel;
    const worker::EntityId EntityId = MorpheusActor->EntityId;

    switch (Level)
    {
        case EMorpheusNetworkLevel::Foreground:
        {
            MorpheusActorsForeground.Add(EntityId, TWeakObjectPtr<AMorpheusActor>{MorpheusActor});
            MorpheusActorsMidground.Remove(EntityId);
            MorpheusActorsBackground.Remove(EntityId);
            break;
        }
        case EMorpheusNetworkLevel::Midground:
        {
            MorpheusActorsForeground.Add(EntityId, TWeakObjectPtr<AMorpheusActor>{MorpheusActor});
            MorpheusActorsMidground.Add(EntityId, TWeakObjectPtr<AMorpheusActor>{MorpheusActor});
            MorpheusActorsBackground.Remove(EntityId);
            break;
        }
        case EMorpheusNetworkLevel::Background:
        {
            MorpheusActorsForeground.Add(EntityId, TWeakObjectPtr<AMorpheusActor>{MorpheusActor});
            MorpheusActorsMidground.Add(EntityId, TWeakObjectPtr<AMorpheusActor>{MorpheusActor});
            MorpheusActorsBackground.Add(EntityId, TWeakObjectPtr<AMorpheusActor>{MorpheusActor});
            break;
        }
        default:
        {
            UE_LOG(LogScavengersHub, Error,
                TEXT("Invalid EMorpheusNetworkLevel passed to AScavengersHubIteratorUtilityr::ActorChangedNetworkLevel"));
            break;
        }
    }
}

void AScavengersHubIteratorUtility::RemoveActor(AMorpheusActor* MorpheusActor)
{
    if (MorpheusActor == nullptr)
    {
        return;
    }

    const worker::EntityId EntityId = MorpheusActor->EntityId;
    MorpheusActorsForeground.Remove(EntityId);
    MorpheusActorsMidground.Remove(EntityId);
    MorpheusActorsBackground.Remove(EntityId);
}

const AScavengersHubIteratorUtility::ActorMap& AScavengersHubIteratorUtility::GetActorsInNetworkLevel(
    EMorpheusNetworkLevel::Type Level) const
{
    switch (Level)
    {
        case EMorpheusNetworkLevel::Foreground:
        {
            return MorpheusActorsForeground;
        }
        case EMorpheusNetworkLevel::Midground:
        {
            return MorpheusActorsMidground;
        }
        case EMorpheusNetworkLevel::Background:
        {
            return MorpheusActorsBackground;
        }
        default:
        {
            UE_LOG(LogScavengersHub, Error,
                TEXT("Invalid EMorpheusNetworkLevel passed to AScavengersHubIteratorUtilityr::GetActorsInNetworkLevel; returning "
                     "Background actors."));
            break;
        }
    }

    return MorpheusActorsBackground;
}

AScavengersHubIteratorUtility* AScavengersHubIteratorUtility::GetInstance(UWorld* World)
{
    if (AScavengersHubIteratorUtility* Instance =
            Cast<AScavengersHubIteratorUtility>(UGameplayStatics::GetActorOfClass(World, StaticClass())))
    {
        return Instance;
    }

    return Cast<AScavengersHubIteratorUtility>(World->SpawnActor(StaticClass()));
}

#endif
