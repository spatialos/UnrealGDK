// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "SpatialFunctionalTest.h"
#include "SpatialTestCharacterMovement.generated.h"

class ATestMovementCharacter;
UCLASS()
class ASpatialTestCharacterMovement : public ASpatialFunctionalTest
{
	GENERATED_BODY()
	
public:	
	ASpatialTestCharacterMovement();

	virtual void BeginPlay() override;

	bool bCharacterReachedDestination;

	UFUNCTION()
	void OnOverlapBegin(AActor* OverlappedActor, AActor* OtherActor);

	// Helper variable used to wait for a certain amount of time before performing an action
	float ElapsedTime;
};
