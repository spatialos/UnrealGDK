// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "TestMaps/GeneratedTestMap.h"
#include "Spatial2WorkerSmallInterestMap.generated.h"

/**
 * This map is a simple 2-server-worker map, where both workers see only a tiny bit (150 units) outside of their authoritative zone (see
 * SpatialTest1x2GridSmallInterestStrategy).
 */
UCLASS()
class SPATIALGDKFUNCTIONALTESTS_API USpatial2WorkerSmallInterestMap : public UGeneratedTestMap
{
	GENERATED_BODY()

public:
	USpatial2WorkerSmallInterestMap();

protected:
	virtual void CreateCustomContentForMap() override;
};
