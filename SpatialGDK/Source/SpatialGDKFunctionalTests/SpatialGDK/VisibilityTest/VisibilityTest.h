// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "SpatialFunctionalTest.h"
#include "VisibilityTest.generated.h"

UCLASS()
class SPATIALGDKFUNCTIONALTESTS_API AVisibilityTest : public ASpatialFunctionalTest
{
	GENERATED_BODY()
public:
	AVisibilityTest();

	virtual void BeginPlay() override;

	// To save original Pawns and possess them back at the end
	TArray<TPair<AController*, APawn*>> OriginalPawns;

	float PreviousPositionUpdateFrequency;
	FVector CharacterRemoteLocation;
	FVector Character1StartingLocation;
	FVector Character2StartingLocation;
};
