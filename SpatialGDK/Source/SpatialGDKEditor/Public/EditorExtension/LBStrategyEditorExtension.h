// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "CoreMinimal.h"

class UAbstractLBStrategy;
class FLBStrategyEditorExtensionManager;
struct FWorkerTypeLaunchSection;

class FLBStrategyEditorExtensionInterface
{
private:
	friend FLBStrategyEditorExtensionManager;
	virtual bool GetDefaultLaunchConfiguration_Virtual(const UAbstractLBStrategy* Strategy, FWorkerTypeLaunchSection& OutConfiguration, FIntPoint& OutWorldDimensions) const = 0;
};

template <typename StrategyImpl, typename Implementation>
class FLBStrategyEditorExtensionTemplate : public FLBStrategyEditorExtensionInterface
{
public:
	using ExtendedStrategy = StrategyImpl;

private:
	bool GetDefaultLaunchConfiguration_Virtual(const UAbstractLBStrategy* Strategy, FWorkerTypeLaunchSection& OutConfiguration, FIntPoint& OutWorldDimensions) const override
	{
		return static_cast<const Implementation*>(this)->GetDefaultLaunchConfiguration(static_cast<const StrategyImpl*>(Strategy), OutConfiguration, OutWorldDimensions);
	}
};

class FLBStrategyEditorExtensionManager
{
public:
	SPATIALGDKEDITOR_API bool GetDefaultLaunchConfiguration(const UAbstractLBStrategy* Strategy, FWorkerTypeLaunchSection& OutConfiguration, FIntPoint& OutWorldDimensions) const;

	template <typename Extension>
	void RegisterExtension()
	{
		RegisterExtension(Extension::ExtendedStrategy::StaticClass(), MakeUnique<Extension>());
	}

	void Cleanup();

private:
	void RegisterExtension(UClass* StrategyClass, TUniquePtr<FLBStrategyEditorExtensionInterface> StrategyExtension);

	using ExtensionArray = TArray<TPair<UClass*, TUniquePtr<FLBStrategyEditorExtensionInterface>>>;

	ExtensionArray Extensions;
};
