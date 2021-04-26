// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "ReplicatedTestActorBase.h"
#include "AlwaysInterestedTestActors.generated.h"

/**
 * A replicated actor with AlwaysInterested properties
 */
UCLASS()
class AAlwaysInterestedTestActor : public AReplicatedTestActorBase
{
	GENERATED_BODY()

public:
	AAlwaysInterestedTestActor();

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	UPROPERTY(Replicated, AlwaysInterested)
	TArray<AActor*> InterestedActors;
};
