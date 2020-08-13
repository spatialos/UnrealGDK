// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "SpatialFunctionalTest.h"
#include "SpatialTestCharacterMovement.generated.h"

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
};
