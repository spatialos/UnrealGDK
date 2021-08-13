// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#if 0

#include "CoreMinimal.h"
#include "ScavengersHubGDK/Common/MorpheusActor/MorpheusActor.h"
#include "UObject/Interface.h"

#include "ScavengersHubPooledActor.generated.h"

USTRUCT()
struct FEntityActorClassPair
{
    GENERATED_BODY()

    UPROPERTY()
    UClass* MorpheusActorClass;

    UPROPERTY()
    UClass* ActorClass;

    FEntityActorClassPair()
    {
        MorpheusActorClass = nullptr;
        ActorClass = nullptr;
    }

    FEntityActorClassPair(TSubclassOf<AMorpheusActor> MorpheusActorClass, UClass* ActorClass)
        : MorpheusActorClass(MorpheusActorClass), ActorClass(ActorClass)
    {
    }

    bool operator==(const FEntityActorClassPair& Other) const
    {
        return MorpheusActorClass == Other.MorpheusActorClass && ActorClass == Other.ActorClass;
    }

    friend uint32 GetTypeHash(const FEntityActorClassPair& Struct)
    {
        return HashCombine(GetTypeHash(Struct.MorpheusActorClass), GetTypeHash(Struct.ActorClass));
    }
};

UINTERFACE(BlueprintType)
class SCAVENGERSHUBGAMEFRAMEWORK_API UScavengersHubPooledActor : public UInterface
{
    GENERATED_BODY()
};

class SCAVENGERSHUBGAMEFRAMEWORK_API IScavengersHubPooledActor
{
    GENERATED_BODY()
public:
    UFUNCTION(BlueprintNativeEvent, Category = "Pooling")
    void OnBeginPlayFromPool();

    UFUNCTION(BlueprintNativeEvent, Category = "Pooling")
    void OnEndPlayToPool();
};

#endif
