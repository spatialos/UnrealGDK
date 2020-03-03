#include "EditorExtension/LBStrategyEditorExtension.h"
#include "LoadBalancing/AbstractLBStrategy.h"

//DEFINE_LOG_CATEGORY(LogSpatialGDKEditorExtensions);

TMap<UClass*, FLBStrategyEditorExtensionInterface*> FLBStrategyEditorExtensionManager::ExtensionMap;

bool FLBStrategyEditorExtensionManager::GetDefaultLaunchConfiguration(const UAbstractLBStrategy* Strategy, FWorkerTypeLaunchSection& OutConfiguration, FIntPoint& OutWorldDimensions)
{
	if (!Strategy)
	{
		return false;
	}

	UClass* StrategyClass = Strategy->GetClass();
	if (FLBStrategyEditorExtensionInterface** Interface = ExtensionMap.Find(StrategyClass))
	{
		(*Interface)->GetDefaultLaunchConfiguration(Strategy, OutConfiguration, OutWorldDimensions);
	}

	//UE_LOG(LogSpatialGDKEditorExtensions, Error, TEXT(""))

	return false;
}
