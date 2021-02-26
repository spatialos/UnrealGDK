// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "TestMaps/GeneratedTestMap.h"
#include "EventTracingMap.generated.h"

/**
 * Generated test maps for Event Tracing tests.
 */

UCLASS()
class SPATIALGDKFUNCTIONALTESTS_API UEventTracingMap : public UGeneratedTestMap
{
	GENERATED_BODY()

public:
	UEventTracingMap();

protected:
	virtual void CreateCustomContentForMap() override;
};
