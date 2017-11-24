// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "GameFramework/Actor.h"
#include "SpatialShadowActor.generated.h"

class USpatialActorChannel;

UCLASS()
class ASpatialShadowActor : public AActor
{
	GENERATED_BODY()
public:
	ASpatialShadowActor();

	// Actor which this actor is "shadowing".
	UPROPERTY()
	TWeakObjectPtr<AActor> PairedActor;

	// Actor channel on clients which have a single UConnection to the "server".
	UPROPERTY()
	USpatialActorChannel* ClientActorChannel;

	virtual void ReplicateChanges(float DeltaTime);
};

