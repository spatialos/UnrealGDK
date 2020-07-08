// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "CubeWithReferences.generated.h"

UCLASS()
class ACubeWithReferences : public AActor
{
	GENERATED_BODY()

public:
	ACubeWithReferences();

	int CountValidNeighbours();

	void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	UPROPERTY()
	UStaticMeshComponent* CubeComponent;

	UPROPERTY(Replicated, EditAnywhere, Category="Neighbours")
	ACubeWithReferences* Neighbour1;

	UPROPERTY(Replicated, EditAnywhere, Category="Neighbours")
	ACubeWithReferences* Neighbour2;
};
