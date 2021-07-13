// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "TestMaps/GeneratedTestMap.h"
#include "Spatial2WorkerMigrationMaps.generated.h"

/**
 * This map is a simple 2-server-worker map, where both workers do not have interest outside their regions.
 */

UCLASS()
class SPATIALGDKFUNCTIONALTESTS_API USpatial2WorkerFullInterestMigrationMap : public UGeneratedTestMap
{
	GENERATED_BODY()

public:
	USpatial2WorkerFullInterestMigrationMap();

protected:
	virtual void CreateCustomContentForMap() override;
};

UCLASS()
class SPATIALGDKFUNCTIONALTESTS_API USpatial2WorkerNoInterestMigrationMap : public UGeneratedTestMap
{
	GENERATED_BODY()

public:
	USpatial2WorkerNoInterestMigrationMap();

protected:
	virtual void CreateCustomContentForMap() override;
};
