// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "SpatialFunctionalTest.h"
#include "SpatialTestWorldComposition.generated.h"

USTRUCT()
/**
 * Struct used to store the expected location and class of an Actor.
 */
struct FExpectedActor
{
	GENERATED_BODY()

	UPROPERTY()
	FVector ExpectedActorLocation;

	UPROPERTY()
	UClass* ExpectedActorClass;
};

UCLASS()
class SPATIALGDKFUNCTIONALTESTS_API ASpatialTestWorldComposition : public ASpatialFunctionalTest
{
	GENERATED_BODY()

public:
	ASpatialTestWorldComposition();

	virtual void PrepareTest() override;

	bool IsCorrectAtLocation(int TestLocation);

	// Helper array used to store all the references returned by GetAllActorsOfClass.
	TArray<AActor*> FoundReplicatedBaseActors;

	// Array storing the Pawn's testing locations and the Actors that must be present at every location for the test to pass.
	TArray<TPair<FVector, TArray<FExpectedActor>>> TestStepsData;

	// A reference to the Client's Pawn to avoid code duplication.
	APawn* ClientOnePawn;

	int TestLocationIndex;
};
