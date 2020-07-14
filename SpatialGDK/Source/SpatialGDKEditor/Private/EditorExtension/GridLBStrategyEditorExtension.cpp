// // Copyright (c) Improbable Worlds Ltd, All Rights Reserved
//
// #include "GridLBStrategyEditorExtension.h"
// #include "SpatialGDKEditorSettings.h"
// #include "SpatialRuntimeLoadBalancingStrategies.h"
//
// class UGridBasedLBStrategy_Spy : public UGridBasedLBStrategy
// {
// public:
// 	using UGridBasedLBStrategy::WorldWidth;
// 	using UGridBasedLBStrategy::WorldHeight;
// 	using UGridBasedLBStrategy::Rows;
// 	using UGridBasedLBStrategy::Cols;
// };
//
// bool FGridLBStrategyEditorExtension::GetDefaultLaunchConfiguration(const UGridBasedLBStrategy* Strategy, UAbstractRuntimeLoadBalancingStrategy*& OutConfiguration, FIntPoint& OutWorldDimensions) const
// {
// 	const UGridBasedLBStrategy_Spy* StrategySpy = static_cast<const UGridBasedLBStrategy_Spy*>(Strategy);
//
// 	UGridRuntimeLoadBalancingStrategy* GridStrategy = NewObject<UGridRuntimeLoadBalancingStrategy>();
// 	GridStrategy->Rows = StrategySpy->Rows;
// 	GridStrategy->Columns = StrategySpy->Cols;
// 	OutConfiguration = GridStrategy;
//
// 	// Convert from cm to m.
// 	OutWorldDimensions = FIntPoint(StrategySpy->WorldWidth / 100, StrategySpy->WorldHeight / 100);
//
// 	return true;
// }
