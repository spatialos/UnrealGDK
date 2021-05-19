// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "ReplicatedTestActorBase_RepGraphAlwaysReplicate.h"
#include "AlwaysInterestedTestActors.generated.h"

/**
 * A replicated actor with AlwaysInterested properties
 */
UCLASS()
class AAlwaysInterestedTestActor : public AReplicatedTestActorBase_RepGraphAlwaysReplicate
{
	GENERATED_BODY()

public:
	AAlwaysInterestedTestActor();

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	UPROPERTY(Replicated, AlwaysInterested)
	TArray<AActor*> InterestedActors;
};

/**
 * A replicated actor with a small NCD
 */
UCLASS()
class ASmallNCDActor : public AReplicatedTestActorBase_RepGraphAlwaysReplicate
{
	GENERATED_BODY()

public:
	ASmallNCDActor();
};
