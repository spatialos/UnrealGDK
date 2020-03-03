#include "GridLBStrategyEditorExtension.h"
#include "SpatialGDKEditorSettings.h"

class UGridBasedLBStrategy_Spy : public UGridBasedLBStrategy
{
public:
	using UGridBasedLBStrategy::WorldWidth;
	using UGridBasedLBStrategy::WorldHeight;
	using UGridBasedLBStrategy::Rows;
	using UGridBasedLBStrategy::Cols;
};

bool FGridLBStrategyEditorExtension::GetDefaultLaunchConfiguration(const UGridBasedLBStrategy* Strategy, FWorkerTypeLaunchSection& OutConfiguration, FIntPoint& OutWorldDimensions) const
{
	const UGridBasedLBStrategy_Spy* StrategySpy = static_cast<const UGridBasedLBStrategy_Spy*>(Strategy);

	OutConfiguration.Rows = StrategySpy->Rows;
	OutConfiguration.Columns = StrategySpy->Cols;
	OutConfiguration.NumEditorInstances = StrategySpy->Rows * StrategySpy->Cols;

	// m ? cm ?
	OutWorldDimensions = FIntPoint(StrategySpy->WorldWidth, StrategySpy->WorldHeight);

	return true;
}
