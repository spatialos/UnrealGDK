// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "SpatialFunctionalTest.h"
#include "SpatialTestNetReference.generated.h"

UCLASS()
class SPATIALGDKFUNCTIONALTESTS_API ASpatialTestNetReference : public ASpatialFunctionalTest
{
	GENERATED_BODY()

public:
	ASpatialTestNetReference();

	virtual void BeginPlay() override;

	// Array used to store the locations in which the character will perform the references check and the number of cubes that should be visible at that location
	TArray<TPair<FVector, int>> TestLocations;

	// Helper variable used as an index in the CharacterTestLocations array.
	int LocationIndex;

	// Helper variable used to wait for some time before performing an action
	float TimerHelper;

	TPair<AController*, APawn*> OriginalPawn;

	float PreviousPositionUpdateFrequency;
};
