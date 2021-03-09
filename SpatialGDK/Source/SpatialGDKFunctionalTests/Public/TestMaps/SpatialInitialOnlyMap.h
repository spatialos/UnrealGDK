// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "TestMaps/GeneratedTestMap.h"
#include "SpatialInitialOnlyMap.generated.h"

/**
 * This map is custom-made for the SpatialTestInitialOnly* tests.
 */
UCLASS()
class SPATIALGDKFUNCTIONALTESTS_API USpatialInitialOnlyMap : public UGeneratedTestMap
{
	GENERATED_BODY()

public:
	USpatialInitialOnlyMap();

protected:
	virtual void CreateCustomContentForMap() override;
};
