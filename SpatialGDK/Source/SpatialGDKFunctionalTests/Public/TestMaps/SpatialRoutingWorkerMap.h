// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "TestMaps/GeneratedTestMap.h"
#include "SpatialRoutingWorkerMap.generated.h"

/**
 * This map is a simple 2-server-worker map, where both workers see everything in the world.
 */
UCLASS()
class SPATIALGDKFUNCTIONALTESTS_API USpatialRoutingWorkerMap : public UGeneratedTestMap
{
	GENERATED_BODY()

public:
	USpatialRoutingWorkerMap();

protected:
	virtual void CreateCustomContentForMap() override;
};
