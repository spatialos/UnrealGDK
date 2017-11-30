// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "GameFramework/Actor.h"
#include "SpatialShadowActor.generated.h"

UCLASS()
class ASpatialShadowActor : public AActor
{
	GENERATED_BODY()
public:
	ASpatialShadowActor();

	// Actor which this actor is "shadowing".
	UPROPERTY()
	TWeakObjectPtr<AActor> PairedActor;

	virtual void ReplicateChanges(float DeltaTime);
};

