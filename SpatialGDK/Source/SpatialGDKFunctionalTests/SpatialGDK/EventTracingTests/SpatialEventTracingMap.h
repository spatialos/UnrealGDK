// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "TestMaps/GeneratedTestMap.h"
#include "SpatialEventTracingMap.generated.h"

/**
 * Generated test maps for Event Tracing tests.
 */

UCLASS()
class SPATIALGDKFUNCTIONALTESTS_API USpatialEventTracingMap : public UGeneratedTestMap
{
	GENERATED_BODY()

public:
	USpatialEventTracingMap();

protected:
	virtual void CreateCustomContentForMap() override;
};
