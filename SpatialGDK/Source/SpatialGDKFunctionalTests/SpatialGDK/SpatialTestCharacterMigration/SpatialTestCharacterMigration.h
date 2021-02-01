// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "SpatialFunctionalTest.h"
#include "SpatialTestCharacterMigration.generated.h"

UCLASS()
class ASpatialTestCharacterMigration : public ASpatialFunctionalTest
{
	GENERATED_BODY()

public:
	ASpatialTestCharacterMigration();

	virtual void PrepareTest() override;

	FVector Origin;
	FVector Destination;

	bool bCharacterReachedDestination;
	bool bCharacterReachedOrigin;
};
