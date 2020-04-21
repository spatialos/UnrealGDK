// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "EditorExtension/LBStrategyEditorExtension.h"
#include "TestLoadBalancingStrategy.h"

class FTestLBStrategyEditorExtension : public FLBStrategyEditorExtensionTemplate<UDummyLoadBalancingStrategy, FTestLBStrategyEditorExtension>
{
public:
	bool GetDefaultLaunchConfiguration(const UDummyLoadBalancingStrategy* Strategy, FWorkerTypeLaunchSection& OutConfiguration, FIntPoint& OutWorldDimensions) const
	{
		if (!Strategy)
		{
			return false;
		}

		OutConfiguration.NumEditorInstances = Strategy->NumberOfWorkers;

		OutWorldDimensions.X = OutWorldDimensions.Y = 0;

		return true;
	}
};

class FTestDerivedLBStrategyEditorExtension : public FLBStrategyEditorExtensionTemplate<UDerivedDummyLoadBalancingStrategy, FTestDerivedLBStrategyEditorExtension>
{
public:

	bool GetDefaultLaunchConfiguration(const UDerivedDummyLoadBalancingStrategy* Strategy, FWorkerTypeLaunchSection& OutConfiguration, FIntPoint& OutWorldDimensions) const
	{
		if (!Strategy)
		{
			return false;
		}

		OutConfiguration.NumEditorInstances = Strategy->NumberOfWorkers;

		OutWorldDimensions.X = OutWorldDimensions.Y = 4242;

		return true;
	}
};
