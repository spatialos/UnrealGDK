#include "EditorExtension/LBStrategyEditorExtension.h"
#include "LoadBalancing/AbstractLBStrategy.h"

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
			StrategyInterface = Extension.Value.Get();
		}
	}

	if (StrategyInterface)
	{
		return StrategyInterface->GetDefaultLaunchConfiguration_Virtual(Strategy, OutConfiguration, OutWorldDimensions);
	}

	return false;
}

void FLBStrategyEditorExtensionManager::RegisterExtension(UClass* StrategyClass, FLBStrategyEditorExtensionInterface* StrategyExtension)
{
	ExtensionMap.Push(decltype(ExtensionMap)::ElementType(StrategyClass, StrategyExtension));
}

void FLBStrategyEditorExtensionManager::Cleanup()
{
	ExtensionMap.Empty();
}
