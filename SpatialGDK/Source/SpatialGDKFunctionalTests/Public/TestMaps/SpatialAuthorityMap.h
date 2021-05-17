// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "TestMaps/GeneratedTestMap.h"
#include "SpatialAuthorityMap.generated.h"

/**
 * This map is custom-made for the SpatialAuthorityTest - it utilizes a gamemode override.
 */
UCLASS()
class SPATIALGDKFUNCTIONALTESTS_API USpatialAuthorityMap : public UGeneratedTestMap
{
	GENERATED_BODY()

public:
	USpatialAuthorityMap();

protected:
	virtual void CreateCustomContentForMap() override;
};
