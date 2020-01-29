// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "SpatialCommonTypes.h"
#include "LoadBalancing/AbstractLBStrategy.h"
#include "LBStrategySpy.generated.h"

/**
 * This class is for testing purposes only.
 */
UCLASS(HideDropdown)
class SPATIALGDKTESTS_API ULBStrategySpy : public UAbstractLBStrategy
{
	GENERATED_BODY()

public:
	ULBStrategySpy() = default;

	VirtualWorkerId GetVirtualWorkerId() const
	{
		return LocalVirtualWorkerId;
	}
};
