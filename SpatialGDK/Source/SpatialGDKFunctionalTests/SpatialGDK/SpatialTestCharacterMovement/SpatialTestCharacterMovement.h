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

	virtual void PrepareTest() override;

	bool HasCharacterReachedDestination(ATestMovementCharacter* PlayerCharacter, const FPlane& DestinationPlane) const;
};
