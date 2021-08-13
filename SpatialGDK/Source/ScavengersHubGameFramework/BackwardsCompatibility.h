#pragma once

#if 0

#include "ScavengersHubGDK/Common/MorpheusActor/MorpheusActor.h"

#include "BackwardsCompatibility.generated.h"

UCLASS(Blueprintable, BlueprintType)
class SCAVENGERSHUBGAMEFRAMEWORK_API AEntityObject : public AMorpheusActor
{
    GENERATED_BODY()
};

UCLASS(Blueprintable, BlueprintType)
class SCAVENGERSHUBGAMEFRAMEWORK_API UEntityObjectRenderTargetComponent : public UMorpheusActorRenderTargetComponent
{
    GENERATED_BODY()
};

UCLASS(Blueprintable, BlueprintType)
class SCAVENGERSHUBGAMEFRAMEWORK_API UEntityObjectMovementComponent : public UMorpheusActorMovementComponent
{
    GENERATED_BODY()
};

UCLASS(Blueprintable, BlueprintType)
class SCAVENGERSHUBGAMEFRAMEWORK_API UEntityObjectComponent : public UMorpheusRenderTargetOwnerComponent
{
    GENERATED_BODY()
};

#endif
