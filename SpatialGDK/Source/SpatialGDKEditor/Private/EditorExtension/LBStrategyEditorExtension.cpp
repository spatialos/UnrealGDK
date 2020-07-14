// // Copyright (c) Improbable Worlds Ltd, All Rights Reserved
//
// #include "EditorExtension/LBStrategyEditorExtension.h"
// #include "LoadBalancing/AbstractLBStrategy.h"
//
// DEFINE_LOG_CATEGORY(LogSpatialGDKEditorLBExtension);
//
// namespace
// {
//
// bool InheritFromClosest(UClass* Derived, UClass* PotentialBase, uint32& InOutPreviousDistance)
// {
// 	uint32 InheritanceDistance = 0;
// 	for (const UStruct* TempStruct = Derived; TempStruct != nullptr; TempStruct = TempStruct->GetSuperStruct())
// 	{
// 		if (TempStruct == PotentialBase)
// 		{
// 			InOutPreviousDistance = InheritanceDistance;
// 			return true;
// 		}
// 		++InheritanceDistance;
// 		if (InheritanceDistance > InOutPreviousDistance)
// 		{
// 			return false;
// 		}
// 	}
// 	return false;
// }
//
// } // anonymous namespace
//
// bool FLBStrategyEditorExtensionManager::GetDefaultLaunchConfiguration(const UAbstractLBStrategy* Strategy, UAbstractRuntimeLoadBalancingStrategy*& OutConfiguration, FIntPoint& OutWorldDimensions) const
// {
// 	if (!Strategy)
// 	{
// 		return false;
// 	}
//
// 	UClass* StrategyClass = Strategy->GetClass();
//
// 	FLBStrategyEditorExtensionInterface* StrategyInterface = nullptr;
// 	uint32 InheritanceDistance = UINT32_MAX;
//
// 	for (auto& Extension : Extensions)
// 	{
// 		if (InheritFromClosest(StrategyClass, Extension.Key, InheritanceDistance))
// 		{
// 			StrategyInterface = Extension.Value.Get();
// 		}
// 	}
//
// 	if (StrategyInterface)
// 	{
// 		return StrategyInterface->GetDefaultLaunchConfiguration_Virtual(Strategy, OutConfiguration, OutWorldDimensions);
// 	}
//
// 	UE_LOG(LogSpatialGDKEditorLBExtension, Error, TEXT("Could not find editor extension for load balancing strategy %s"), *StrategyClass->GetName());
// 	return false;
// }
//
// void FLBStrategyEditorExtensionManager::RegisterExtension(UClass* StrategyClass, TUniquePtr<FLBStrategyEditorExtensionInterface> StrategyExtension)
// {
// 	Extensions.Add(StrategyClass, MoveTemp(StrategyExtension));
// }
//
// void FLBStrategyEditorExtensionManager::UnregisterExtension(UClass* StrategyClass)
// {
// 	Extensions.Remove(StrategyClass);
// }
//
// void FLBStrategyEditorExtensionManager::Cleanup()
// {
// 	Extensions.Empty();
// }
