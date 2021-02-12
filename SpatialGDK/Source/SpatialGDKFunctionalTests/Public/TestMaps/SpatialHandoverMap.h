// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "TestMaps/GeneratedTestMap.h"
#include "SpatialHandoverMap.generated.h"

/**
 * Custom made for the SpatialTestHandover, placed in the slow category, due to low reliability when re-running.
 */
UCLASS()
class SPATIALGDKFUNCTIONALTESTS_API USpatialHandoverMap : public UGeneratedTestMap
{
	GENERATED_BODY()

public:
	USpatialHandoverMap();

protected:
	virtual void CreateCustomContentForMap() override;
};
