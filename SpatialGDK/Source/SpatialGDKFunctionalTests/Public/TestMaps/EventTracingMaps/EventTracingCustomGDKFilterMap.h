// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "TestMaps/GeneratedTestMap.h"
#include "EventTracingCustomGDKFilterMap.generated.h"

UCLASS()
class SPATIALGDKFUNCTIONALTESTS_API UEventTracingCustomGDKFilterMap : public UGeneratedTestMap
{
	GENERATED_BODY()

public:
	UEventTracingCustomGDKFilterMap();

protected:
	virtual void CreateCustomContentForMap() override;
};
