// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "TestMaps/GeneratedTestMap.h"
#include "SpatialComponentMap.generated.h"

/**
 * This map is custom-made for the SpatialComponentTest - it utilizes a gamemode override.
 */
UCLASS()
class SPATIALGDKFUNCTIONALTESTS_API USpatialComponentMap : public UGeneratedTestMap
{
	GENERATED_BODY()

public:
	USpatialComponentMap();

protected:
	virtual void CreateCustomContentForMap() override;
};
