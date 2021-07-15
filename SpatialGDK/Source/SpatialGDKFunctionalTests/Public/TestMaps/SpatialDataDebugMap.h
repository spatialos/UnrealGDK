// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "TestMaps/GeneratedTestMap.h"
#include "SpatialDataDebugMap.generated.h"

/**
 * This map is a simple 2-server-worker map, where both workers see everything in the world.
 * It has the bEnableSpatialDataDebugger flag set to check for non-auth modification of data
 */
UCLASS()
class SPATIALGDKFUNCTIONALTESTS_API USpatialDataDebugMap : public UGeneratedTestMap
{
	GENERATED_BODY()

public:
	USpatialDataDebugMap();

protected:
	virtual void CreateCustomContentForMap() override;
};
