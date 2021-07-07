// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "TestMaps/GeneratedTestMap.h"
#include "SpatialInitialDormancyMap.generated.h"

/**
 * This map is mostly for tests that require a DORM_Initial actor to be present in the map
 */
UCLASS()
class SPATIALGDKFUNCTIONALTESTS_API USpatialInitialDormancyMap : public UGeneratedTestMap
{
	GENERATED_BODY()

public:
	USpatialInitialDormancyMap();

protected:
	virtual void CreateCustomContentForMap() override;
};
