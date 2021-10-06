// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "TestMaps/GeneratedTestMap.h"
#include "SpatialPropertyReplicationMap.generated.h"

/**
 * This map is mostly for use with SpatialTestPropertyReplication.
 * It is a simple example map that demonstrates how to create a map for a test.
 */
UCLASS()
class SPATIALGDKFUNCTIONALTESTS_API USpatialPropertyReplicationMap : public UGeneratedTestMap
{
	GENERATED_BODY()

public:
	USpatialPropertyReplicationMap();

protected:
	virtual void CreateCustomContentForMap() override;
};
