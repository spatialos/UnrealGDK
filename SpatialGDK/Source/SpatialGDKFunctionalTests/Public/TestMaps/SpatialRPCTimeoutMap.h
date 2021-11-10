// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "TestMaps/GeneratedTestMap.h"
#include "SpatialRPCTimeoutMap.generated.h"

/**
 * This map is for use with SpatialTestRPCTimeout
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
