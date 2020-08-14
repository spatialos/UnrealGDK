// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "SpatialFunctionalTest.h"
#include "SpatialTestWorldComposition.generated.h"

UCLASS()
class SPATIALGDKFUNCTIONALTESTS_API ASpatialTestWorldComposition : public ASpatialFunctionalTest
{
	GENERATED_BODY()

public:
	ASpatialTestWorldComposition();

	virtual void BeginPlay() override;

	bool bIsCorrectAtLocation(int TestLocationIndex);

	// Helper array used to store all the references returned by GetAllActorsOfClass.
	TArray<AActor*> FoundReplicatedBaseActors;

	// The locations that the Pawn will move to to perform the level loading/unloading checks.
	TArray<FVector> TestLocations;

	// The locations that the Actors placed in the sub-levels have, used to validate that the level was correctly loaded.
	TArray<FVector> ActorsLocations;

	// A reference to the Client's Pawn to avoid code duplication.
	APawn* ClientOnePawn;

	int TestLocationIndex;
};
