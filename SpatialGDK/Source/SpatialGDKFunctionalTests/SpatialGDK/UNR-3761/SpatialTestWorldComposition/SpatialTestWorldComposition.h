// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "SpatialFunctionalTest.h"
#include "SpatialTestWorldComposition.generated.h"

class ATestMovementCharacter;
UCLASS()
class SPATIALGDKFUNCTIONALTESTS_API ASpatialTestWorldComposition : public ASpatialFunctionalTest
{
	GENERATED_BODY()

public:
	ASpatialTestWorldComposition();

	virtual void BeginPlay() override;

	TArray<AActor*> FoundReplicatedBaseActors;

	TArray<FVector> TestLocations;

	TArray<FVector> ActorsLocations;

	// Reference to Client1's Pawn
	APawn* ClientOnePawn;

	int TestLocationIndex;
};
