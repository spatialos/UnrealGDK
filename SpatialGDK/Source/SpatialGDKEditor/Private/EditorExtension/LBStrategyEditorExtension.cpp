// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

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

} // anonymous namespace

bool FLBStrategyEditorExtensionManager::GetDefaultLaunchConfiguration(const UAbstractLBStrategy* Strategy, FWorkerTypeLaunchSection& OutConfiguration, FIntPoint& OutWorldDimensions)
{
	if (!Strategy)
	{
		return false;
	}

	UClass* StrategyClass = Strategy->GetClass();

	FLBStrategyEditorExtensionInterface* StrategyInterface = nullptr;
	uint32 InheritanceDistance = UINT32_MAX;

	for (auto& Extension : Extensions)
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

void FLBStrategyEditorExtensionManager::RegisterExtension(UClass* StrategyClass, TUniquePtr<FLBStrategyEditorExtensionInterface> StrategyExtension)
{
	Extensions.Push(ExtensionArray::ElementType(StrategyClass, MoveTemp(StrategyExtension)));
}

void FLBStrategyEditorExtensionManager::Cleanup()
{
	Extensions.Empty();
}
