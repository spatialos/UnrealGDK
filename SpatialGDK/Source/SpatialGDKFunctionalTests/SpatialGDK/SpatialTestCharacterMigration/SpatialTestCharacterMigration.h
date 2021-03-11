// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "SpatialFunctionalTest.h"
#include "TestMaps/GeneratedTestMap.h"

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

UCLASS()
class SPATIALGDKFUNCTIONALTESTS_API USpatialTestCharacterMigrationMap : public UGeneratedTestMap
{
	GENERATED_BODY()

public:
	USpatialTestCharacterMigrationMap();

protected:
	virtual void CreateCustomContentForMap() override;
};
