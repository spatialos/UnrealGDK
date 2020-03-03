#pragma once

#include "CoreMinimal.h"

class UAbstractLBStrategy;
struct FWorkerTypeLaunchSection;

class FLBStrategyEditorExtensionInterface
{
private:
	friend class FLBStrategyEditorExtensionManager;
	virtual bool GetDefaultLaunchConfiguration(const UAbstractLBStrategy* Strategy, FWorkerTypeLaunchSection& OutConfiguration, FIntPoint& OutWorldDimensions) const = 0;
};

template <typename StrategyImpl, typename Implementation>
class FLBStrategyEditorExtensionTemplate : public FLBStrategyEditorExtensionInterface
{
public:
	typedef StrategyImpl ExtendedStrategy;
private:

	bool GetDefaultLaunchConfiguration(const UAbstractLBStrategy* Strategy, FWorkerTypeLaunchSection& OutConfiguration, FIntPoint& OutWorldDimensions) const override
	{
		return static_cast<const Implementation*>(this)->GetDefaultLaunchConfiguration(static_cast<const StrategyImpl*>(Strategy), OutConfiguration, OutWorldDimensions);
	}
};

class FLBStrategyEditorExtensionManager
{
public:
	static bool GetDefaultLaunchConfiguration(const UAbstractLBStrategy* Strategy, FWorkerTypeLaunchSection& OutConfiguration, FIntPoint& OutWorldDimensions);

	template <typename Extension>
	static void RegisterExtension()
	{
		RegisterExtension(Extension::ExtendedStrategy::StaticClass(), new Extension);
	}

private:

	static void RegisterExtension(UClass* StrategyClass, FLBStrategyEditorExtensionInterface* StrategyExtension)
	{
		ExtensionMap.Add(StrategyClass, StrategyExtension);
	}

	static TMap<UClass*, FLBStrategyEditorExtensionInterface*> ExtensionMap;
};
