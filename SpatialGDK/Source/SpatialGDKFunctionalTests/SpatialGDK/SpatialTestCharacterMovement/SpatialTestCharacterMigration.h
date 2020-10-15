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

	bool bCharacterReachedDestination;
	bool bCharacterReachedOrigin;

	UFUNCTION()
	void OnOverlapBeginDestination(AActor* OverlappedActor, AActor* OtherActor);
	UFUNCTION()
	void OnOverlapBeginOrigin(AActor* OverlappedActor, AActor* OtherActor);
};
