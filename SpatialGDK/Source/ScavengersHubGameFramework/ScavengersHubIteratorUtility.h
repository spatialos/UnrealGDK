#pragma once

#if 0

#include "ScavengersHubGDK/Common/MorpheusActor/MorpheusActor.h"

#include "ScavengersHubIteratorUtility.generated.h"

UCLASS()
class SCAVENGERSHUBGAMEFRAMEWORK_API AScavengersHubIteratorUtility : public AActor
{
    GENERATED_BODY()

public:
    typedef TMap<int64, TWeakObjectPtr<AMorpheusActor>> ActorMap;

    void ActorChangedNetworkLevel(AMorpheusActor* MorpheusActor);
    void RemoveActor(AMorpheusActor* MorpheusActor);

    const ActorMap& GetActorsInNetworkLevel(EMorpheusNetworkLevel::Type Level) const;

    // Singleton pattern - spawns the actor in the scene if it doesn't already exist
    static AScavengersHubIteratorUtility* GetInstance(UWorld* World);

private:
    ActorMap MorpheusActorsForeground;
    ActorMap MorpheusActorsMidground;
    ActorMap MorpheusActorsBackground;
};

#endif
