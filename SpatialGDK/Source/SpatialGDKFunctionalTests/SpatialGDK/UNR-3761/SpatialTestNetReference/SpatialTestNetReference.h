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

	virtual void FinishTest(EFunctionalTestResult TestResult, const FString& Message) override;

	virtual void BeginPlay() override;

	// Array used to store the locations in which the character will perform the references check and the number of cubes that should be
	// visible at that location
	TArray<TPair<FVector, int>> TestLocations;

	// Helper array used to store the relative locations of the camera, so that it can see all cubes from every test location, used for
	// visual debugging
	TArray<FVector> CameraRelativeLocations;

	// Helper rotator used to store the relative rotation of the camera so that it can see all cubes from every test location, used for
	// visual debugging
	FRotator CameraRelativeRotation;

	TPair<AController*, APawn*> OriginalPawn;

	float PreviousPositionUpdateFrequency;
};
