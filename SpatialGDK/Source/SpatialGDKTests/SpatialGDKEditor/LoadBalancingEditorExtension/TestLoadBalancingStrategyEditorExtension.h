// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "EditorExtension/LBStrategyEditorExtension.h"
#include "SpatialRuntimeLoadBalancingStrategies.h"
#include "TestLoadBalancingStrategy.h"

class FTestLBStrategyEditorExtension : public FLBStrategyEditorExtensionTemplate<UDummyLoadBalancingStrategy, FTestLBStrategyEditorExtension>
{
public:
	bool GetDefaultLaunchConfiguration(const UDummyLoadBalancingStrategy* Strategy, UAbstractRuntimeLoadBalancingStrategy*& OutConfiguration, FIntPoint& OutWorldDimensions) const
	{
		if (Strategy == nullptr)
		{
			return false;
		}

		UEntityShardingRuntimeLoadBalancingStrategy* Conf = NewObject<UEntityShardingRuntimeLoadBalancingStrategy>();
		Conf->NumWorkers = Strategy->NumberOfWorkers;
		OutConfiguration = Conf;

		OutWorldDimensions.X = OutWorldDimensions.Y = 0;

		return true;
	}
};

class FTestDerivedLBStrategyEditorExtension : public FLBStrategyEditorExtensionTemplate<UDerivedDummyLoadBalancingStrategy, FTestDerivedLBStrategyEditorExtension>
{
public:

	bool GetDefaultLaunchConfiguration(const UDerivedDummyLoadBalancingStrategy* Strategy, UAbstractRuntimeLoadBalancingStrategy*& OutConfiguration, FIntPoint& OutWorldDimensions) const
	{
		if (Strategy == nullptr)
		{
			return false;
		}

		UEntityShardingRuntimeLoadBalancingStrategy* Conf = NewObject<UEntityShardingRuntimeLoadBalancingStrategy>();
		Conf->NumWorkers = Strategy->NumberOfWorkers;
		OutConfiguration = Conf;

		OutWorldDimensions.X = OutWorldDimensions.Y = 4242;

		return true;
	}
};
