#pragma once

#include "EditorExtension/LBStrategyEditorExtension.h"
#include "LoadBalancing/GridBasedLBStrategy.h"

class FGridLBStrategyEditorExtension : public FLBStrategyEditorExtensionTemplate<UGridBasedLBStrategy, FGridLBStrategyEditorExtension>
{
public:
	bool GetDefaultLaunchConfiguration(const UGridBasedLBStrategy* Strategy, FWorkerTypeLaunchSection& OutConfiguration, FIntPoint& OutWorldDimensions) const;
};
