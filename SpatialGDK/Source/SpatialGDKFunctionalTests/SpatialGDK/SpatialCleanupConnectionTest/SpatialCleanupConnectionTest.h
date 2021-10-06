// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "EngineClasses/SpatialNetDriver.h"
#include "SpatialFunctionalTest.h"
#include "SpatialGDKFunctionalTests/SpatialGDK/TestActors/TestMovementCharacter.h"
#include "SpatialCleanupConnectionTest.generated.h"

UCLASS()
class SPATIALGDKFUNCTIONALTESTS_API ASpatialCleanupConnectionTest : public ASpatialFunctionalTest
{
	GENERATED_BODY()

public:
	ASpatialCleanupConnectionTest();

	virtual void PrepareTest() override;

	// This needs to be a position that belongs to Server 1.
	UPROPERTY(EditAnywhere, Category = "Default")
	FVector Server1Position;

	// This needs to be a position that belongs to Server 2.
	UPROPERTY(EditAnywhere, Category = "Default")
	FVector Server2Position;

	// This needs to be a position that belongs to Server 1 and is in the interest border of Server 2.
	UPROPERTY(EditAnywhere, Category = "Default")
	FVector Server1PositionAndInInterestBorderServer2;

	UPROPERTY()
	APawn* DefaultPawn; // Keep track of the original pawn, so  we can possess it when we cleanup and that other tests start from the
						// expected, default set-up

	UPROPERTY()
	ATestMovementCharacter* SpawnedPawn;

	UPROPERTY()
	APlayerController* PlayerController;
};
