// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ReplicatedTestActorBase.generated.h"

/**
 * A replicated Actor with a Cube Mesh, used as a base for Actors used in spatial tests.
 */
UCLASS()
class AReplicatedTestActorBase : public AActor
{
	GENERATED_BODY()

public:
	AReplicatedTestActorBase();

	UPROPERTY()
	UStaticMeshComponent* CubeComponent;
};
