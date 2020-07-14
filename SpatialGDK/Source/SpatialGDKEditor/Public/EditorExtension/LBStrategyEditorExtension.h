// // Copyright (c) Improbable Worlds Ltd, All Rights Reserved
//
// #pragma once
//
// #include "CoreMinimal.h"
//
// DECLARE_LOG_CATEGORY_EXTERN(LogSpatialGDKEditorLBExtension, Log, All);
//
// class UAbstractLBStrategy;
// class FLBStrategyEditorExtensionManager;
// class UAbstractRuntimeLoadBalancingStrategy;
// struct FWorkerTypeLaunchSection;
//
// class FLBStrategyEditorExtensionInterface
// {
// public:
// 	virtual ~FLBStrategyEditorExtensionInterface() {}
// private:
// 	friend FLBStrategyEditorExtensionManager;
// 	virtual bool GetDefaultLaunchConfiguration_Virtual(const UAbstractLBStrategy* Strategy, UAbstractRuntimeLoadBalancingStrategy*& OutConfiguration, FIntPoint& OutWorldDimensions) const = 0;
// };
//
// template <typename StrategyImpl, typename Implementation>
// class FLBStrategyEditorExtensionTemplate : public FLBStrategyEditorExtensionInterface
// {
// public:
// 	using ExtendedStrategy = StrategyImpl;
//
// private:
// 	bool GetDefaultLaunchConfiguration_Virtual(const UAbstractLBStrategy* Strategy, UAbstractRuntimeLoadBalancingStrategy*& OutConfiguration, FIntPoint& OutWorldDimensions) const override
// 	{
// 		return static_cast<const Implementation*>(this)->GetDefaultLaunchConfiguration(static_cast<const StrategyImpl*>(Strategy), OutConfiguration, OutWorldDimensions);
// 	}
// };
//
// class FLBStrategyEditorExtensionManager
// {
// public:
// 	SPATIALGDKEDITOR_API bool GetDefaultLaunchConfiguration(const UAbstractLBStrategy* Strategy, UAbstractRuntimeLoadBalancingStrategy*& OutConfiguration, FIntPoint& OutWorldDimensions) const;
//
// 	template <typename Extension>
// 	void RegisterExtension()
// 	{
// 		RegisterExtension(Extension::ExtendedStrategy::StaticClass(), MakeUnique<Extension>());
// 	}
//
// 	template <typename Extension>
// 	void UnregisterExtension()
// 	{
// 		UnregisterExtension(Extension::ExtendedStrategy::StaticClass());
// 	}
//
// 	void Cleanup();
//
// private:
// 	SPATIALGDKEDITOR_API void RegisterExtension(UClass* StrategyClass, TUniquePtr<FLBStrategyEditorExtensionInterface> StrategyExtension);
//
// 	SPATIALGDKEDITOR_API void UnregisterExtension(UClass* StrategyClass);
//
// 	using ExtensionArray = TMap<UClass*, TUniquePtr<FLBStrategyEditorExtensionInterface>>;
//
// 	ExtensionArray Extensions;
// };
