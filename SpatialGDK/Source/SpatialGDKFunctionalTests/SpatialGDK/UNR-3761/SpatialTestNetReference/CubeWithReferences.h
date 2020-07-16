// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "ReplicatedTestActorBase.h"
#include "CubeWithReferences.generated.h"

UCLASS()
class ACubeWithReferences : public AReplicatedTestActorBase
{
	GENERATED_BODY()

public:
	ACubeWithReferences();

	int CountValidNeighbours();

	void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	UPROPERTY(Replicated)
	ACubeWithReferences* Neighbour1;

	UPROPERTY(Replicated)
	ACubeWithReferences* Neighbour2;
};
