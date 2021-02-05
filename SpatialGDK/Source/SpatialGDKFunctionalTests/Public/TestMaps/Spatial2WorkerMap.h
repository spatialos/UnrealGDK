// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "TestMaps/GeneratedTestMap.h"
#include "Spatial2WorkerMap.generated.h"

/**
 * This map is a simple 2-server-worker map, where both workers see everything in the world.
 */
UCLASS()
class SPATIALGDKFUNCTIONALTESTS_API USpatial2WorkerMap : public UGeneratedTestMap
{
	GENERATED_BODY()

public:
	USpatial2WorkerMap();

protected:
	virtual void CreateCustomContentForMap() override;
};
