// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "TestMaps/GeneratedTestMap.h"
#include "SpatialRPCTimeoutMap.generated.h"

/**
 * This map is mostly for use with SpatialTestPropertyReplication.
 * It is a simple example map that demonstrates how to create a map for a test.
 */
UCLASS()
class SPATIALGDKFUNCTIONALTESTS_API USpatialRPCTimeoutMap : public UGeneratedTestMap
{
	GENERATED_BODY()

public:
	USpatialRPCTimeoutMap();

protected:
	virtual void CreateCustomContentForMap() override;
};
