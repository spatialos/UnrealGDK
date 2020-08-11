// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "LoadBalancing/AbstractLBStrategy.h"
#include "SpatialCommonTypes.h"

#include "LBStrategyStub.generated.h"

/**
 * This class is for testing purposes only.
 */
UCLASS(HideDropdown)
class SPATIALGDKTESTS_API ULBStrategyStub : public UAbstractLBStrategy
{
	GENERATED_BODY()

public:
	VirtualWorkerId GetVirtualWorkerId() const { return LocalVirtualWorkerId; }
};
