#pragma once

#include "CoreMinimal.h"

class UAbstractLBStrategy;
struct FWorkerTypeLaunchSection;

class FLBStrategyEditorExtensionInterface
{
private:
	friend class FLBStrategyEditorExtensionManager;
	virtual bool GetDefaultLaunchConfiguration_Virtual(const UAbstractLBStrategy* Strategy, FWorkerTypeLaunchSection& OutConfiguration, FIntPoint& OutWorldDimensions) const = 0;
};

template <typename StrategyImpl, typename Implementation>
class FLBStrategyEditorExtensionTemplate : public FLBStrategyEditorExtensionInterface
{
public:
	typedef StrategyImpl ExtendedStrategy;
private:

	bool GetDefaultLaunchConfiguration_Virtual(const UAbstractLBStrategy* Strategy, FWorkerTypeLaunchSection& OutConfiguration, FIntPoint& OutWorldDimensions) const override
	{
		return static_cast<const Implementation*>(this)->GetDefaultLaunchConfiguration(static_cast<const StrategyImpl*>(Strategy), OutConfiguration, OutWorldDimensions);
	}
};

class FLBStrategyEditorExtensionManager
{
public:
	SPATIALGDKEDITOR_API bool GetDefaultLaunchConfiguration(const UAbstractLBStrategy* Strategy, FWorkerTypeLaunchSection& OutConfiguration, FIntPoint& OutWorldDimensions);

	template <typename Extension>
	void RegisterExtension()
	{
		RegisterExtension(Extension::ExtendedStrategy::StaticClass(), new Extension);
	}

	void Cleanup();

private:

	void RegisterExtension(UClass* StrategyClass, FLBStrategyEditorExtensionInterface* StrategyExtension);

	TArray<TPair<UClass*, TUniquePtr<FLBStrategyEditorExtensionInterface>>> ExtensionMap;
};
