#include "EditorExtension/LBStrategyEditorExtension.h"
#include "LoadBalancing/AbstractLBStrategy.h"

//DEFINE_LOG_CATEGORY(LogSpatialGDKEditorExtensions);

namespace
{
	bool InheritFromClosest(UClass* Derived, UClass* PotentialBase, uint32& InOutPreviousDistance)
	{
		uint32 InheritanceDistance = 0;
		for (const UStruct* TempStruct = Derived; TempStruct; TempStruct = TempStruct->GetSuperStruct())
		{
			if (TempStruct == PotentialBase)
			{
				break;
			}
			++InheritanceDistance;
			if (InheritanceDistance > InOutPreviousDistance)
			{
				return false;
			}
		}

		InOutPreviousDistance = InheritanceDistance;
		return true;
	}
}

TArray<TPair<UClass*, FLBStrategyEditorExtensionInterface*>> FLBStrategyEditorExtensionManager::ExtensionMap;

bool FLBStrategyEditorExtensionManager::GetDefaultLaunchConfiguration(const UAbstractLBStrategy* Strategy, FWorkerTypeLaunchSection& OutConfiguration, FIntPoint& OutWorldDimensions)
{
	if (!Strategy)
	{
		return false;
	}

	UClass* StrategyClass = Strategy->GetClass();

	FLBStrategyEditorExtensionInterface* StrategyInterface = nullptr;
	uint32 InheritanceDistance = UINT32_MAX;

	for (auto& Extension : ExtensionMap)
	{
		if (InheritFromClosest(StrategyClass, Extension.Key, InheritanceDistance))
		{
			StrategyInterface = Extension.Value;
		}
	}

	if (StrategyInterface)
	{
		return StrategyInterface->GetDefaultLaunchConfiguration_Virtual(Strategy, OutConfiguration, OutWorldDimensions);
	}

	//UE_LOG(LogSpatialGDKEditorExtensions, Error, TEXT(""))

	return false;
}
