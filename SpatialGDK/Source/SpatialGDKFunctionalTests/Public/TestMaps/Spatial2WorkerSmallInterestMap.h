// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "TestMaps/GeneratedTestMap.h"
#include "Spatial2WorkerSmallInterestMap.generated.h"

/**
 * This map is a simple 2-server-worker map, where both workers see only a tiny bit (150 units) outside of their authoritative zone.
 */
// meta comment ^ not sure how useful it is to include the interest radius here, as this is basically duplicating information... could just
// say small radius and refer to the implementation for the actual value
UCLASS()
class SPATIALGDKFUNCTIONALTESTS_API USpatial2WorkerSmallInterestMap : public UGeneratedTestMap
{
	GENERATED_BODY()

public:
	USpatial2WorkerSmallInterestMap();

protected:
	virtual void CreateCustomContentForMap() override;
};
