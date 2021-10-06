// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "TestMaps/GeneratedTestMap.h"
#include "Spatial4WorkerMap.generated.h"

/**
 * This map is a simple 4-server-worker map (2x2, so quadrants), where all workers see everything in the world.
 */
UCLASS()
class SPATIALGDKFUNCTIONALTESTS_API USpatial4WorkerMap : public UGeneratedTestMap
{
	GENERATED_BODY()

public:
	USpatial4WorkerMap();

protected:
	virtual void CreateCustomContentForMap() override;
};
