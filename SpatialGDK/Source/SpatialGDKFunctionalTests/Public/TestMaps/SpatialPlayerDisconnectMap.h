// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "TestMaps/GeneratedTestMap.h"
#include "SpatialPlayerDisconnectMap.generated.h"

/**
 * This map is custom-made for the SpatialPlayerDisconnectTest - it utilizes a gamemode override.
 */
UCLASS()
class SPATIALGDKFUNCTIONALTESTS_API USpatialPlayerDisconnectMap : public UGeneratedTestMap
{
	GENERATED_BODY()

public:
	USpatialPlayerDisconnectMap();

protected:
	virtual void CreateCustomContentForMap() override;
};
