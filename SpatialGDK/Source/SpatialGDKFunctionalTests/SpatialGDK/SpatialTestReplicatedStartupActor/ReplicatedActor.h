// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ReplicatedActor.generated.h"

UCLASS()
class AReplicatedActor : public AActor
{
	GENERATED_BODY()
	
public:	
	AReplicatedActor();

	UPROPERTY()
	UStaticMeshComponent* CubeComponent;
};
